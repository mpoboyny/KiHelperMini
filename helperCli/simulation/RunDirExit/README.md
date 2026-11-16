# RunDirExit Simulation

## Overview

This simulation tests the `helperCli` application by launching it, executing the `/run dir` command to verify directory listing functionality, and then sending an exit command to verify clean termination.

## What It Tests

The **RunDirExit** simulation performs the following steps:

1. **Launches `helperCli`** with a specified model file
2. **Executes `/run dir`** command to test directory listing functionality
3. **Sends `/exit` command** to gracefully terminate the application
4. **Verifies successful termination** by checking:
   - Directory listing output is captured
   - The application exits within the timeout period
   - The exit code is `0` (success)

## Key Difference from RunAndExit

The main difference from `RunAndExit` simulation is that `RunDirExit` **enables the `RUN_DIR_BEFORE_EXIT` preprocessor flag**, which causes the `/run dir` command to be sent before the `/exit` command. This allows testing of the command execution feature.

## Platform-Specific Behavior

### Windows (`.exe`)

- Uses Windows API (`CreateProcessW`, `AttachConsole`, `WriteConsoleInputW`) to:
  - Launch `helperCli` in a new console window
  - Inject keyboard input (`/run dir<TAB><ESC><ENTER>`, then `/exit<ENTER>`) into the console
  - Wait for process termination and retrieve exit code
- Requires `helperCli.exe` and the model file to be on Windows

### Linux

- Uses a named pipe to communicate with `helperCli`
- Launches `helperCli` with `--simple-io --no-warmup` flags
- Writes commands to stdin via the pipe:
  ```
  /run dir\n
  /exit\n
  ```
- Polls for process termination with a 2-minute timeout

## Build & Execution

### Linux
```bash
cd simulation/RunDirExit
make release
# Run the simulation (opens a new window)
../../bin/simulation/RunDirExit/release/RunDirExit
# Or run programmatically (captures output)
../../bin/simulation/RunDirExit/release/RunDirExit
```

### Windows (MSVC)
```cmd
cd simulation\RunDirExit
nmake /f NMakefile release
# Run the simulation (opens a new window)
..\..\bin\simulation\RunDirExit\release\RunDirExit.exe
# Or run programmatically (captures output)
..\..\bin\simulation\RunDirExit\release\RunDirExit.exe
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

> Command output:
AGENTS.md  helperCliRunner.sh  Makefile   obj     scripts  source
bin       include       NMakefile  README.md  simulation  tests

Exit code: 0
Command output lines: 2

>
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

## Implementation Notes

The `simulation.cxx` file simply includes the RunAndExit simulation code with the `RUN_DIR_BEFORE_EXIT` macro defined:

```cpp
#define RUN_DIR_BEFORE_EXIT
#include "../RunAndExit/simulation.cxx"
```

This macro causes the `run_simulation()` function to execute the `/run dir` command before the `/exit` command, making RunDirExit a superset of RunAndExit functionality.