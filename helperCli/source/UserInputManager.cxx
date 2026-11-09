#include "pre.hxx"

#include "UserInputManager.hxx"
#include "cli-ui.hxx"

std::string UserInputManager::ReadInput(bool simple_io, bool multiline_input, bool color_enabled,
    std::vector<run_input_command> & commands)
{
    if (!simple_io) {
        std::string line = ui::read_run_input(color_enabled, this);
        commands = ui::take_run_input_commands();
        return line;
    }
    ui::user_turn user_turn;
    return user_turn.read_input(multiline_input);
}

void UserInputManager::AddPrompt(const std::string & prompt, const std::vector<run_input_command> & commands)
{
    if (!prompt.empty()) {
        history.push_back({prompt, commands});
    }
}

void UserInputManager::Clear() noexcept
{
    history.clear();
    draft.line.clear();
    draft.commands.clear();
    position = 0;
}

void UserInputManager::BeginInput() noexcept
{
    position = history.size();
    draft.line.clear();
    draft.commands.clear();
}

bool UserInputManager::Previous(std::string & line, std::vector<run_input_command> & commands)
{
    if (position == 0) {
        return false;
    }
    if (position == history.size()) {
        draft = {line, commands};
    }
    const Entry & entry = history[--position];
    line = entry.line;
    commands = entry.commands;
    return true;
}

bool UserInputManager::Next(std::string & line, std::vector<run_input_command> & commands)
{
    if (position >= history.size()) {
        return false;
    }
    ++position;
    const Entry & entry = position == history.size() ? draft : history[position];
    line = entry.line;
    commands = entry.commands;
    return true;
}
