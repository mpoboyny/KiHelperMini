#pragma once

#include <algorithm>
#include <cctype>
#include <istream>
#include <string>
#include <utility>
#include <vector>

namespace todo {

inline bool read_prompts(std::istream & input, std::vector<std::string> & prompts, std::string & error) {
    std::vector<std::string> parsed;
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
        if (line.empty() || line.front() != '>') {
            error = "Every todo line must start with '>' (line " + std::to_string(line_number) + ").";
            return false;
        }
        line.erase(0, 1);
        if (line.find_first_not_of(" \t") == std::string::npos) {
            error = "Todo prompts must not be empty (line " + std::to_string(line_number) + ").";
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
    return true;
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