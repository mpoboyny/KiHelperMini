# RunAndExit Simulation

## Overview

This simulation tests the basic lifecycle of the `helperCli` application by launching it, sending it an exit command, and verifying it terminates cleanly.

## What It Tests

The **RunAndExit** simulation performs the following steps:

1. **Launches `helperCli`** with a specified model file
2. **Sends `/exit` command** to gracefully terminate the application
3. **Verifies successful termination** by checking:
   - The application exits within the timeout period
   - The exit code is `0` (success)

## Platform-Specific Behavior

### Windows (`.exe`)

- Uses Windows API (`CreateProcessW`, `AttachConsole`, `WriteConsoleInputW`) to:
  - Launch `helperCli` in a new console window
  - Inject keyboard input (`/exit`) into the console
  - Wait for process termination and retrieve exit code
- Requires `helperCli.exe` and the model file to be on Windows

### Linux

- Uses a named pipe to communicate with `helperCli`
- Launches `helperCli` with `--simple-io --no-warmup` flags
- Writes commands to stdin via the pipe
- Polls for process termination with a 2-minute timeout

## Build & Execution

### Linux
```bash
cd simulation/RunAndExit
make release
# Run the simulation (opens a new window)
../../bin/simulation/RunAndExit/release/RunAndExit
# Or run programmatically (captures output)
../../bin/simulation/RunAndExit/release/RunAndExit
```

### Windows (MSVC)
```cmd
cd simulation\RunAndExit
nmake /f NMakefile release
# Run the simulation (opens a new window)
..\..\bin\simulation\RunAndExit\release\RunAndExit.exe
# Or run programmatically (captures output)
..\..\bin\simulation\RunAndExit\release\RunAndExit.exe
```

## Expected Output

### Successful Run
```
Loading model...

[ASCII logo displayed]

build      : b181-fabd1727
model      : /path/to/model.gguf
ftype      : Q4_K - Medium
modalities : text

available commands:
  /exit or Ctrl+C      stop or exit
  /regen               regenerate the last response
  /clear               clear the chat history
  /read <file>         add a text file
  /run or /# <command> run a command
  /glob <pattern>      add text files using globbing pattern

Exiting...
PASS: helperCli exited successfully.
```

### Failure Conditions

| Exit Code | Condition |
|-----------|-----------|
| 1 | `helperCli.exe` not found |
| 1 | Model file not found |
| 1 | Cannot launch helperCli process |
| 1 | Cannot send commands to helperCli |
| 2 | helperCli is still running after timeout |

## Prerequisites

- `helperCli` release binary built in `bin/release/helperCli`
- Model file: `Qwen_Qwen3.5-0.8B-Q4_K_M.gguf` in `~/LocalKiModels/` (Linux) or `%USERPROFILE%\LocalKiModels\` (Windows)

## Notes

This is a minimal simulation that only tests application lifecycle (launch → exit). To test the `/run dir` command functionality, use the `RunDirExit` simulation instead.