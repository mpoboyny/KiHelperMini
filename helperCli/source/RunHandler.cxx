#include "pre.hxx"
#include "RunHandler.hxx"

#include <cerrno>
#include <cstdio>
#include <limits>
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

namespace {

class unique_handle {
public:
    explicit unique_handle(HANDLE handle = nullptr) noexcept : handle_(handle) {}
    ~unique_handle() { reset(); }

    unique_handle(const unique_handle &) = delete;
    unique_handle & operator=(const unique_handle &) = delete;

    HANDLE get() const noexcept { return handle_; }
    void reset() noexcept {
        if (handle_ != nullptr && handle_ != INVALID_HANDLE_VALUE) {
            CloseHandle(handle_);
        }
        handle_ = nullptr;
    }

private:
    HANDLE handle_;
};

} // namespace

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
            read_error = GetLastError() != ERROR_BROKEN_PIPE;
            break;
        }
        if (count == 0) {
            break;
        }
        output.append(buffer, count);
    }
}

static bool normalize_command_output(std::string & output)
{
    if (output.empty()) {
        return true;
    }
    if (output.size() > static_cast<size_t>(std::numeric_limits<int>::max())) {
        return false;
    }

    const int length = static_cast<int>(output.size());
    if (MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, output.data(), length, nullptr, 0) > 0) {
        return true;
    }

    const UINT code_page = GetOEMCP();
    if (code_page == CP_UTF8) {
        return false;
    }
    const int wide_length = MultiByteToWideChar(code_page, 0, output.data(), length, nullptr, 0);
    if (wide_length == 0) {
        return false;
    }
    std::wstring wide(static_cast<size_t>(wide_length), L'\0');
    if (MultiByteToWideChar(code_page, 0, output.data(), length, wide.data(), wide_length) == 0) {
        return false;
    }

    const int utf8_length = WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, wide.data(), wide_length,
                                                 nullptr, 0, nullptr, nullptr);
    if (utf8_length == 0) {
        return false;
    }
    std::string utf8(static_cast<size_t>(utf8_length), '\0');
    if (WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, wide.data(), wide_length,
                            utf8.data(), utf8_length, nullptr, nullptr) == 0) {
        return false;
    }
    output = std::move(utf8);
    return true;
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
    unique_handle read_handle(read_end);
    unique_handle write_handle(write_end);
    if (!SetHandleInformation(read_handle.get(), HANDLE_FLAG_INHERIT, 0)) {
        status = std::string("Failed to configure process pipe: ") + std::to_string(GetLastError());
        return status;
    }

    STARTUPINFOW startup{};
    startup.cb = sizeof(startup);
    startup.dwFlags = STARTF_USESTDHANDLES;
    startup.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
    startup.hStdOutput = write_handle.get();
    startup.hStdError = write_handle.get();
    PROCESS_INFORMATION process{};
    if (!CreateProcessW(nullptr, command.data(), nullptr, nullptr, TRUE, CREATE_NO_WINDOW,
                        nullptr, nullptr, &startup, &process)) {
        const DWORD code = GetLastError();
        status = std::string("Failed to start process: ") + std::to_string(code);
        return status;
    }
    unique_handle process_handle(process.hProcess);
    unique_handle thread_handle(process.hThread);
    // EOF is observable only after every writer, including the parent's, closes.
    write_handle.reset();

    std::string output;
    bool read_error = false;
    read_pipe(read_handle.get(), output, read_error);
    read_handle.reset();
    const bool decode_error = !normalize_command_output(output);
    if (decode_error) {
        output = "Failed to decode command output as text.";
    }

    if (WaitForSingleObject(process_handle.get(), INFINITE) == WAIT_FAILED) {
        const DWORD code = GetLastError();
        status = std::string("Failed to wait for process: ") + std::to_string(code);
        if (read_error) {
            status += " (failed to read complete output)";
        }
        if (decode_error) {
            status += " (failed to decode command output)";
        }
        return output;
    }
    DWORD exit_code = 0;
    if (GetExitCodeProcess(process_handle.get(), &exit_code) == 0) {
        status = "Process ended with unknown status.";
    } else {
        status = "Exit code: " + std::to_string(static_cast<long>(exit_code));
        succeeded = exit_code == 0 && !read_error && !decode_error;
    }
    if (read_error) {
        status += " (failed to read complete output)";
    }
    if (decode_error) {
        status += " (failed to decode command output)";
    }
    return output;
}
#endif
