#include <gtest/gtest.h>
#include "cli-ui.hxx"
#include "common.hxx"

#include <filesystem>
#include <cstdio>
#include <fstream>
#include <iterator>
#include <string>

#ifdef _WIN32
#include <windows.h>

namespace {
class test_handle {
public:
    explicit test_handle(HANDLE handle) noexcept : handle_(handle) {}
    ~test_handle() { close(); }
    test_handle(const test_handle &) = delete;
    test_handle & operator=(const test_handle &) = delete;
    HANDLE get() const noexcept { return handle_; }
    void close() noexcept {
        if (handle_ != nullptr && handle_ != INVALID_HANDLE_VALUE) {
            CloseHandle(handle_);
        }
        handle_ = nullptr;
    }
private:
    HANDLE handle_;
};
} // namespace
#endif

// Scripts a sequence of characters as the reader's console input.
// Returns what ui::read_run_input hands to the chat loop, i.e. the line with
// its trailing '\n' kept, or "" for a real EOF.
static std::string read_run_input_with_keys(const std::string & keys)
{
    size_t pos = 0;
    run_input_source = [&]() -> char32_t {
        if (pos >= keys.size()) {
            return WEOF;
        }
        return static_cast<char32_t>(static_cast<unsigned char>(keys[pos++]));
    };
    std::string result = ui::read_run_input(false);
    run_input_source = nullptr;
    return result;
}

#ifdef _WIN32
static std::string read_run_input_with_key_codes(const std::u32string & keys)
{
    size_t pos = 0;
    run_input_source = [&]() -> char32_t {
        return pos < keys.size() ? keys[pos++] : WEOF;
    };
    std::string result = ui::read_run_input(false);
    run_input_source = nullptr;
    return result;
}
#endif

TEST(HelpOutput, StartsWithInputCommands)
{
#ifdef _WIN32
    const char * command = "bin\\release\\helperCli.exe --help";
    FILE * pipe = _popen(command, "r");
#else
    const char * command = "bin/release/helperCli --help";
    FILE * pipe = popen(command, "r");
#endif
    ASSERT_NE(pipe, nullptr);
    std::string output;
    char buffer[256] = {};
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        output += buffer;
    }
#ifdef _WIN32
    EXPECT_EQ(_pclose(pipe), 0);
#else
    EXPECT_EQ(pclose(pipe), 0);
#endif
    EXPECT_EQ(output.find("----- input commands -----\n"), 0u);
    EXPECT_NE(output.find("/read <file>"), std::string::npos);
    EXPECT_NE(output.find("/run or /# <command>"), std::string::npos);
    EXPECT_NE(output.find("----- common params -----"), std::string::npos);
}

TEST(RunInput, TabEndsCommandAndKeepsInputOpen)
{
    EXPECT_EQ(read_run_input_with_keys("/run\tdir\r"), "/run dir\n");
    const std::vector<run_input_command> commands = ui::take_run_input_commands();
    ASSERT_EQ(commands.size(), 1u);
    EXPECT_EQ(commands[0].start, 0u);
    EXPECT_EQ(commands[0].end, 4u);
    EXPECT_EQ(commands[0].prefix_length, 4u);
}

TEST(RunInput, TabEndsHashCommandThenEnterSubmits)
{
    EXPECT_EQ(read_run_input_with_keys("/#\tdir\r"), "/# dir\n");
    const std::vector<run_input_command> commands = ui::take_run_input_commands();
    ASSERT_EQ(commands.size(), 1u);
    EXPECT_EQ(commands[0].start, 0u);
    EXPECT_EQ(commands[0].end, 2u);
    EXPECT_EQ(commands[0].prefix_length, 2u);
}

// Tab does not submit on its own, and EOF discards an unfinished line.
TEST(RunInput, TabAloneDoesNotSubmit)
{
    EXPECT_EQ(read_run_input_with_keys("/run\t"), "");
}

static std::string read_history_input(UserInputManager & manager, const std::u32string & keys)
{
    size_t pos = 0;
    run_input_source = [&]() -> char32_t {
        return pos < keys.size() ? keys[pos++] : WEOF;
    };
    std::string result = ui::read_run_input(false, &manager);
    run_input_source = nullptr;
    return result;
}

#if defined(_WIN32)
static const std::u32string history_up = U"\uE002";
static const std::u32string history_down = U"\uE003";
static const std::u32string cursor_left = U"\uE000";
#else
static const std::u32string history_up = U"\x1b[A";
static const std::u32string history_down = U"\x1b[B";
static const std::u32string cursor_left = U"\x1b[D";
#endif

