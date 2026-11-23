#pragma once

#include <vector>
#include <string>
#include <string_view>

class LogConfigurator
{
public:
    /**
     * Configure the logging system based on command line arguments.
     * If --log-file is provided and verbosity is requested, disable console output.
     */
    static void configure(int argc, char** argv);

private:
    static bool has_argument(int argc, char** argv, std::string_view arg);
    static bool is_env_set(std::string_view env_var);
};
