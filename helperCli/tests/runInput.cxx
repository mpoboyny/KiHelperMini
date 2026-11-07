#include <gtest/gtest.h>
#include "cli-ui.hxx"
#include "cli-todo.hxx"
#include "common.hxx"

#include <filesystem>
#include <fstream>
#include <iterator>
#include <sstream>
#include <string>

#if defined(__linux__)
#include <cerrno>
#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <sys/wait.h>
#include <unistd.h>
#endif

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

TEST(TodoInput, ParsesPromptsAndAppendsExit)
{
    std::istringstream input("\xEF\xBB\xBF>Hallo\r\n>$/run hostname$\r\n>/read \"C:\\Temp\\main.cxx\"\r\n>Create new file xxx.txt");
    std::vector<std::string> prompts;
    std::string folder;
    std::string error;
    ASSERT_TRUE(todo::read_prompts(input, prompts, folder, error)) << error;
    EXPECT_TRUE(folder.empty());
    ASSERT_EQ(prompts.size(), 5u);
    EXPECT_EQ(prompts[0], "Hallo");
    EXPECT_EQ(prompts[1], "$/run hostname$");
    EXPECT_EQ(prompts[2], "/read \"C:\\Temp\\main.cxx\"");
    EXPECT_EQ(prompts[3], "Create new file xxx.txt");
    EXPECT_EQ(prompts[4], "/exit");
}

TEST(TodoInput, FindsDelimitedCommandsInOrder)
{
    const std::string text = "Compare $/run dir *.dll$ with $/# echo done$ now";
    std::vector<todo::command> commands;
    std::string error;
    ASSERT_TRUE(todo::find_commands(text, commands, error)) << error;
    ASSERT_EQ(commands.size(), 2u);
    EXPECT_EQ(text.substr(commands[0].start, commands[0].end - commands[0].start), "$/run dir *.dll$");
    EXPECT_EQ(commands[0].arguments, "dir *.dll");
    EXPECT_EQ(text.substr(commands[1].start, commands[1].end - commands[1].start), "$/# echo done$");
    EXPECT_EQ(commands[1].arguments, "echo done");
}

TEST(TodoInput, DoesNotRecognizeUndelimitedCommandsOrOrdinaryDollarSigns)
{
    for (const std::string & text : {std::string("/run dir"), std::string("Question /# dir"),
                                     std::string("Price $5 and $/runner test$"), std::string("$/exit$")}) {
        std::vector<todo::command> commands;
        std::string error;
        ASSERT_TRUE(todo::find_commands(text, commands, error)) << error;
        EXPECT_TRUE(commands.empty());
    }
}

TEST(TodoInput, FindsAllSupportedDelimitedCommands)
{
    const std::string text = "Compare $/read \"file name.txt\"$ and $/glob *.txt$ "
        "with $/image image.png$ $/audio sound.wav$ $/video video.mp4$ $/clear$ $/run dir$ $/# echo ok$";
    std::vector<todo::command> commands;
    std::string error;
    ASSERT_TRUE(todo::find_commands(text, commands, error)) << error;
    ASSERT_EQ(commands.size(), 8u);
    EXPECT_EQ(commands[0].name, "/read");
    EXPECT_EQ(commands[0].arguments, "\"file name.txt\"");
    EXPECT_EQ(commands[1].name, "/glob");
    EXPECT_EQ(commands[1].arguments, "*.txt");
    EXPECT_EQ(commands[2].name, "/image");
    EXPECT_EQ(commands[3].name, "/audio");
    EXPECT_EQ(commands[4].name, "/video");
    EXPECT_EQ(commands[5].name, "/clear");
    EXPECT_TRUE(commands[5].arguments.empty());
    EXPECT_EQ(commands[6].name, "/run");
    EXPECT_EQ(commands[7].name, "/#");
}

TEST(TodoInput, RegenerateMustStandAlone)
{
    for (const std::string & text : {std::string("$/regen$"), std::string("  $/regen$  ")}) {
        std::vector<todo::command> commands;
        std::string error;
        ASSERT_TRUE(todo::find_commands(text, commands, error)) << error;
        ASSERT_EQ(commands.size(), 1u);
        EXPECT_EQ(commands[0].name, "/regen");
    }
    for (const std::string & text : {std::string("text $/regen$"), std::string("$/regen$ text"),
                                     std::string("$/clear$ $/regen$"), std::string("$/regen extra$")}) {
        std::vector<todo::command> commands;
        std::string error;
        EXPECT_FALSE(todo::find_commands(text, commands, error));
        EXPECT_FALSE(error.empty());
    }
}

