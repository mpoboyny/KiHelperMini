#include "pre.hxx"
#include "RunHandler.hxx"

#include <cerrno>
#include <cstdio>

std::string RunHandler::Handle(const std::string & args) {
    TrFu;
    if (args.empty()) {
        return "Hallo";
    }

    std::string command = "bash -c '";
    for (char ch : args) {
        command += ch == '\'' ? "'\\''" : std::string(1, ch);
    }
    command += "' 2>&1";
    FILE * pipe = popen(command.c_str(), "r");
    if (!pipe) {
        return std::string("Failed to start process: ") + std::strerror(errno);
    }

    std::string output;
    char buffer[4096];
    size_t count;
    while ((count = std::fread(buffer, 1, sizeof(buffer), pipe)) > 0) {
        output.append(buffer, count);
    }
    pclose(pipe);
    return output;
}
