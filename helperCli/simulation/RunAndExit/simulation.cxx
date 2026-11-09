#if defined(_WIN32)

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <tlhelp32.h>

#include <array>
#include <cwchar>
#include <filesystem>
#include <iostream>
#include <iterator>
#include <string>
#include <string_view>
#include <vector>

namespace {

constexpr wchar_t model_filename[] = L"Qwen_Qwen3.5-0.8B-Q4_K_M.gguf";

class unique_handle {
public:
    explicit unique_handle(HANDLE handle = nullptr) noexcept : handle_(handle) {}
    ~unique_handle() {
        if (handle_ != nullptr && handle_ != INVALID_HANDLE_VALUE) {
            CloseHandle(handle_);
        }
    }

    unique_handle(const unique_handle &) = delete;
    unique_handle & operator=(const unique_handle &) = delete;

    HANDLE get() const noexcept { return handle_; }
    bool valid() const noexcept { return handle_ != nullptr && handle_ != INVALID_HANDLE_VALUE; }

private:
    HANDLE handle_;
};

std::filesystem::path executable_path() {
    std::vector<wchar_t> buffer(32768);
    const DWORD length = GetModuleFileNameW(nullptr, buffer.data(), static_cast<DWORD>(buffer.size()));
    if (length == 0 || length >= buffer.size()) {
        return {};
    }
    return std::filesystem::path(std::wstring(buffer.data(), length));
}

bool start_process(const std::wstring & application, const std::wstring & command,
                   const std::wstring & directory, DWORD flags, PROCESS_INFORMATION & process) {
    STARTUPINFOW startup{};
    startup.cb = sizeof(startup);
    std::vector<wchar_t> mutable_command(command.begin(), command.end());
    mutable_command.push_back(L'\0');
    return CreateProcessW(application.c_str(), mutable_command.data(), nullptr, nullptr, FALSE,
                          flags, nullptr, directory.empty() ? nullptr : directory.c_str(),
                          &startup, &process) != FALSE;
}

DWORD find_helper_process(DWORD parent_id) {
    unique_handle snapshot(CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0));
    if (!snapshot.valid()) {
        std::wcerr << L"Cannot enumerate processes: " << GetLastError() << L'\n';
        return 0;
    }

    PROCESSENTRY32W entry{};
    entry.dwSize = sizeof(entry);
    if (!Process32FirstW(snapshot.get(), &entry)) {
        std::wcerr << L"Cannot read the process list: " << GetLastError() << L'\n';
        return 0;
    }
    do {
        if (entry.th32ParentProcessID == parent_id &&
            _wcsicmp(entry.szExeFile, L"helperCli.exe") == 0) {
            return entry.th32ProcessID;
        }
    } while (Process32NextW(snapshot.get(), &entry));
    return 0;
}

int inject_input(DWORD process_id, std::wstring_view text) {
    if (!AttachConsole(process_id)) {
        return 1;
    }
    unique_handle input(CreateFileW(L"CONIN$", GENERIC_READ | GENERIC_WRITE,
                                    FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr,
                                    OPEN_EXISTING, 0, nullptr));
    if (!input.valid()) {
        FreeConsole();
        return 1;
    }

    std::vector<INPUT_RECORD> records(text.size() * 2);
    std::size_t index = 0;
    for (const wchar_t character : text) {
        SHORT key = 0;
        if (character == L'\r') {
            key = VK_RETURN;
        } else if (character == L'\t') {
            key = VK_TAB;
        } else if (character == 0x1B) {
            key = VK_ESCAPE;
        } else {
            key = VkKeyScanW(character);
        }
        if (key == -1) {
            FreeConsole();
            return 1;
        }
        const WORD virtual_key = LOBYTE(key);
        const BYTE modifiers = HIBYTE(key);
        DWORD control_state = 0;
        if (modifiers & 1) control_state |= SHIFT_PRESSED;
        if (modifiers & 2) control_state |= LEFT_CTRL_PRESSED;
        if (modifiers & 4) control_state |= LEFT_ALT_PRESSED;
        for (const BOOL key_down : { TRUE, FALSE }) {
            INPUT_RECORD & record = records[index++];
            record.EventType = KEY_EVENT;
            record.Event.KeyEvent.bKeyDown = key_down;
            record.Event.KeyEvent.wRepeatCount = 1;
            record.Event.KeyEvent.wVirtualKeyCode = virtual_key;
            record.Event.KeyEvent.wVirtualScanCode = static_cast<WORD>(MapVirtualKeyW(virtual_key, MAPVK_VK_TO_VSC));
            record.Event.KeyEvent.uChar.UnicodeChar = character;
            record.Event.KeyEvent.dwControlKeyState = control_state;
        }
    }

    DWORD written = 0;
    const bool success = WriteConsoleInputW(input.get(), records.data(),
                                            static_cast<DWORD>(records.size()), &written) &&
                         written == records.size();
    FreeConsole();
    return success ? 0 : 1;
}