TEST(TodoInput, ValidatesAllDelimitedCommandsBeforeReturningPrompts)
{
    for (const std::string & command : {std::string("$/read$"), std::string("$/glob   $"),
                                         std::string("$/image$"), std::string("$/audio$"),
                                         std::string("$/video$"), std::string("$/clear extra$"),
                                         std::string("$/read file"), std::string("text $/regen$")}) {
        std::istringstream input(">first\n>" + command + "\n");
        std::vector<std::string> prompts = {"unchanged"};
        std::string folder = "unchanged";
        std::string error;
        EXPECT_FALSE(todo::read_prompts(input, prompts, folder, error));
        EXPECT_NE(error.find("line 2"), std::string::npos);
        ASSERT_EQ(prompts.size(), 1u);
        EXPECT_EQ(prompts[0], "unchanged");
        EXPECT_EQ(folder, "unchanged");
    }
}

TEST(TodoInput, RejectsInvalidDelimitedCommandsBeforeReturningPrompts)
{
    for (const std::string & command : {std::string("$/run dir"), std::string("$/# dir"),
                                        std::string("$/run$"), std::string("$/#   $"),
                                        std::string("$/run echo ok$ and $/run$")}) {
        std::istringstream input(">first\n>" + command + "\n");
        std::vector<std::string> prompts = {"unchanged"};
        std::string folder = "unchanged";
        std::string error;
        EXPECT_FALSE(todo::read_prompts(input, prompts, folder, error));
        EXPECT_NE(error.find("line 2"), std::string::npos);
        ASSERT_EQ(prompts.size(), 1u);
        EXPECT_EQ(prompts[0], "unchanged");
        EXPECT_EQ(folder, "unchanged");
    }
}

TEST(TodoInput, RejectsInvalidLinesBeforeReturningPrompts)
{
    std::istringstream input(">first\nnot a prompt\n>third\n");
    std::vector<std::string> prompts = {"unchanged"};
    std::string folder = "unchanged";
    std::string error;
    EXPECT_FALSE(todo::read_prompts(input, prompts, folder, error));
    EXPECT_NE(error.find("line 2"), std::string::npos);
    ASSERT_EQ(prompts.size(), 1u);
    EXPECT_EQ(prompts[0], "unchanged");
    EXPECT_EQ(folder, "unchanged");
}

TEST(TodoInput, RejectsEmptyLinesAndEmptyPrompts)
{
    for (const std::string & text : {std::string(">valid\n\n"), std::string("> \t\n")}) {
        std::istringstream input(text);
        std::vector<std::string> prompts;
        std::string folder;
        std::string error;
        EXPECT_FALSE(todo::read_prompts(input, prompts, folder, error));
        EXPECT_FALSE(error.empty());
    }
}

TEST(TodoInput, ParsesFirstLineOutputFolder)
{
    std::istringstream input("\xEF\xBB\xBF\xC2\xA7" "folder with spaces\r\n>first\r\n>second\r\n");
    std::vector<std::string> prompts;
    std::string folder;
    std::string error;
    ASSERT_TRUE(todo::read_prompts(input, prompts, folder, error)) << error;
    EXPECT_EQ(folder, "folder with spaces");
    ASSERT_EQ(prompts.size(), 3u);
    EXPECT_EQ(prompts[0], "first");
    EXPECT_EQ(prompts[1], "second");
    EXPECT_EQ(prompts[2], "/exit");
}

TEST(TodoInput, RejectsEmptyOrMisplacedOutputFolder)
{
    for (const std::string & text : {std::string("\xC2\xA7\n>first\n"),
                                     std::string("\xC2\xA7 \t\n>first\n"),
                                     std::string(">first\n\xC2\xA7" "folder\n")}) {
        std::istringstream input(text);
        std::vector<std::string> prompts = {"unchanged"};
        std::string folder = "unchanged";
        std::string error;
        EXPECT_FALSE(todo::read_prompts(input, prompts, folder, error));
        EXPECT_EQ(prompts[0], "unchanged");
        EXPECT_EQ(folder, "unchanged");
        EXPECT_FALSE(error.empty());
    }
}

TEST(TodoInput, CreatesOutputDirectoryAndRejectsSameFile)
{
    namespace fs = std::filesystem;
    const fs::path directory = fs::absolute(fs::path(g_TestOutputRoot) / fs::u8path("todo \xC3\xA4 path") / "nested");
    fs::path output;
    std::string error;
    ASSERT_TRUE(todo::prepare_output_path(directory.u8string(), fs::path("todo.txt"), output, error)) << error;
    EXPECT_EQ(output, directory / "lastResponses.txt");
    EXPECT_TRUE(fs::is_directory(directory));
    {
        std::ofstream file(output);
        ASSERT_TRUE(file.is_open());
        file << ">/exit\n";
    }
    EXPECT_FALSE(todo::prepare_output_path(directory.u8string(), output, output, error));
    EXPECT_NE(error.find("must not be the response file"), std::string::npos);
}

