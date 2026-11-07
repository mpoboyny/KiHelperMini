#pragma once

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <exception>
#include <filesystem>
#include <istream>
#include <string>
#include <utility>
#include <vector>

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

namespace todo {

struct command
{
    size_t start;
    size_t end;
    std::string name;
    std::string arguments;
};

inline bool find_commands(const std::string & text, std::vector<command> & commands,
                          std::string & error)
{
    std::vector<command> parsed;
    size_t pos = 0;
    while ((pos = text.find('$', pos)) != std::string::npos) {
        const size_t start = pos + 1;
        std::string name;
        for (const char * candidate : {"/run", "/#", "/read", "/glob", "/image", "/audio", "/video", "/clear", "/regen"}) {
            const std::string value(candidate);
            if (text.compare(start, value.size(), value) == 0 &&
                (start + value.size() == text.size() || text[start + value.size()] == '$' ||
                 std::isspace(static_cast<unsigned char>(text[start + value.size()])))) {
                name = value;
                break;
            }
        }
        if (name.empty()) {
            ++pos;
            continue;
        }
        const size_t end = text.find('$', start + name.size());
        if (end == std::string::npos) {
            error = "A todo " + name + " command must end with '$'.";
            return false;
        }
        const size_t first = text.find_first_not_of(" \t\r\n", start + name.size());
        const bool has_arguments = first != std::string::npos && first < end;
        const bool requires_arguments = name != "/clear" && name != "/regen";
        if (requires_arguments && !has_arguments) {
            error = "A todo " + name + " command requires an argument.";
            return false;
        }
        if (!requires_arguments && has_arguments) {
            error = "A todo " + name + " command does not accept arguments.";
            return false;
        }
        const std::string arguments = has_arguments
            ? text.substr(first, text.find_last_not_of(" \t\r\n", end - 1) - first + 1) : "";
        if (arguments.find('\0') != std::string::npos) {
            error = "A todo " + name + " command must not contain a null character.";
            return false;
        }
        if (name == "/regen" &&
            (text.find_first_not_of(" \t\r\n", 0) != pos ||
             text.find_first_not_of(" \t\r\n", end + 1) != std::string::npos)) {
            error = "A todo /regen command must be alone on its line.";
            return false;
        }
        parsed.push_back({pos, end + 1, name, arguments});
        pos = end + 1;
    }
    commands = std::move(parsed);
    return true;
}

inline bool read_prompts(std::istream & input, std::vector<std::string> & prompts,
                         std::string & output_folder, std::string & error) {
    std::vector<std::string> parsed;
    std::string parsed_folder;
    std::string line;
    size_t line_number = 0;
    while (std::getline(input, line)) {
        ++line_number;
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        if (line_number == 1 && line.compare(0, 3, "\xEF\xBB\xBF") == 0) {
            line.erase(0, 3);
        }
        if (line_number == 1 && line.compare(0, 2, "\xC2\xA7") == 0) {
            parsed_folder = line.substr(2);
            if (parsed_folder.find_first_not_of(" \t") == std::string::npos ||
                parsed_folder.find('\0') != std::string::npos) {
                error = "The todo output folder must not be empty or contain a null character.";
                return false;
            }
            continue;
        }
        if (line.empty() || line.front() != '>') {
            error = "Every todo line must start with '>' (line " + std::to_string(line_number) + ").";
            return false;
        }
        line.erase(0, 1);
        if (line.find_first_not_of(" \t") == std::string::npos) {
            error = "Todo prompts must not be empty (line " + std::to_string(line_number) + ").";
            return false;
        }
        std::vector<command> commands;
        if (!find_commands(line, commands, error)) {
            error += " (line " + std::to_string(line_number) + ").";
            return false;
        }
        parsed.push_back(std::move(line));
    }
    if (input.bad()) {
        error = "Failed to read the todo file.";
        return false;
    }
    parsed.push_back("/exit");
    prompts = std::move(parsed);
    output_folder = std::move(parsed_folder);
    return true;
}

inline bool prepare_output_path(const std::string & folder, const std::filesystem::path & todo_path,
                                std::filesystem::path & output_path, std::string & error) {
    namespace fs = std::filesystem;
    try {
        fs::path directory;
#ifdef _WIN32
        const std::wstring source = folder.empty() ? L"%USERPROFILE%\\temp" : fs::u8path(folder).wstring();
        for (size_t pos = 0; (pos = source.find(L'%', pos)) != std::wstring::npos;) {
            const size_t end = source.find(L'%', pos + 1);
            if (end == std::wstring::npos) {
                break;
            }
            const std::wstring name = source.substr(pos + 1, end - pos - 1);
            if (name.empty() || GetEnvironmentVariableW(name.c_str(), nullptr, 0) == 0) {
                error = "An environment variable in the todo output folder is not set.";
                return false;
            }
            pos = end + 1;
        }
        DWORD size = ExpandEnvironmentStringsW(source.c_str(), nullptr, 0);
        while (size != 0) {
            std::vector<wchar_t> expanded(size);
            const DWORD length = ExpandEnvironmentStringsW(source.c_str(), expanded.data(), size);
            if (length == 0) {
                break;
            }
            if (length <= size) {
                directory = fs::path(expanded.data());
                break;
            }
            size = length;
        }
        if (directory.empty()) {
            error = "Cannot expand the todo output folder.";
            return false;
        }
#else
        std::string path = folder.empty() ? "~/temp" : folder;
        if (path == "~" || path.compare(0, 2, "~/") == 0) {
            const char * home = std::getenv("HOME");
            if (home == nullptr || home[0] == '\0') {
                error = "Cannot expand the todo output folder: HOME is not set.";
                return false;
            }
            path = std::string(home) + path.substr(1);
        }
        directory = fs::u8path(path);
#endif
        const fs::path result = directory / "lastResponses.txt";
        std::error_code ec;
        if (fs::equivalent(todo_path, result, ec) && !ec) {
            error = "The todo file must not be the response file.";
            return false;
        }
        ec.clear();
        fs::create_directories(directory, ec);
        if (ec) {
            error = "Cannot create response directory: " + ec.message();
            return false;
        }
        if (!fs::is_directory(directory, ec) || ec) {
            error = "The todo output folder is not a directory.";
            return false;
        }
        output_path = result;
        return true;
    } catch (const std::exception & e) {
        error = std::string("Invalid todo output folder: ") + e.what();
        return false;
    }
}

// Text responses have no structured confirmation flag. Only answer explicit
// confirmation requests; ordinary conversational questions need no reply.
inline bool asks_confirmation(const std::string & text) {
    std::string lower = text;
    std::transform(lower.begin(), lower.end(), lower.begin(), [](unsigned char ch) {
        return static_cast<char>(std::tolower(ch));
    });
    for (const char * marker : {"[y/n]", "[y/n", "(y/n)", "yes/no", "yes or no"}) {
        if (lower.find(marker) != std::string::npos) {
            return true;
        }
    }
    if (lower.find('?') == std::string::npos) {
        return false;
    }
    for (const char * phrase : {"do you want me to", "would you like me to", "may i ",
                                "shall i ", "should i ", "can i proceed", "can i continue",
                                "proceed?", "continue?", "confirm?", "please confirm"}) {
        if (lower.find(phrase) != std::string::npos) {
            return true;
        }
    }
    return false;
}

} // namespace todo
