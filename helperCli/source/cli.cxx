#include "pre.hxx"

#include "arg.h"
#include "common.h"
#include "log.h"

#include "cli-context.hxx"
#include "cli-todo.hxx"

#include <filesystem>
#include <fstream>
#include <signal.h>
#include <vector>

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#ifndef NOMINMAX
#   define NOMINMAX
#endif
#include <windows.h>
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

int llama_cli(int argc, char ** argv) {
    std::vector<char *> forwarded_args;
    forwarded_args.push_back(argv[0]);
    std::string todo_path;
    bool todo_mode = false;
    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];
        const bool split_option = arg == "-" && i + 1 < argc && std::string(argv[i + 1]) == "todo";
        if (arg == "-todo" || arg == "--todo" || split_option) {
            if (todo_mode) {
                fprintf(stderr, "The todo option may only be specified once.\n");
                return 1;
            }
            if (split_option) {
                ++i;
            }
            if (i + 1 >= argc || argv[i + 1][0] == '\0' || argv[i + 1][0] == '-') {
                fprintf(stderr, "The todo option requires a file path.\n");
                return 1;
            }
            todo_mode = true;
            todo_path = argv[++i];
        } else {
            forwarded_args.push_back(argv[i]);
        }
    }
    forwarded_args.push_back(nullptr);

    std::vector<std::string> todo_prompts;
    if (todo_mode) {
        std::ifstream input(std::filesystem::u8path(todo_path), std::ios::binary);
        if (!input) {
            fprintf(stderr, "Cannot open todo file: %s\n", todo_path.c_str());
            return 1;
        }
        std::string error;
        if (!todo::read_prompts(input, todo_prompts, error)) {
            fprintf(stderr, "%s\n", error.c_str());
            return 1;
        }
    }

    common_params params;

    params.verbosity = LOG_LEVEL_ERROR; // by default, less verbose logs

    common_init();

    if (!common_params_parse(static_cast<int>(forwarded_args.size() - 1), forwarded_args.data(), params, LLAMA_EXAMPLE_CLI)) {
        return 1;
    }

    if (todo_mode) {
#ifdef _WIN32
        const std::filesystem::path output_path = L"C:\\Temp\\lastResponses.txt";
#else
        const std::filesystem::path output_path = "/tmp/lastResponses.txt";
#endif
        std::error_code ec;
        const bool same_file = std::filesystem::equivalent(std::filesystem::u8path(todo_path), output_path, ec);
        if (!ec && same_file) {
            fprintf(stderr, "The todo file must not be the response file.\n");
            return 1;
        }
        ec.clear();
        std::filesystem::create_directories(output_path.parent_path(), ec);
        if (ec) {
            fprintf(stderr, "Cannot create response directory: %s\n", ec.message().c_str());
            return 1;
        }
        params.out_file = output_path.u8string();
        params.prompt.clear();
        params.single_turn = false;
    }

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
    ctx_cli.todo_mode = todo_mode;
    ctx_cli.todo_prompts = std::move(todo_prompts);

    if (!ctx_cli.init()) {
        return 1;
    }

    return ctx_cli.run();
}