TEST(TodoInput, RejectsOutputDirectoryThatIsAFile)
{
    namespace fs = std::filesystem;
    const fs::path file_path = fs::absolute(fs::path(g_TestOutputRoot) / "todo_output_file");
    {
        std::ofstream file(file_path);
        ASSERT_TRUE(file.is_open());
    }
    fs::path output;
    std::string error;
    EXPECT_FALSE(todo::prepare_output_path(file_path.u8string(), fs::path("todo.txt"), output, error));
    EXPECT_FALSE(error.empty());
}

#if defined(_WIN32) || defined(__linux__)
class TestHomeEnvironment
{
public:
    TestHomeEnvironment()
    {
#ifdef _WIN32
        const DWORD size = GetEnvironmentVariableW(L"USERPROFILE", nullptr, 0);
        if (size != 0) {
            std::vector<wchar_t> value(size);
            const DWORD length = GetEnvironmentVariableW(L"USERPROFILE", value.data(), size);
            if (length != 0 && length < size) {
                original_ = std::wstring(value.data(), length);
                existed_ = true;
            }
        }
#else
        const char * value = std::getenv("HOME");
        if (value != nullptr) {
            original_ = value;
            existed_ = true;
        }
#endif
    }

    ~TestHomeEnvironment()
    {
#ifdef _WIN32
        SetEnvironmentVariableW(L"USERPROFILE", existed_ ? original_.c_str() : nullptr);
#else
        if (existed_) {
            setenv("HOME", original_.c_str(), 1);
        } else {
            unsetenv("HOME");
        }
#endif
    }

    bool set(const std::filesystem::path & path)
    {
#ifdef _WIN32
        return SetEnvironmentVariableW(L"USERPROFILE", path.c_str()) != 0;
#else
        return setenv("HOME", path.c_str(), 1) == 0;
#endif
    }

    bool unset()
    {
#ifdef _WIN32
        return SetEnvironmentVariableW(L"USERPROFILE", nullptr) != 0;
#else
        return unsetenv("HOME") == 0;
#endif
    }

private:
#ifdef _WIN32
    std::wstring original_;
#else
    std::string original_;
#endif
    bool existed_ = false;
};

TEST(TodoInput, ExpandsHomeForDefaultAndSpecifiedOutputFolders)
{
    namespace fs = std::filesystem;
    const fs::path home = fs::absolute(fs::path(g_TestOutputRoot) / "test home");
    TestHomeEnvironment environment;
    ASSERT_TRUE(environment.set(home));
    fs::path output;
    std::string error;
    ASSERT_TRUE(todo::prepare_output_path("", fs::path("todo.txt"), output, error)) << error;
    EXPECT_EQ(output, home / "temp" / "lastResponses.txt");
    EXPECT_TRUE(fs::is_directory(home / "temp"));
#ifdef _WIN32
    const std::string folder = "%USERPROFILE%\\custom folder";
#else
    const std::string folder = "~/custom folder";
#endif
    ASSERT_TRUE(todo::prepare_output_path(folder, fs::path("todo.txt"), output, error)) << error;
    EXPECT_EQ(output, home / "custom folder" / "lastResponses.txt");
    EXPECT_TRUE(fs::is_directory(home / "custom folder"));
    ASSERT_TRUE(environment.unset());
    EXPECT_FALSE(todo::prepare_output_path("", fs::path("todo.txt"), output, error));
    EXPECT_FALSE(error.empty());
}
#endif

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