TEST(UserInputHistory, NavigatesEditsAndRestoresDraft)
{
    UserInputManager manager;
    manager.AddPrompt("first", {});
    manager.AddPrompt("second", {});
    EXPECT_EQ(read_history_input(manager, U"draft" + history_up + history_up + history_down + history_down + U"\r"), "draft\n");
    EXPECT_EQ(read_history_input(manager, history_up + U"!\r"), "second!\n");
    EXPECT_EQ(read_history_input(manager, history_up + U"\r"), "second\n");
}

TEST(UserInputHistory, EditedPromptIsSubmittedOnlyOnEnterAndCanBeRecalled)
{
    UserInputManager manager;
    const std::string first = read_history_input(manager, U"first prompt\r");
    ASSERT_EQ(first, "first prompt\n");
    manager.AddPrompt(first.substr(0, first.size() - 1), ui::take_run_input_commands());

    const std::string second = read_history_input(manager, U"second prompt\r");
    ASSERT_EQ(second, "second prompt\n");
    manager.AddPrompt(second.substr(0, second.size() - 1), ui::take_run_input_commands());

    EXPECT_EQ(read_history_input(manager, history_up + history_up + U" edited"), "");
    const std::string edited = read_history_input(manager, history_up + history_up + U" edited\r");
    ASSERT_EQ(edited, "first prompt edited\n");
    manager.AddPrompt(edited.substr(0, edited.size() - 1), ui::take_run_input_commands());

    EXPECT_EQ(read_history_input(manager, history_up + history_up + history_down + U"\r"),
        "first prompt edited\n");
}

TEST(UserInputHistory, DoesNotSubmitWithoutEnterAndClearRemovesHistory)
{
    UserInputManager manager;
    manager.AddPrompt("prompt", {});
    EXPECT_EQ(read_history_input(manager, history_up), "");
    EXPECT_EQ(read_history_input(manager, history_up + U"\x04"), "");
    manager.Clear();
    EXPECT_EQ(read_history_input(manager, history_up + U"\r"), "\n");
}

TEST(UserInputHistory, PreservesRunCommandBoundariesWhenEdited)
{
    UserInputManager manager;
    manager.AddPrompt("/run ls question", {{0, 7, 4}});
    EXPECT_EQ(read_history_input(manager, history_up + U"!\r"), "/run ls question!\n");
    const auto commands = ui::take_run_input_commands();
    ASSERT_EQ(commands.size(), 1u);
    EXPECT_EQ(commands[0].start, 0u);
    EXPECT_EQ(commands[0].end, 7u);
    EXPECT_EQ(commands[0].prefix_length, 4u);
}

TEST(UserInputHistory, AdjustsRunCommandBoundaryWhenCommandIsEdited)
{
    UserInputManager manager;
    manager.AddPrompt("/run ls question", {{0, 7, 4}});
    std::u32string keys = history_up;
    for (size_t i = 0; i < 9; ++i) {
        keys += cursor_left;
    }
    EXPECT_EQ(read_history_input(manager, keys + U" -l\r"), "/run ls -l question\n");
    const auto commands = ui::take_run_input_commands();
    ASSERT_EQ(commands.size(), 1u);
    EXPECT_EQ(commands[0].end, 10u);
}

TEST(RunInput, EnterSubmitsRunCommand)
{
    EXPECT_EQ(read_run_input_with_keys("/run dir\r"), "/run dir\n");
}

TEST(RunInput, EnterSubmitsHashCommand)
{
    EXPECT_EQ(read_run_input_with_keys("/# dir\r"), "/# dir\n");
}

TEST(RunInput, TabDoesNotDoubleSpace)
{
    // The line already ends in a space, so Tab must not add another.
    EXPECT_EQ(read_run_input_with_keys("/run \t\r"), "/run \n");
}

#ifdef _WIN32
TEST(RunInput, TabEndsCommandBeforeQuestion)
{
    EXPECT_EQ(read_run_input_with_key_codes(U"/run dir *.dll\tHow many DLL files?\r"),
              "/run dir *.dll How many DLL files?\n");
    const std::vector<run_input_command> commands = ui::take_run_input_commands();
    ASSERT_EQ(commands.size(), 1u);
    EXPECT_EQ(commands[0].start, 0u);
    EXPECT_EQ(commands[0].end, 14u);
    EXPECT_EQ(commands[0].prefix_length, 4u);
}

