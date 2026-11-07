#include <gtest/gtest.h>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <string>
#include "cli-text.hxx"
#include "common.hxx"

static std::string shell_quote(const std::string & value)
{
#ifdef _WIN32
    return '"' + value + '"';
#else
    std::string quoted = "'";
    for (char c : value) {
        quoted += c == '\'' ? "'\\''" : std::string(1, c);
    }
    return quoted + "'";
#endif
}

TEST(ReadText, ValidatesUtf8)
{
    EXPECT_TRUE(text_file::is_utf8("ASCII"));
    EXPECT_TRUE(text_file::is_utf8("\xC2\xA9\xE2\x82\xAC\xF0\x9F\x98\x80"));
    for (const std::string & invalid : {std::string("\xB8"), std::string("\xC0\x80"),
                                        std::string("\xE0\x80\x80"), std::string("\xED\xA0\x80"),
                                        std::string("\xF4\x90\x80\x80"), std::string("\xE2\x82")}) {
        EXPECT_FALSE(text_file::is_utf8(invalid));
    }
}

TEST(ReadText, PreservesValidUtf8)
{
    const std::string input = "Copyright \xC2\xA9";
    std::string output;
    bool converted = true;
    ASSERT_TRUE(text_file::to_utf8(input, output, converted));
    EXPECT_EQ(output, input);
    EXPECT_FALSE(converted);
}

TEST(ReadText, HandlesNonUtf8Input)
{
    const std::string input = "Copyright \xB8";
    std::string output = "unchanged";
    bool converted = false;
#ifdef _WIN32
    if (GetOEMCP() == CP_UTF8) {
        EXPECT_FALSE(text_file::to_utf8(input, output, converted));
    } else {
        ASSERT_TRUE(text_file::to_utf8(input, output, converted));
        EXPECT_TRUE(converted);
        EXPECT_TRUE(text_file::is_utf8(output));
        EXPECT_EQ(output.substr(0, 10), "Copyright ");
        if (GetOEMCP() == 850) {
            EXPECT_EQ(output, "Copyright \xC2\xA9");
        }
    }
#else
    EXPECT_FALSE(text_file::to_utf8(input, output, converted));
#endif
}

