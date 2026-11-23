#include <array>
#include <cerrno>
#include <csignal>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <iostream>
#include <string>
#include <thread>
#include <chrono>

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
    commands += "/run dir\n";
    commands += "\x1B[Z";
    commands += "how many so files are are given\n";
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

