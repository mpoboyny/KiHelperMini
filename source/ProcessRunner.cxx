//
// ProcessRunner.cxx
//

#include "prc.hxx"
#include "ProcessRunner.hxx"

wxString ProcessRunner::Run(const wxString& exePath, const wxString& args)
{
    wxString output;
    wxArrayString stdoutLines;
    wxArrayString stderrLines;
    wxString command = wxString::Format("\"%s\" %s", exePath, args);

    long exitCode = wxExecute(command, stdoutLines, stderrLines, wxEXEC_SYNC);
    if (exitCode == -1) {
        return wxEmptyString;
    }

    auto appendLines = [&](const wxArrayString& lines) {
        for (const auto& line : lines) {
            if (!output.IsEmpty()) {
                output += "\n";
            }
            output += line;
        }
    };

    appendLines(stdoutLines);
    if (output.IsEmpty()) {
        appendLines(stderrLines);
    }

    return output;
}

#ifdef _WIN32

bool ProcessRunner::RunAsyncInNewWindow(const wxString& scriptPath, int WXUNUSED(displayIndex))
{
    TrFu;
    // Launch script in a new console window using cmd's start command.
    wxString command = wxString::Format("cmd /c start \"\" \"%s\"", scriptPath);
    long pid = wxExecute(command, wxEXEC_ASYNC);
    return pid > 0;
}

bool ProcessRunner::RunSyncInNewWindow(const wxString& scriptPath, int WXUNUSED(displayIndex))
{
    TrFu;
    wxString command = wxString::Format("cmd /c start /wait \"\" \"%s\"", scriptPath);
    long exitCode = wxExecute(command, wxEXEC_SYNC);
    return exitCode != -1;
}

#elif defined(__gnu_linux__)

bool ProcessRunner::RunAsyncInNewWindow(const wxString& scriptPath, int displayIndex)
{
    TrFu;
    /*
        Bei -fs 12 (Fontgröße 12) ist ein Zeichen grob 8 Pixel breit und 18 Pixel hoch.
            1000 Pixel / 8  etwa 125 Spalten
            700 Pixel / 18 etwa 40 Zeilen
        Jetzt in Zeichen (Columns) und Zeilen (Rows) statt Pixeln!
    */
    int targetCols = 125; 
    int targetRows = 40;  
    int posX = 500; // Position bleibt in Pixeln
    int posY = 300;

    auto buildCommand = [&](bool holdWindow) {
        return wxString::Format(
            "xterm -geometry %dx%d+%d+%d " // Geometrie kompakt zusammengefasst
            "-xrm \"XTerm*selectToClipboard: true\" "
            "-fa Monospace -fs 12 %s-e \"%s\"",
            targetCols, targetRows, posX, posY,
            holdWindow ? "-hold " : "",
            scriptPath
        );
    };
    
    long pid = wxExecute(buildCommand(true), wxEXEC_ASYNC);
    return pid > 0;
}

bool ProcessRunner::RunSyncInNewWindow(const wxString& scriptPath, int displayIndex)
{
    TrFu;
    wxRect screenRect = wxDisplay(displayIndex).GetClientArea();

    int targetWidth = 500;
    int targetHeight = 300;

    int posX = screenRect.x + (screenRect.width - targetWidth) / 2;
    int posY = screenRect.y + (screenRect.height - targetHeight) / 2;

    wxString command = wxString::Format(
        "xterm -geometry +%d+%d "
        "-xrm \"XTerm*vt100.geometry: %dx%d\\nXTerm*selectToClipboard: true\" "
        "-fa Monospace -fs 12 -hold -e \"%s\"",
        posX, posY,
        targetWidth, targetHeight,
        scriptPath
    );

    long pid = wxExecute(command, wxEXEC_ASYNC);
    if (pid <= 0) {
        return false;
    }

    auto isStillRunning = [](long processId) {
        wxString procPath = wxString::Format("/proc/%ld", processId);
        return wxDirExists(procPath);
    };

    while (isStillRunning(pid)) {
        wxYieldIfNeeded();
        wxMilliSleep(100);
    }

    return true;
}

#else

bool ProcessRunner::RunAsyncInNewWindow(const wxString& WXUNUSED(scriptPath), int WXUNUSED(displayIndex))
{
    TrFu;
    return false;
}

bool ProcessRunner::RunSyncInNewWindow(const wxString& WXUNUSED(scriptPath), int WXUNUSED(displayIndex))
{
    TrFu;
    return false;
}

#endif