static void test_create_file(const std::string & model, const std::string & name)
{
    namespace fs = std::filesystem;
    const fs::path output_dir = fs::absolute(fs::path(g_TestOutputRoot) / "CreateAndEdit");
    const fs::path output_file = output_dir / (name + ".txt");
    const fs::path log_file = output_dir / (name + ".log");
#ifdef _WIN32
    const fs::path executable = fs::absolute("bin/release/helperCli.exe");
#else
    const fs::path executable = fs::absolute("bin/release/helperCli");
#endif
    ASSERT_TRUE(fs::is_regular_file(executable)) << executable;
    ASSERT_NO_THROW(fs::create_directories(output_dir));
    ASSERT_NO_THROW(fs::remove(output_file));

    const std::string expected = "Hello from helperCli";
    const std::string prompt = "Create new file named " + output_file.filename().string() +
        " containing exactly this text: " + expected;
#ifdef _WIN32
    const std::string cd = "cd /d ";
#else
    const std::string cd = "cd ";
#endif
    const std::string command = cd + shell_quote(output_dir.string()) + " && echo y | " +
        shell_quote(executable.string()) + " -m " + shell_quote(model) +
        " --simple-io --single-turn --no-warmup --reasoning off --temp 0 -c 4096 -n 512 -p " +
        shell_quote(prompt) + " > " + shell_quote(log_file.string()) + " 2>&1";
    const int result = std::system(command.c_str());

    std::ifstream log(log_file, std::ios::binary);
    const std::string log_content((std::istreambuf_iterator<char>(log)), std::istreambuf_iterator<char>());
    ASSERT_EQ(result, 0) << log_content;
    ASSERT_TRUE(fs::is_regular_file(output_file)) << log_content;
    std::ifstream file(output_file, std::ios::binary);
    ASSERT_TRUE(file.is_open()) << output_file;
    const std::string actual((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    EXPECT_EQ(actual, expected) << log_content;
}

static void test_read_file(const std::string & model, const std::string & name)
{
    namespace fs = std::filesystem;
    test_create_file(model, name);
    if (::testing::Test::HasFailure()) {
        return;
    }

    const fs::path output_dir = fs::absolute(fs::path(g_TestOutputRoot) / "CreateAndEdit");
    const fs::path input_file = output_dir / (name + ".input");
    const fs::path log_file = output_dir / (name + "_read.log");
    const std::string filename = name + ".txt";
#ifdef _WIN32
    const fs::path executable = fs::absolute("bin/release/helperCli.exe");
    const std::string cd = "cd /d ";
#else
    const fs::path executable = fs::absolute("bin/release/helperCli");
    const std::string cd = "cd ";
#endif
    {
        std::ofstream input(input_file);
        ASSERT_TRUE(input.is_open()) << input_file;
        input << "/read " << filename << "\n"
              << "What is the exact text in the file? Reply with only the file text.\n"
              << "/exit\n";
        input.close();
        ASSERT_TRUE(input) << input_file;
    }

    const std::string command = cd + shell_quote(output_dir.string()) + " && " +
        shell_quote(executable.string()) + " -m " + shell_quote(model) +
        " --simple-io --single-turn --no-warmup --reasoning off --temp 0 -c 4096 -n 512 < " +
        shell_quote(input_file.string()) + " > " + shell_quote(log_file.string()) + " 2>&1";
    const int result = std::system(command.c_str());

    std::ifstream log(log_file, std::ios::binary);
    const std::string log_content((std::istreambuf_iterator<char>(log)), std::istreambuf_iterator<char>());
    ASSERT_EQ(result, 0) << log_content;
    const std::string loaded_message = "Loaded text from '" + filename + "'";
    const size_t loaded_pos = log_content.find(loaded_message);
    ASSERT_NE(loaded_pos, std::string::npos) << log_content;
    EXPECT_NE(log_content.find("Processing prompt...", loaded_pos + loaded_message.size()), std::string::npos)
        << log_content;
    EXPECT_NE(log_content.find("Hello from helperCli", loaded_pos + loaded_message.size()), std::string::npos)
        << log_content;
}

static void test_edit_file(const std::string & model, const std::string & name)
{
    namespace fs = std::filesystem;
    test_create_file(model, name);
    if (::testing::Test::HasFailure()) {
        return;
    }

    const fs::path output_dir = fs::absolute(fs::path(g_TestOutputRoot) / "CreateAndEdit");
    const fs::path output_file = output_dir / (name + ".txt");
    const fs::path input_file = output_dir / (name + "_edit.input");
    const fs::path log_file = output_dir / (name + "_edit.log");
    const std::string filename = output_file.filename().string();
    const std::string expected = "Updated by helperCli";
#ifdef _WIN32
    const fs::path executable = fs::absolute("bin/release/helperCli.exe");
    const std::string cd = "cd /d ";
#else
    const fs::path executable = fs::absolute("bin/release/helperCli");
    const std::string cd = "cd ";
#endif
    {
        std::ofstream input(input_file);
        ASSERT_TRUE(input.is_open()) << input_file;
        input << "/read " << filename << "\n"
              << "Change " << filename << " by replacing its entire contents with exactly this text: " << expected << "\n"
              << "y\n"
              << "/exit\n";
        input.close();
        ASSERT_TRUE(input) << input_file;
    }

    const std::string command = cd + shell_quote(output_dir.string()) + " && " +
        shell_quote(executable.string()) + " -m " + shell_quote(model) +
        " --simple-io --single-turn --no-warmup --reasoning off --temp 0 -c 4096 -n 512 < " +
        shell_quote(input_file.string()) + " > " + shell_quote(log_file.string()) + " 2>&1";
    const int result = std::system(command.c_str());

    std::ifstream log(log_file, std::ios::binary);
    const std::string log_content((std::istreambuf_iterator<char>(log)), std::istreambuf_iterator<char>());
    ASSERT_EQ(result, 0) << log_content;
    ASSERT_NE(log_content.find("Loaded text from '" + filename + "'"), std::string::npos) << log_content;
    ASSERT_NE(log_content.find("Updated file: " + filename), std::string::npos) << log_content;
    std::ifstream file(output_file, std::ios::binary);
    ASSERT_TRUE(file.is_open()) << output_file;
    const std::string actual((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    EXPECT_EQ(actual, expected) << log_content;
}

TEST(CreateAndEdit, Test001_Qwen3dot5_0_8B_Q4_K_M_CreateFile)
{
    test_create_file(g_TestModel_Qwen_Qwen3dot5_0_8B_Q4_K_M, "qwen3_5_0_8b");
}

#if USE_BIG_MODELS
TEST(CreateAndEdit, Test002_Qwen3dot5_9B_Q4_K_M_CreateFile)
{
    test_create_file(g_TestModel_Qwen3dot5_9B_Q4_K_M, "qwen3_5_9b");
}
#endif

TEST(CreateAndEdit, Test003_Qwen3dot5_0_8B_Q4_K_M_ReadFile)
{
    test_read_file(g_TestModel_Qwen_Qwen3dot5_0_8B_Q4_K_M, "qwen3_5_0_8b_read");
}

#if USE_BIG_MODELS
TEST(CreateAndEdit, Test004_Qwen3dot5_9B_Q4_K_M_ReadFile)
{
    test_read_file(g_TestModel_Qwen3dot5_9B_Q4_K_M, "qwen3_5_9b_read");
}
#endif

TEST(CreateAndEdit, Test005_Qwen3dot5_0_8B_Q4_K_M_EditFile)
{
    test_edit_file(g_TestModel_Qwen_Qwen3dot5_0_8B_Q4_K_M, "qwen3_5_0_8b_edit");
}

#if USE_BIG_MODELS
TEST(CreateAndEdit, Test006_Qwen3dot5_9B_Q4_K_M_EditFile)
{
    test_edit_file(g_TestModel_Qwen3dot5_9B_Q4_K_M, "qwen3_5_9b_edit");
}
#endif
