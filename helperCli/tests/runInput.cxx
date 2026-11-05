#include <gtest/gtest.h>
#include "cli-ui.hxx"
#include "cli-todo.hxx"

#include <sstream>
#include <string>

#ifdef _WIN32
#include <windows.h>
#include <filesystem>
#include <fstream>
#include <iterator>
#include "common.hxx"

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

TEST(TodoInput, ParsesPromptsAndAppendsExit)
{
    std::istringstream input("\xEF\xBB\xBF>Hallo\r\n>/run hostname\r\n>/read \"C:\\Temp\\main.cxx\"\r\n>Create new file xxx.txt");
    std::vector<std::string> prompts;
    std::string error;
    ASSERT_TRUE(todo::read_prompts(input, prompts, error)) << error;
    ASSERT_EQ(prompts.size(), 5u);
    EXPECT_EQ(prompts[0], "Hallo");
    EXPECT_EQ(prompts[1], "/run hostname");
    EXPECT_EQ(prompts[2], "/read \"C:\\Temp\\main.cxx\"");
    EXPECT_EQ(prompts[3], "Create new file xxx.txt");
    EXPECT_EQ(prompts[4], "/exit");
}

TEST(TodoInput, RejectsInvalidLinesBeforeReturningPrompts)
{
    std::istringstream input(">first\nnot a prompt\n>third\n");
    std::vector<std::string> prompts = {"unchanged"};
    std::string error;
    EXPECT_FALSE(todo::read_prompts(input, prompts, error));
    EXPECT_NE(error.find("line 2"), std::string::npos);
    ASSERT_EQ(prompts.size(), 1u);
    EXPECT_EQ(prompts[0], "unchanged");
}

TEST(TodoInput, RejectsEmptyLinesAndEmptyPrompts)
{
    for (const std::string & text : {std::string(">valid\n\n"), std::string("> \t\n")}) {
        std::istringstream input(text);
        std::vector<std::string> prompts;
        std::string error;
        EXPECT_FALSE(todo::read_prompts(input, prompts, error));
        EXPECT_FALSE(error.empty());
    }
}

TEST(TodoInput, DetectsConfirmationRequestsButNotOrdinaryQuestions)
{
    EXPECT_TRUE(todo::asks_confirmation("Continue? [y/N]"));
    EXPECT_TRUE(todo::asks_confirmation("Should I proceed? Please confirm."));
    EXPECT_TRUE(todo::asks_confirmation("Would you like me to create the file?"));
    EXPECT_FALSE(todo::asks_confirmation("Hello! How are you?"));
    // The German greeting from the reported response, including its emoji.
    EXPECT_FALSE(todo::asks_confirmation(
        "\x48\x61\x6c\x6c\x6f\x21\x20\x57\x69\x65\x20\x67\x65\x68\x74\x20"
        "\x65\x73\x20\x64\x69\x72\x3f\x20\xf0\x9f\x98\x8a"));
    EXPECT_FALSE(todo::asks_confirmation("What can I help you with?"));
    EXPECT_FALSE(todo::asks_confirmation("Completed."));
}

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

TEST(RunInput, TabKeepsCommandOpen)
{
    EXPECT_EQ(read_run_input_with_keys("/run\tdir\r"), "/run dir\n");
}

TEST(RunInput, TabEndsHashCommandThenEnterSubmits)
{
    EXPECT_EQ(read_run_input_with_keys("/#\tdir\r"), "/# dir\n");
}

// Tab does not submit on its own: no Enter means the read is still pending,
// so a following WEOF returns whatever was typed so far.
TEST(RunInput, TabAloneDoesNotSubmit)
{
    // "/run" + Tab leaves the reader waiting; the injected input then ends
    // (WEOF) with a non-empty line, which is submitted with a newline.
    EXPECT_EQ(read_run_input_with_keys("/run\t"), "/run \n");
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
TEST(RunInput, EscapeAfterTabRestoresNormalInput)
{
    EXPECT_EQ(read_run_input_with_key_codes(U"/run dir *.dll\t\uE009How many DLL files?\r"),
              "/run dir *.dll  How many DLL files?\n");
    const std::vector<run_input_command> commands = ui::take_run_input_commands();
    ASSERT_EQ(commands.size(), 1u);
    EXPECT_EQ(commands[0].start, 0u);
    EXPECT_EQ(commands[0].end, 15u);
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
