param(
	[string] $Name
)

# The worker runs in a new cmd window. The encoded command keeps paths and
# simulation names out of cmd.exe's command-line parsing.
$worker = {
	param([string] $ScriptDirectory, [string] $SimulationName, [int] $ExtraArgumentCount)

	try {
		$ErrorActionPreference = 'Stop'
		if ([string]::IsNullOrWhiteSpace($SimulationName) -or $ExtraArgumentCount -ne 0) {
			throw 'Provide exactly one simulation name.'
		}

		$configPath = Join-Path $ScriptDirectory 'simulation.xml'
		$settings = New-Object System.Xml.XmlReaderSettings
		$settings.DtdProcessing = [System.Xml.DtdProcessing]::Prohibit
		$settings.XmlResolver = $null
		$reader = [System.Xml.XmlReader]::Create($configPath, $settings)
		try {
			$config = New-Object System.Xml.XmlDocument
			$config.XmlResolver = $null
			$config.Load($reader)
		}
		finally {
			$reader.Dispose()
		}

		$matches = @($config.SelectNodes('/Simulations/Simulation') | Where-Object {
			$_.GetAttribute('Name') -eq $SimulationName
		})
		if ($matches.Count -ne 1) {
			throw "Expected exactly one simulation named '$SimulationName'; found $($matches.Count)."
		}

		$folder = $matches[0].GetAttribute('Folder')
		if ([string]::IsNullOrWhiteSpace($folder) -or $folder -eq '.' -or $folder -eq '..' -or
			$folder.IndexOfAny([System.IO.Path]::GetInvalidFileNameChars()) -ge 0 -or
			$folder.Contains('\') -or $folder.Contains('/')) {
			throw 'The simulation folder must be a valid folder name.'
		}

		$releaseDirectory = Join-Path (Join-Path $ScriptDirectory $folder) 'release'
		$executable = Join-Path $releaseDirectory ($folder + '.exe')
		if (-not (Test-Path -LiteralPath $executable -PathType Leaf)) {
			throw "Simulation executable not found: $executable"
		}

		Write-Host "Running simulation '$SimulationName': $executable"
		# Native stderr is represented as error records by Windows PowerShell;
		# Continue prevents those records from interrupting output collection.
		$ErrorActionPreference = 'Continue'
		& $executable 2>&1 | ForEach-Object { Write-Host $_ }
		$exitCode = $LASTEXITCODE
		Write-Host "Simulation exit code: $exitCode"
	}
	catch {
		[Console]::Error.WriteLine($_.Exception.Message)
	}
}

$scriptDirectoryLiteral = $PSScriptRoot.Replace("'", "''")
$nameLiteral = ([string] $Name).Replace("'", "''")
$command = "& { $worker } '$scriptDirectoryLiteral' '$nameLiteral' $($args.Count)"
$encodedCommand = [Convert]::ToBase64String([Text.Encoding]::Unicode.GetBytes($command))
$powerShell = Join-Path $env:SystemRoot 'System32\WindowsPowerShell\v1.0\powershell.exe'
$commandLine = "/d /k `"`"$powerShell`" -NoLogo -NoProfile -EncodedCommand $encodedCommand`""
Start-Process -FilePath $env:ComSpec -ArgumentList $commandLine
