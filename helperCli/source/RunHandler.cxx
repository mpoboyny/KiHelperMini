#include "pre.hxx"
#include "RunHandler.hxx"

#include <cerrno>
#include <cstdio>
#if !defined(_WIN32)
#include <sys/wait.h>
#endif

#if !defined(_WIN32)
std::string RunHandler::Handle(const std::string & args) {
    TrFu;
    status.clear();
    succeeded = false;
    if (args.empty()) {
        status = "No command was executed.";
        return "Hallo";
    }

    std::string command = "bash -c '";
    for (char ch : args) {
        command += ch == '\'' ? "'\\''" : std::string(1, ch);
    }
    command += "' 2>&1";
    FILE * pipe = popen(command.c_str(), "r");
    if (!pipe) {
        status = std::string("Failed to start process: ") + std::strerror(errno);
        return status;
    }

    std::string output;
    char buffer[4096];
    size_t count;
    while ((count = std::fread(buffer, 1, sizeof(buffer), pipe)) > 0) {
        output.append(buffer, count);
    }
    bool read_error = std::ferror(pipe);
    int result = pclose(pipe);
    if (result == -1) {
        status = std::string("Failed to wait for process: ") + std::strerror(errno);
    } else if (WIFEXITED(result)) {
        int exit_code = WEXITSTATUS(result);
        status = "Exit code: " + std::to_string(exit_code);
        succeeded = exit_code == 0 && !read_error;
    } else if (WIFSIGNALED(result)) {
        status = "Terminated by signal: " + std::to_string(WTERMSIG(result));
    } else {
        status = "Process ended with unknown status.";
    }
    if (read_error) {
        status += " (failed to read complete output)";
    }
    return output;
}
#else

static std::wstring wide_from_utf8(const std::string & text)
{
    if (text.empty()) {
        return std::wstring();
    }
    const int size = MultiByteToWideChar(CP_UTF8, 0, text.data(), static_cast<int>(text.size()), nullptr, 0);
    std::wstring wide(static_cast<size_t>(size), L'\0');
    MultiByteToWideChar(CP_UTF8, 0, text.data(), static_cast<int>(text.size()), wide.data(), size);
    return wide;
}

static void read_pipe(HANDLE handle, std::string & output, bool & read_error)
{
    char buffer[4096];
    for (;;) {
        DWORD count = 0;
        if (!ReadFile(handle, buffer, sizeof(buffer), &count, nullptr)) {
            read_error = true;
            break;
        }
        if (count == 0) {
            break;
        }
        output.append(buffer, count);
    }
}

std::string RunHandler::Handle(const std::string & args)
{
    TrFu;
    status.clear();
    succeeded = false;
    if (args.empty()) {
        status = "No command was executed.";
        return "Hallo";
    }

    std::string escaped;
    for (char ch : args) {
        if (ch == '"') {
            escaped += '\\';
        }
        escaped += ch;
    }
    std::wstring command = L"cmd.exe /d /s /c \"" + wide_from_utf8(escaped) + L"\"";

    SECURITY_ATTRIBUTES pipe_attrs;
    pipe_attrs.nLength = sizeof(pipe_attrs);
    pipe_attrs.bInheritHandle = TRUE;
    pipe_attrs.lpSecurityDescriptor = nullptr;
    HANDLE read_end = nullptr;
    HANDLE write_end = nullptr;
    if (!CreatePipe(&read_end, &write_end, &pipe_attrs, 0)) {
        status = std::string("Failed to start process: ") + std::to_string(GetLastError());
        return status;
    }
    SetHandleInformation(read_end, HANDLE_FLAG_INHERIT, 0);

    STARTUPINFOW startup{};
    startup.cb = sizeof(startup);
    startup.dwFlags = STARTF_USESTDHANDLES;
    startup.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
    startup.hStdOutput = write_end;
    startup.hStdError = write_end;
    PROCESS_INFORMATION process{};
    if (!CreateProcessW(nullptr, command.data(), nullptr, nullptr, TRUE, CREATE_NO_WINDOW,
                        nullptr, nullptr, &startup, &process)) {
        const DWORD code = GetLastError();
        CloseHandle(read_end);
        CloseHandle(write_end);
        status = std::string("Failed to start process: ") + std::to_string(code);
        return status;
    }
    CloseHandle(process.hThread);

    std::string output;
    bool read_error = false;
    read_pipe(read_end, output, read_error);
    CloseHandle(read_end);
    CloseHandle(write_end);

    if (WaitForSingleObject(process.hProcess, INFINITE) == WAIT_FAILED) {
        const DWORD code = GetLastError();
        CloseHandle(process.hProcess);
        status = std::string("Failed to wait for process: ") + std::to_string(code);
        if (read_error) {
            status += " (failed to read complete output)";
        }
        return output;
    }
    DWORD exit_code = 0;
    if (GetExitCodeProcess(process.hProcess, &exit_code) == 0) {
        status = "Process ended with unknown status.";
    } else {
        status = "Exit code: " + std::to_string(static_cast<long>(exit_code));
        succeeded = exit_code == 0 && !read_error;
    }
    CloseHandle(process.hProcess);
    if (read_error) {
        status += " (failed to read complete output)";
    }
    return output;
}
#endif
