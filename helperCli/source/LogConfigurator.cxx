#include "pre.hxx"
#include "LogConfigurator.hxx"
#include "log.h"
#include <algorithm>
#include <cstdlib>
#include <cstring>


void LogConfigurator::configure(int argc, char** argv)
{
    bool has_log_file = has_argument(argc, argv, "--log-file") || is_env_set("LLAMA_ARG_LOG_FILE");
    bool has_verbose = has_argument(argc, argv, "-v") || has_argument(argc, argv, "--verbose");

    if (has_log_file && has_verbose)
    {
        common_log_set_stdo(common_log_main(), false);
    }
}

bool LogConfigurator::has_argument(int argc, char** argv, std::string_view arg)
{
    for (int i = 1; i < argc; ++i)
    {
        if (argv[i] && std::string_view(argv[i]).find(arg) == 0)
        {
            return true;
        }
    }
    return false;
}

bool LogConfigurator::is_env_set(std::string_view env_var)
{
    const char* val = std::getenv(env_var.data());
    return val != nullptr && std::strlen(val) > 0;
}