TEST(RunInput, EscapeRecordsMultipleCommands)
{
    read_run_input_with_key_codes(U"/run one\uE009question /run two\uE009done\r");
    const std::vector<run_input_command> commands = ui::take_run_input_commands();
    ASSERT_EQ(commands.size(), 2u);
    EXPECT_EQ(commands[0].prefix_length, 4u);
    EXPECT_EQ(commands[1].prefix_length, 4u);
}
#endif

TEST(RunInput, EnterOnEmptyLineIsSkippableLineNotEOF)
{
    // Must stay a line ("\n"), not collapse into the EOF marker ("");
    // the chat loop breaks on "" and skips "\n".
    EXPECT_EQ(read_run_input_with_keys("\r"), "\n");
}

TEST(RunInput, CtrlDEmptyLineIsEOF)
{
    EXPECT_EQ(read_run_input_with_keys("\x04"), "");
}

TEST(RunInput, TabOnPartialRunIsCompletionNotSubmit)
{
    // "/ru" is not yet a /run command: Tab completes it, then Enter submits.
    EXPECT_EQ(read_run_input_with_keys("/ru\t\r"), "/run \n");
}

TEST(RunInput, TabOnSlashAloneKeepsLine)
{
    // No completion candidate for "/": Tab does nothing, Enter submits "/".
    EXPECT_EQ(read_run_input_with_keys("/\t\r"), "/\n");
}

#ifdef _WIN32
TEST(RunInput, RunDirCompletesAndExitIsProcessed)
{
    namespace fs = std::filesystem;
    const fs::path executable = fs::absolute("bin/release/helperCli.exe");
    const fs::path model = fs::absolute(g_TestModel_Qwen_Qwen3dot5_0_8B_Q4_K_M);
    const fs::path input_path = fs::absolute(fs::path(g_TestOutputRoot) / "run_dir.input");
    const fs::path output_path = fs::absolute(fs::path(g_TestOutputRoot) / "run_dir.log");
    ASSERT_TRUE(fs::is_regular_file(executable));
    {
        std::ofstream input(input_path, std::ios::binary);
        ASSERT_TRUE(input.is_open());
        input << "/run dir\r\n/exit\r\n";
        input.close();
        ASSERT_TRUE(input);
    }

    SECURITY_ATTRIBUTES attributes{};
    attributes.nLength = sizeof(attributes);
    attributes.bInheritHandle = TRUE;
    test_handle input(CreateFileW(input_path.c_str(), GENERIC_READ, FILE_SHARE_READ,
                                  &attributes, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr));
    ASSERT_NE(input.get(), INVALID_HANDLE_VALUE);
    test_handle output(CreateFileW(output_path.c_str(), GENERIC_WRITE, FILE_SHARE_READ,
                                   &attributes, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr));
    ASSERT_NE(output.get(), INVALID_HANDLE_VALUE);

    STARTUPINFOW startup{};
    startup.cb = sizeof(startup);
    startup.dwFlags = STARTF_USESTDHANDLES;
    startup.hStdInput = input.get();
    startup.hStdOutput = output.get();
    startup.hStdError = output.get();
    std::wstring command = L"\"" + executable.wstring() + L"\" -m \"" + model.wstring() +
                           L"\" --simple-io --no-warmup";
    PROCESS_INFORMATION process{};
    ASSERT_TRUE(CreateProcessW(executable.c_str(), command.data(), nullptr, nullptr, TRUE,
                               CREATE_NO_WINDOW, nullptr, nullptr, &startup, &process))
        << "CreateProcessW failed: " << GetLastError();
    test_handle process_handle(process.hProcess);
    test_handle thread_handle(process.hThread);
    input.close();
    output.close();

    const DWORD wait_result = WaitForSingleObject(process_handle.get(), 120000);
    if (wait_result == WAIT_TIMEOUT) {
        if (TerminateProcess(process_handle.get(), 1)) {
            WaitForSingleObject(process_handle.get(), 5000);
        }
    }
    ASSERT_EQ(wait_result, WAIT_OBJECT_0) << "helperCli did not complete /run and /exit";
    DWORD exit_code = 0;
    ASSERT_TRUE(GetExitCodeProcess(process_handle.get(), &exit_code));
    std::ifstream log(output_path, std::ios::binary);
    ASSERT_TRUE(log.is_open());
    const std::string contents((std::istreambuf_iterator<char>(log)), std::istreambuf_iterator<char>());
    EXPECT_EQ(exit_code, 0u) << contents;
    EXPECT_NE(contents.find("Command output:"), std::string::npos) << contents;
    EXPECT_NE(contents.find("Exit code: 0"), std::string::npos) << contents;
    EXPECT_NE(contents.find("Exiting..."), std::string::npos) << contents;
}
#endif
