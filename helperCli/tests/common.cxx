#include <gtest/gtest.h>
#include <filesystem>
#include "common.hxx"

class CommonEnvironment : public ::testing::Environment
{
public:
    void SetUp() override
    {
        ASSERT_TRUE(std::filesystem::exists(g_TestModel_Qwen_Qwen3dot5_0_8B_Q4_K_M))
            << g_TestModel_Qwen_Qwen3dot5_0_8B_Q4_K_M;
        ASSERT_TRUE(std::filesystem::exists(g_TestModel_Qwen3dot5_9B_Q4_K_M))
            << g_TestModel_Qwen3dot5_9B_Q4_K_M;

        ASSERT_NO_THROW(std::filesystem::remove_all(g_TestOutputRoot));
        ASSERT_NO_THROW(std::filesystem::create_directories(g_TestOutputRoot));
        ASSERT_TRUE(std::filesystem::is_directory(g_TestOutputRoot)) << g_TestOutputRoot;
        ASSERT_TRUE(std::filesystem::is_empty(g_TestOutputRoot)) << g_TestOutputRoot;
    }
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