bool send_command(const std::filesystem::path & self, DWORD helper_id, const wchar_t * command) {
    const std::wstring inject_command = L"\"" + self.wstring() + L"\" --inject " +
                                        std::to_wstring(helper_id) + L" " + command;
    PROCESS_INFORMATION injector{};
    if (!start_process(self.wstring(), inject_command, L"", DETACHED_PROCESS, injector)) {
        std::wcerr << L"Cannot start the input sender: " << GetLastError() << L'\n';
        return false;
    }
    unique_handle injector_process(injector.hProcess);
    unique_handle injector_thread(injector.hThread);
    DWORD injector_exit = 0;
    if (WaitForSingleObject(injector_process.get(), INFINITE) != WAIT_OBJECT_0 ||
        !GetExitCodeProcess(injector_process.get(), &injector_exit) || injector_exit != 0) {
        std::wcerr << L"Cannot send input to the helperCli console.\n";
        return false;
    }
    return true;
}

int run_simulation(const std::filesystem::path & self) {
    const std::filesystem::path helper =
        (self.parent_path() / L".." / L".." / L".." / L".." /
         L"bin" / L"release" / L"helperCli.exe").lexically_normal();
    std::error_code error;
    if (!std::filesystem::is_regular_file(helper, error)) {
        std::wcerr << L"helperCli executable not found: " << helper.c_str() << L'\n';
        return 1;
    }
    const DWORD profile_size = GetEnvironmentVariableW(L"USERPROFILE", nullptr, 0);
    if (profile_size == 0) {
        std::wcerr << L"USERPROFILE is not set.\n";
        return 1;
    }
    std::vector<wchar_t> profile(profile_size);
    const DWORD profile_length = GetEnvironmentVariableW(L"USERPROFILE", profile.data(), profile_size);
    if (profile_length == 0 || profile_length >= profile_size) {
        std::wcerr << L"Cannot read USERPROFILE.\n";
        return 1;
    }
    const std::filesystem::path model_path = std::filesystem::path(profile.data()) / L"LocalKiModels" / model_filename;
    if (!std::filesystem::is_regular_file(model_path, error)) {
        std::wcerr << L"Model not found: " << model_path.c_str() << L'\n';
        return 1;
    }

    std::array<wchar_t, MAX_PATH> system_directory{};
    const UINT length = GetSystemDirectoryW(system_directory.data(), static_cast<UINT>(system_directory.size()));
    if (length == 0 || length >= system_directory.size()) {
        std::wcerr << L"Cannot find the system directory.\n";
        return 1;
    }
    const std::wstring command_processor = (std::filesystem::path(system_directory.data()) / L"cmd.exe").wstring();
    const std::wstring command = L"\"" + command_processor + L"\" /d /s /c \"\"" +
                                 helper.wstring() + L"\" -m \"" + model_path.wstring() + L"\"\"";
    PROCESS_INFORMATION shell{};
    if (!start_process(command_processor, command, helper.parent_path().wstring(), CREATE_NEW_CONSOLE, shell)) {
        std::wcerr << L"Cannot start the command window: " << GetLastError() << L'\n';
        return 1;
    }
    unique_handle shell_process(shell.hProcess);
    unique_handle shell_thread(shell.hThread);

    std::wcout << L"Started helperCli in a new command window; waiting 5 seconds...\n";
    Sleep(5000);
    const DWORD helper_id = find_helper_process(shell.dwProcessId);
    if (helper_id == 0) {
        std::wcerr << L"helperCli is not running after startup.\n";
        return 1;
    }
    unique_handle helper_process(OpenProcess(SYNCHRONIZE | PROCESS_QUERY_LIMITED_INFORMATION, FALSE, helper_id));
    if (!helper_process.valid()) {
        std::wcerr << L"Cannot open helperCli process: " << GetLastError() << L'\n';
        return 1;
    }

#ifdef RUN_DIR_BEFORE_EXIT
    if (!send_command(self, helper_id, L"run-dir")) {
        std::wcerr << L"Cannot send /run dir to the helperCli console.\n";
        return 1;
    }
    std::wcout << L"Sent /run dir and Enter; waiting 3 seconds...\n";
    Sleep(3000);
#endif
    if (!send_command(self, helper_id, L"exit")) {
        std::wcerr << L"Cannot send /exit to the helperCli console.\n";
        return 1;
    }

    std::wcout << L"Sent /exit and Enter; waiting for helperCli to finish...\n";
    const DWORD state = WaitForSingleObject(helper_process.get(), 120000);
    if (state == WAIT_TIMEOUT) {
        std::wcerr << L"FAIL: helperCli is still running.\n";
        return 2;
    }
    if (state != WAIT_OBJECT_0) {
        std::wcerr << L"Cannot check helperCli: " << GetLastError() << L'\n';
        return 1;
    }
    DWORD exit_code = 0;
    if (!GetExitCodeProcess(helper_process.get(), &exit_code)) {
        std::wcerr << L"Cannot retrieve the helperCli exit code: " << GetLastError() << L'\n';
        return 1;
    }
    if (exit_code != 0) {
        std::wcerr << L"FAIL: helperCli exited with code " << exit_code << L".\n";
        return 1;
    }
    std::wcout << L"PASS: helperCli exited successfully.\n";
    return 0;
}

} // namespace

