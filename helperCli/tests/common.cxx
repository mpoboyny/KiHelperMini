#include <gtest/gtest.h>
#include <filesystem>
#include <cstdlib>
#include <string>
#include "common.hxx"

#ifdef _WIN32
std::string g_TestModel_Qwen_Qwen3dot5_0_8B_Q4_K_M;
#if USE_BIG_MODELS
std::string g_TestModel_Qwen3dot5_9B_Q4_K_M;
#endif

// Expand %NAME% environment variable references in a Windows path.
static std::string resolve_windows_path(const std::string & path)
{
    std::string result;
    result.reserve(path.size());
    size_t pos = 0;
    while (pos < path.size()) {
        const size_t begin = path.find('%', pos);
        if (begin == std::string::npos) {
            result.append(path, pos, std::string::npos);
            break;
        }
        result.append(path, pos, begin - pos);
        const size_t end = path.find('%', begin + 1);
        if (end == std::string::npos) {
            result.append(path, begin, std::string::npos);
            break;
        }
        const std::string name = path.substr(begin + 1, end - begin - 1);
        const char * value = std::getenv(name.c_str());
        if (value != nullptr) {
            result += value;
        } else {
            result.append(path, begin, end - begin + 1);
        }
        pos = end + 1;
    }
    return result;
}
#endif

class CommonEnvironment : public ::testing::Environment
{
public:

#ifdef _WIN32
    void SetUp() override
    {
        g_TestModel_Qwen_Qwen3dot5_0_8B_Q4_K_M = resolve_windows_path(g_TestModel_Qwen_Qwen3dot5_0_8B_Q4_K_M_In);
#if USE_BIG_MODELS
        g_TestModel_Qwen3dot5_9B_Q4_K_M = resolve_windows_path(g_TestModel_Qwen3dot5_9B_Q4_K_M_In);
#endif

        ASSERT_TRUE(std::filesystem::exists(g_TestModel_Qwen_Qwen3dot5_0_8B_Q4_K_M))
            << g_TestModel_Qwen_Qwen3dot5_0_8B_Q4_K_M;
#if USE_BIG_MODELS
        ASSERT_TRUE(std::filesystem::exists(g_TestModel_Qwen3dot5_9B_Q4_K_M))
            << g_TestModel_Qwen3dot5_9B_Q4_K_M;
#endif

        ASSERT_NO_THROW(std::filesystem::remove_all(g_TestOutputRoot));
        ASSERT_NO_THROW(std::filesystem::create_directories(g_TestOutputRoot));
        ASSERT_TRUE(std::filesystem::is_directory(g_TestOutputRoot)) << g_TestOutputRoot;
        ASSERT_TRUE(std::filesystem::is_empty(g_TestOutputRoot)) << g_TestOutputRoot;
    }
#else
    void SetUp() override
    {
        ASSERT_TRUE(std::filesystem::exists(g_TestModel_Qwen_Qwen3dot5_0_8B_Q4_K_M))
            << g_TestModel_Qwen_Qwen3dot5_0_8B_Q4_K_M;
#if USE_BIG_MODELS
        ASSERT_TRUE(std::filesystem::exists(g_TestModel_Qwen3dot5_9B_Q4_K_M))
            << g_TestModel_Qwen3dot5_9B_Q4_K_M;
#endif

        ASSERT_NO_THROW(std::filesystem::remove_all(g_TestOutputRoot));
        ASSERT_NO_THROW(std::filesystem::create_directories(g_TestOutputRoot));
        ASSERT_TRUE(std::filesystem::is_directory(g_TestOutputRoot)) << g_TestOutputRoot;
        ASSERT_TRUE(std::filesystem::is_empty(g_TestOutputRoot)) << g_TestOutputRoot;
    }
#endif

};

TEST(COMMON, EnvironmentReady)
{
    ASSERT_TRUE(std::filesystem::is_directory(g_TestOutputRoot)) << g_TestOutputRoot;
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    ::testing::AddGlobalTestEnvironment(new CommonEnvironment);
    return RUN_ALL_TESTS();
}