#if defined(__linux__)
TEST(TodoInput, LiteralHomeModelPathAndRelativeTodoFile)
{
    namespace fs = std::filesystem;
    const fs::path executable = fs::absolute("bin/release/helperCli");
    const fs::path model = fs::absolute(g_TestModel_Qwen_Qwen3dot5_0_8B_Q4_K_M);
    const fs::path home = model.parent_path().parent_path();
    const fs::path workdir = fs::absolute(fs::path(g_TestOutputRoot) / "todo_home_model_path");
    const fs::path output_path = workdir / "output.log";
    const std::string model_arg = "~/LocalKiModels/" + model.filename().string();
    ASSERT_TRUE(fs::is_regular_file(executable));
    ASSERT_TRUE(fs::create_directories(workdir));
    {
        std::ofstream input(workdir / "todo.txt", std::ios::binary);
        ASSERT_TRUE(input.is_open());
        input << "\xC2\xA7" << (workdir / "responses").u8string() << "\n>/exit\n";
        input.close();
        ASSERT_TRUE(input);
    }

    const pid_t child = fork();
    ASSERT_GE(child, 0);
    if (child == 0) {
        if (chdir(workdir.c_str()) != 0 || setenv("HOME", home.c_str(), 1) != 0 ||
            freopen(output_path.c_str(), "w", stdout) == nullptr ||
            dup2(STDOUT_FILENO, STDERR_FILENO) < 0) {
            _exit(127);
        }
        execl(executable.c_str(), executable.c_str(), "-m", model_arg.c_str(),
              "-todo", "todo.txt", "--no-warmup", static_cast<char *>(nullptr));
        _exit(127);
    }

    int status = 0;
    pid_t result = 0;
    for (int i = 0; i < 1200 && result == 0; ++i) {
        result = waitpid(child, &status, WNOHANG);
        if (result < 0 && errno == EINTR) {
            result = 0;
        }
        if (result == 0) {
            usleep(100000);
        }
    }
    if (result != child) {
        kill(child, SIGKILL);
        waitpid(child, &status, 0);
    }
    ASSERT_EQ(result, child) << "helperCli did not complete todo processing";
    std::ifstream log(output_path, std::ios::binary);
    ASSERT_TRUE(log.is_open());
    const std::string contents((std::istreambuf_iterator<char>(log)), std::istreambuf_iterator<char>());
    ASSERT_TRUE(WIFEXITED(status)) << contents;
    EXPECT_EQ(WEXITSTATUS(status), 0) << contents;
    EXPECT_NE(contents.find(">/exit"), std::string::npos) << contents;
    EXPECT_NE(contents.find("Exiting..."), std::string::npos) << contents;
    EXPECT_TRUE(fs::is_regular_file(workdir / "responses" / "lastResponses.txt"));
}
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
TEST(TodoInput, LiteralUserProfileModelPathAndRelativeTodoFile)
{
    namespace fs = std::filesystem;
    const fs::path executable = fs::absolute("bin/release/helperCli.exe");
    const fs::path workdir = fs::absolute(fs::path(g_TestOutputRoot) / "todo_model_path");
    const fs::path input_path = workdir / "todo.txt";
    const fs::path output_path = workdir / "output.log";
    ASSERT_TRUE(fs::is_regular_file(executable));
    ASSERT_TRUE(fs::create_directories(workdir));
    {
        std::ofstream input(input_path, std::ios::binary);
        ASSERT_TRUE(input.is_open());
        input << "\xC2\xA7" << (workdir / "responses").u8string() << "\n>/exit\n";
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
    std::wstring command = L"\"" + executable.wstring() +
        L"\" -m \"%USERPROFILE%\\LocalKiModels\\Qwen_Qwen3.5-0.8B-Q4_K_M.gguf\" -todo \"todo.txt\" --no-warmup";
    PROCESS_INFORMATION process{};
    ASSERT_TRUE(CreateProcessW(executable.c_str(), command.data(), nullptr, nullptr, TRUE,
                               CREATE_NO_WINDOW, nullptr, workdir.c_str(), &startup, &process))
        << "CreateProcessW failed: " << GetLastError();
    test_handle process_handle(process.hProcess);
    test_handle thread_handle(process.hThread);
    input.close();
    output.close();

    const DWORD wait_result = WaitForSingleObject(process_handle.get(), 120000);
    if (wait_result == WAIT_TIMEOUT && TerminateProcess(process_handle.get(), 1)) {
        WaitForSingleObject(process_handle.get(), 5000);
    }
    ASSERT_EQ(wait_result, WAIT_OBJECT_0) << "helperCli did not complete todo processing";
    DWORD exit_code = 0;
    ASSERT_TRUE(GetExitCodeProcess(process_handle.get(), &exit_code));
    std::ifstream log(output_path, std::ios::binary);
    ASSERT_TRUE(log.is_open());
    const std::string contents((std::istreambuf_iterator<char>(log)), std::istreambuf_iterator<char>());
    EXPECT_EQ(exit_code, 0u) << contents;
    EXPECT_NE(contents.find(">/exit"), std::string::npos) << contents;
    EXPECT_NE(contents.find("Exiting..."), std::string::npos) << contents;
    EXPECT_TRUE(fs::is_regular_file(workdir / "responses" / "lastResponses.txt"));
}

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