int wmain(int argc, wchar_t ** argv) {
    if (argc == 4 && std::wcscmp(argv[1], L"--inject") == 0) {
        wchar_t * end = nullptr;
        const unsigned long process_id = std::wcstoul(argv[2], &end, 10);
        if (argv[2][0] == L'\0' || *end != L'\0' || process_id == 0) {
            return 1;
        }
        if (std::wcscmp(argv[3], L"exit") == 0) {
            return inject_input(static_cast<DWORD>(process_id), L"/exit\r");
        }
        if (std::wcscmp(argv[3], L"run-dir") == 0) {
            return inject_input(static_cast<DWORD>(process_id), L"/run dir\t\x1B\r");
        }
        return 1;
    }

    const std::filesystem::path self = executable_path();
    if (self.empty()) {
        std::wcerr << L"Cannot determine the simulation executable path.\n";
        return 1;
    }
    if (argc == 2 && std::wcscmp(argv[1], L"--run") == 0) {
        return run_simulation(self);
    }
    if (argc != 1) {
        std::wcerr << L"Unexpected arguments.\n";
        return 1;
    }

    // Keep stdout and stderr connected to a caller capturing the simulation.
    // An interactive launch still gets its own console below.
    if (GetFileType(GetStdHandle(STD_OUTPUT_HANDLE)) == FILE_TYPE_PIPE ||
        GetFileType(GetStdHandle(STD_ERROR_HANDLE)) == FILE_TYPE_PIPE) {
        return run_simulation(self);
    }

    PROCESS_INFORMATION simulation{};
    const std::wstring command = L"\"" + self.wstring() + L"\" --run";
    if (!start_process(self.wstring(), command, L"", CREATE_NEW_CONSOLE, simulation)) {
        std::wcerr << L"Cannot start the simulation window: " << GetLastError() << L'\n';
        return 1;
    }
    unique_handle simulation_process(simulation.hProcess);
    unique_handle simulation_thread(simulation.hThread);
    DWORD exit_code = 1;
    if (WaitForSingleObject(simulation_process.get(), INFINITE) != WAIT_OBJECT_0 ||
        !GetExitCodeProcess(simulation_process.get(), &exit_code)) {
        std::wcerr << L"Cannot retrieve the simulation result.\n";
        return 1;
    }
    if (exit_code == 2) {
        std::wcerr << L"Simulation failed: helperCli is still running.\n";
    } else {
        std::wcout << (exit_code == 0 ? L"Simulation passed.\n" : L"Simulation failed.\n");
    }
    return exit_code == 0 ? 0 : 1;
}

