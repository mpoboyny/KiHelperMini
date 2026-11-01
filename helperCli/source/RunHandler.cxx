#include "pre.hxx"
#include "RunHandler.hxx"

#include <cerrno>
#include <cstdio>
#include <sys/wait.h>

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
