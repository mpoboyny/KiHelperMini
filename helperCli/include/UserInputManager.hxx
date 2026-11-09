#pragma once

#include <cstddef>
#include <string>
#include <vector>

struct run_input_command
{
    size_t start;
    size_t end;
    size_t prefix_length;
};

class UserInputManager
{
public:
    std::string ReadInput(bool simple_io, bool multiline_input, bool color_enabled,
        std::vector<run_input_command> & commands);
    void AddPrompt(const std::string & prompt, const std::vector<run_input_command> & commands);
    void Clear() noexcept;
    void BeginInput() noexcept;
    bool Previous(std::string & line, std::vector<run_input_command> & commands);
    bool Next(std::string & line, std::vector<run_input_command> & commands);

private:
    struct Entry
    {
        std::string line;
        std::vector<run_input_command> commands;
    };

    std::vector<Entry> history;
    Entry draft;
    size_t position = 0;
};