#elif defined(__linux__)

#include <chrono>
#include <cerrno>
#include <csignal>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <iostream>
#include <string>
#include <thread>

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

static constexpr char model_filename[] = "Qwen_Qwen3.5-0.8B-Q4_K_M.gguf";

static std::filesystem::path executable_path()
{
    std::error_code error;
    const std::filesystem::path path = std::filesystem::read_symlink("/proc/self/exe", error);
    return error ? std::filesystem::path() : path;
}

static bool write_all(const int file_descriptor, const std::string & text)
{
    size_t written = 0;
    while (written < text.size()) {
        const ssize_t result = write(file_descriptor, text.data() + written, text.size() - written);
        if (result > 0) {
            written += static_cast<size_t>(result);
            continue;
        }
        if (result < 0 && errno == EINTR) {
            continue;
        }
        return false;
    }
    return true;
}

static int run_simulation(const std::filesystem::path & self)
{
    const std::filesystem::path helper =
        (self.parent_path() / ".." / ".." / ".." / ".." / "bin" / "release" / "helperCli").lexically_normal();
    std::error_code error;
    if (!std::filesystem::is_regular_file(helper, error)) {
        std::cerr << "helperCli executable not found: " << helper << '\n';
        return 1;
    }

    const char * home = std::getenv("HOME");
    if (home == nullptr || home[0] == '\0') {
        std::cerr << "HOME is not set.\n";
        return 1;
    }
    const std::filesystem::path model_path = std::filesystem::path(home) / "LocalKiModels" / model_filename;
    if (!std::filesystem::is_regular_file(model_path, error)) {
        std::cerr << "Model not found: " << model_path << '\n';
        return 1;
    }

    int input_pipe[2] = {-1, -1};
    if (pipe(input_pipe) != 0) {
        std::cerr << "Cannot create the input pipe: " << std::strerror(errno) << '\n';
        return 1;
    }

    const pid_t child = fork();
    if (child < 0) {
        std::cerr << "Cannot start helperCli: " << std::strerror(errno) << '\n';
        close(input_pipe[0]);
        close(input_pipe[1]);
        return 1;
    }
    if (child == 0) {
        close(input_pipe[1]);
        if (dup2(input_pipe[0], STDIN_FILENO) < 0) {
            _exit(127);
        }
        close(input_pipe[0]);
        execl(helper.c_str(), helper.c_str(), "-m", model_path.c_str(), "--simple-io", "--no-warmup", static_cast<char *>(nullptr));
        _exit(127);
    }

    close(input_pipe[0]);
    std::signal(SIGPIPE, SIG_IGN);
    std::string commands;
#ifdef RUN_DIR_BEFORE_EXIT
    commands += "/run dir\n";
#endif
    commands += "/exit\n";
    const bool input_sent = write_all(input_pipe[1], commands);
    close(input_pipe[1]);
    if (!input_sent) {
        std::cerr << "Cannot send input to helperCli.\n";
        kill(child, SIGKILL);
        waitpid(child, nullptr, 0);
        return 1;
    }

    const auto deadline = std::chrono::steady_clock::now() + std::chrono::minutes(2);
    int status = 0;
    while (true) {
        const pid_t result = waitpid(child, &status, WNOHANG);
        if (result == child) {
            break;
        }
        if (result < 0 && errno != EINTR) {
            std::cerr << "Cannot check helperCli: " << std::strerror(errno) << '\n';
            return 1;
        }
        if (std::chrono::steady_clock::now() >= deadline) {
            std::cerr << "FAIL: helperCli is still running.\n";
            kill(child, SIGKILL);
            waitpid(child, nullptr, 0);
            return 2;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
        std::cerr << "FAIL: helperCli exited with code "
                  << (WIFEXITED(status) ? WEXITSTATUS(status) : 1) << ".\n";
        return 1;
    }
    std::cout << "PASS: helperCli exited successfully.\n";
    return 0;
}

int main()
{
    const std::filesystem::path self = executable_path();
    if (self.empty()) {
        std::cerr << "Cannot determine the simulation executable path.\n";
        return 1;
    }
    return run_simulation(self);
}

#else
#error Unsupported platform
#endif
