#include "pre.hxx"

#include "arg.h"
#include "common.h"
#include "log.h"

#include "cli-context.hxx"

#include <cstdlib>
#include <signal.h>
#include <vector>

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#ifndef NOMINMAX
#   define NOMINMAX
#endif
#include <windows.h>

static bool expand_model_path(std::string & path)
{
    if (path.find('%') == std::string::npos) {
        return true;
    }
    DWORD size = ExpandEnvironmentStringsA(path.c_str(), nullptr, 0);
    while (size != 0) {
        std::vector<char> expanded(size);
        const DWORD length = ExpandEnvironmentStringsA(path.c_str(), expanded.data(), size);
        if (length == 0) {
            return false;
        }
        if (length <= size) {
            path.assign(expanded.data(), length - 1);
            return true;
        }
        size = length;
    }
    return false;
}
#endif

#if defined (__unix__) || (defined (__APPLE__) && defined (__MACH__)) || defined (_WIN32)
static void signal_handler(int) {
    if (cli_context::interrupted().load()) {
        // second Ctrl+C - exit immediately
        // make sure to clear colors before exiting (not using LOG or console.cpp here to avoid deadlock)
        fprintf(stdout, "\033[0m\n");
        fflush(stdout);
        std::exit(130);
    }
    cli_context::interrupted().store(true);
}
#endif

// satisfies -Wmissing-declarations
int llama_cli(int argc, char ** argv);

static bool usage_requested(const int argc, char ** argv)
{
    for (int i = 1; i < argc; ++i) {
        if (std::strcmp(argv[i], "-h") == 0 || std::strcmp(argv[i], "--help") == 0 ||
            std::strcmp(argv[i], "--usage") == 0) {
            return true;
        }
    }
    return false;
}

static void print_input_commands()
{
    fprintf(stdout, "----- input commands -----\n\n");
    fprintf(stdout, "  /exit or Ctrl+C      stop or exit\n");
    fprintf(stdout, "  /regen               regenerate the last response\n");
    fprintf(stdout, "  /clear               clear the chat history\n");
    fprintf(stdout, "  /read <file>         add a text file\n");
    fprintf(stdout, "  /run or /# <command> run a command\n");
    fprintf(stdout, "  /glob <pattern>      add text files using globbing pattern\n");
    fprintf(stdout, "  /image <file>        add an image file when supported\n");
    fprintf(stdout, "  /audio <file>        add an audio file when supported\n");
    fprintf(stdout, "  /video <file>        add a video file when supported\n\n");
}

int llama_cli(int argc, char ** argv) {
    if (usage_requested(argc, argv)) {
        print_input_commands();
    }

    common_params params;

    params.verbosity = LOG_LEVEL_ERROR; // by default, less verbose logs

    common_init();

    if (!common_params_parse(argc, argv, params, LLAMA_EXAMPLE_CLI)) {
        return 1;
    }

#if defined(_WIN32)
    if (!expand_model_path(params.model.path)) {
        fprintf(stderr, "Cannot expand environment variables in the model path.\n");
        return 1;
    }
#elif defined(__linux__)
    if (params.model.path == "~" || params.model.path.compare(0, 2, "~/") == 0) {
        const char * home = std::getenv("HOME");
        if (home == nullptr || home[0] == '\0') {
            fprintf(stderr, "Cannot expand the model path: HOME is not set.\n");
            return 1;
        }
        params.model.path = std::string(home) + params.model.path.substr(1);
    }
#endif

    llama_backend_init();
    llama_numa_init(params.numa);

#if defined (__unix__) || (defined (__APPLE__) && defined (__MACH__))
    struct sigaction sigint_action;
    sigint_action.sa_handler = signal_handler;
    sigemptyset (&sigint_action.sa_mask);
    sigint_action.sa_flags = 0;
    sigaction(SIGINT, &sigint_action, NULL);
    sigaction(SIGTERM, &sigint_action, NULL);
#elif defined (_WIN32)
    auto console_ctrl_handler = +[](DWORD ctrl_type) -> BOOL {
        return (ctrl_type == CTRL_C_EVENT) ? (signal_handler(SIGINT), true) : false;
    };
    SetConsoleCtrlHandler(reinterpret_cast<PHANDLER_ROUTINE>(console_ctrl_handler), true);
#endif

    cli_context ctx_cli(params);

    if (!ctx_cli.init()) {
        return 1;
    }

    return ctx_cli.run();
}
