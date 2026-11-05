#ifndef COMMON_HXX
#define COMMON_HXX  

#define USE_BIG_MODELS 0

#ifdef _WIN32
    const std::string g_TestModel_Qwen_Qwen3dot5_0_8B_Q4_K_M_In = "%USERPROFILE%\\LocalKiModels\\Qwen_Qwen3.5-0.8B-Q4_K_M.gguf";
    extern std::string g_TestModel_Qwen_Qwen3dot5_0_8B_Q4_K_M;
    #if USE_BIG_MODELS
        const std::string g_TestModel_Qwen3dot5_9B_Q4_K_M_In = "%USERPROFILE%\\LocalKiModels\\Qwen3.5-9B-Q4_K_M.gguf";
        extern std::string g_TestModel_Qwen3dot5_9B_Q4_K_M;
    #endif
#else
    const std::string g_TestModel_Qwen_Qwen3dot5_0_8B_Q4_K_M = "/home/tato/LocalKiModels/Qwen_Qwen3.5-0.8B-Q4_K_M.gguf";
    #if USE_BIG_MODELS
        const std::string g_TestModel_Qwen3dot5_9B_Q4_K_M = "/home/tato/LocalKiModels/Qwen_Qwen3.5-9B-Q4_K_M.gguf";
    #endif
#endif

const std::string g_TestOutputRoot = "bin/tests/release/TestsOutput";

#endif // COMMON_HXX
