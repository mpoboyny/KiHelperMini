#include "pre.hxx"

#include "cli-context.hxx"
#include "cli-ui.hxx"
#include "RunHandler.hxx"

#include "arg.h"
#include "base64.hpp"
#include "log.h"
#include "console.h"

#include "json.h"

#include <algorithm>
#include <cctype>
#include <cerrno>
#include <filesystem>
#include <fstream>
#include <map>
#include <set>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

using json = common_json;

struct cli_context_impl {
    json messages      = json::array();
    json pending_media = json::array(); // staged multimodal content parts
};

cli_context::cli_context(const common_params & params) : params(params), impl(new cli_context_impl()) {}

cli_context::~cli_context() {
    shutdown();
}

std::atomic<bool> & cli_context::interrupted() {
    static std::atomic<bool> flag = false;
    return flag;
}

static bool should_stop() {
    return cli_context::interrupted().load();
}

static constexpr size_t FILE_GLOB_MAX_RESULTS = 100;
static constexpr size_t FILE_TOOL_MAX_CALLS = 4;

static std::string edit_file_from_tool(const std::string & arguments, std::map<std::string, std::string> & loaded_files)
{
    json args = json::parse_no_throw(arguments);
    if (!args.is_object() || !args.contains("path") || !args.at("path").is_string() ||
        !args.contains("content") || !args.at("content").is_string()) {
        return "Error: expected a filename and replacement text.";
    }
    std::string path = args.at("path").get<std::string>();
    std::string content = args.at("content").get<std::string>();
    if (path.compare(0, 2, "./") == 0) {
        path.erase(0, 2);
    }
    if (path.empty() || path == "." || path == ".." || path.find('/') != std::string::npos ||
        path.find('\0') != std::string::npos) {
        return "Error: only a filename in the current directory is allowed.";
    }
    auto loaded = loaded_files.find(path);
    if (loaded == loaded_files.end()) {
        return "Error: load the file with /read before editing it.";
    }
    if (content.find("</parameter>") != std::string::npos &&
        content.find("</function>") != std::string::npos &&
        (content.find("<tool_call>") != std::string::npos || content.find("</tool_call>") != std::string::npos)) {
        return "Error: replacement contains malformed tool-call markup. No file was changed.";
    }

    int fd = open(path.c_str(), O_RDONLY | O_NOFOLLOW | O_NONBLOCK);
    if (fd < 0) {
        return std::string("Error opening file: ") + std::strerror(errno);
    }
    struct stat original;
    if (fstat(fd, &original) != 0 || !S_ISREG(original.st_mode)) {
        close(fd);
        return "Error: the file must be a regular file, not a symbolic link.";
    }
    std::string current;
    char buffer[4096];
    ssize_t count;
    while ((count = read(fd, buffer, sizeof(buffer))) != 0) {
        if (count < 0) {
            if (errno == EINTR) {
                continue;
            }
            int error = errno;
            close(fd);
            return std::string("Error reading file: ") + std::strerror(error);
        }
        current.append(buffer, static_cast<size_t>(count));
    }
    if (current != loaded->second) {
        close(fd);
        loaded->second = current;
        return "File changed since /read. No edit was applied. The current content has been reloaded below; "
            "review it and call edit_file again with the complete updated content.\n--- File: " + path + " ---\n" + current;
    }
    if (content == current) {
        close(fd);
        return "File is unchanged: " + path;
    }
    ui::show_message("Edit file in current directory: " + path + "\nCurrent content:\n" + current + "\nProposed content:\n" + content);
    ui::user_turn user_turn;
    std::string answer = string_strip(user_turn.read_input(false, "Replace this file's content? [y/N] "));
    if (should_stop() || (answer != "y" && answer != "Y" && answer != "yes" && answer != "YES")) {
        close(fd);
        return "File edit cancelled by user.";
    }

    char temp_path[] = ".kihelper-edit-XXXXXX";
    int temp_fd = mkstemp(temp_path);
    if (temp_fd < 0) {
        close(fd);
        return std::string("Error creating temporary file: ") + std::strerror(errno);
    }
    bool ok = fchmod(temp_fd, original.st_mode & 0777) == 0;
    size_t written = 0;
    while (ok && written < content.size()) {
        ssize_t n = write(temp_fd, content.data() + written, content.size() - written);
        if (n < 0 && errno == EINTR) {
            continue;
        }
        if (n <= 0) {
            ok = false;
        } else {
            written += static_cast<size_t>(n);
        }
    }
    if (ok && fsync(temp_fd) != 0) {
        ok = false;
    }
    if (close(temp_fd) != 0) {
        ok = false;
    }
    struct stat after, named;
    if (!ok || fstat(fd, &after) != 0 || lstat(path.c_str(), &named) != 0 ||
        after.st_dev != original.st_dev || after.st_ino != original.st_ino ||
        named.st_dev != original.st_dev || named.st_ino != original.st_ino ||
        after.st_size != original.st_size || after.st_mtim.tv_sec != original.st_mtim.tv_sec ||
        after.st_mtim.tv_nsec != original.st_mtim.tv_nsec || after.st_ctim.tv_sec != original.st_ctim.tv_sec ||
        after.st_ctim.tv_nsec != original.st_ctim.tv_nsec) {
        close(fd);
        unlink(temp_path);
        return "Error: file changed during confirmation or the replacement could not be prepared; no edit was applied.";
    }
    close(fd);
    if (rename(temp_path, path.c_str()) != 0) {
        int error = errno;
        unlink(temp_path);
        return std::string("Error replacing file: ") + std::strerror(error);
    }
    loaded->second = content;
    return "Updated file: " + path;
}

static std::string create_file_from_tool(const std::string & arguments, std::set<std::string> & created_files,
                                         const std::map<std::string, std::string> & loaded_files)
{
    json args = json::parse_no_throw(arguments);
    if (!args.is_object() || !args.contains("path") || !args.at("path").is_string() ||
        (args.contains("content") && !args.at("content").is_string())) {
        return "Error: expected a filename and optional text content.";
    }
    std::string path = args.at("path").get<std::string>();
    std::string content = args.value("content", std::string());
    if (path.empty() || path == "." || path == ".." || path.find('/') != std::string::npos ||
        path.find('\0') != std::string::npos) {
        return "Error: only a filename in the current directory is allowed.";
    }
    if (created_files.count(path)) {
        return "File already created during this request: " + path + ". No additional action was taken.";
    }
    if (content.find("</parameter>") != std::string::npos &&
        content.find("</function>") != std::string::npos &&
        (content.find("<tool_call>") != std::string::npos || content.find("</tool_call>") != std::string::npos)) {
        return "Error: file content contains malformed tool-call markup. No file was created; retry with the intended content only.";
    }

    struct stat existing;
    if (lstat(path.c_str(), &existing) == 0) {
        return loaded_files.count(path)
            ? "Error: file already exists; use edit_file instead: " + path
            : "Error: file already exists; load it with /read before editing: " + path;
    }
    if (errno != ENOENT) {
        return std::string("Error checking file: ") + std::strerror(errno);
    }

    ui::show_message("Create file in current directory: " + path + "\nContent:\n" + content);
    ui::user_turn user_turn;
    std::string answer = string_strip(user_turn.read_input(false, "Create this file? [y/N] "));
    if (should_stop() || (answer != "y" && answer != "Y" && answer != "yes" && answer != "YES")) {
        return "File creation cancelled by user.";
    }

    int fd = open(path.c_str(), O_WRONLY | O_CREAT | O_EXCL, 0666);
    if (fd < 0) {
        if (errno == EEXIST) {
            return loaded_files.count(path)
                ? "Error: file already exists; use edit_file instead: " + path
                : "Error: file already exists; load it with /read before editing: " + path;
        }
        return std::string("Error creating file: ") + std::strerror(errno);
    }
    size_t written = 0;
    while (written < content.size()) {
        ssize_t count = write(fd, content.data() + written, content.size() - written);
        if (count < 0 && errno == EINTR) {
            continue;
        }
        if (count <= 0) {
            int error = errno;
            close(fd);
            return std::string("Error writing file; a partial file may remain: ") + std::strerror(error);
        }
        written += static_cast<size_t>(count);
    }
    if (close(fd) != 0) {
        return std::string("Error closing file; the file may be incomplete: ") + std::strerror(errno);
    }
    created_files.insert(path);
    return "Created file: " + path;
}

const char * LLAMA_ASCII_LOGO =
    "\n"
    "▄▄ ▄▄\n"
    "██ ██\n"
    "██ ██  ▀▀█▄ ███▄███▄  ▀▀█▄    ▄████ ████▄ ████▄ \x1b[32mKiHelperMini\x1b[0m\n"
    "██ ██ ▄█▀██ ██ ██ ██ ▄█▀██    ██    ██ ██ ██ ██\n"
    "██ ██ ▀█▄██ ██ ██ ██ ▀█▄██ ██ ▀████ ████▀ ████▀\n"
    "                                    ██    ██\n"
    "                                    ▀▀    ▀▀\n";

// number of values an arg consumes on the command line
static int arg_num_values(const common_arg & opt) {
    if (opt.value_hint_2 != nullptr) {
        return 2;
    }
    if (opt.value_hint != nullptr) {
        return 1;
    }
    return 0;
}

static std::string format_error_message(const json & err) {
    if (err.contains("error") && err.at("error").is_object()) {
        const auto & e = err.at("error");
        if (e.contains("message") && e.at("message").is_string()) {
            return e.at("message").get<std::string>();
        }
    }
    return err.dump();
}

// err is the raw response body of a failed request; it may or may not be JSON
static std::string format_error_message(const std::string & err) {
    json parsed = json::parse_no_throw(err);
    if (!parsed.is_discarded()) {
        return format_error_message(parsed);
    }
    return err;
}

static std::string media_type_from_ext(const std::string & fname) {
    std::string ext = std::filesystem::path(fname).extension().string();
    std::transform(ext.begin(), ext.end(), ext.begin(), [](unsigned char c) { return std::tolower(c); });
    if (ext == ".wav" || ext == ".mp3") {
        return "audio";
    }
    if (ext == ".mp4" || ext == ".avi" || ext == ".mkv" || ext == ".mov" || ext == ".webm") {
        return "video";
    }
    return "image";
}

bool cli_context::init() {
    ui::init(params);

    std::optional<ui::spinner> spinner;

    bool use_external_server = !params.server_base.empty();
    if (use_external_server) {
        std::string base = params.server_base;
        while (!base.empty() && base.back() == '/') {
            base.pop_back();
        }
        client.server_base = base;

        spinner.emplace("Connecting to server at " + base);
    } else {
        if (params.model.path.empty() && params.model.url.empty() &&
                params.model.hf_repo.empty() && params.model.docker_repo.empty()) {
            ui::show_error(
                "no model specified",
                "use -m <file.gguf> or -hf <user/repo> to run a local model,\n"
                "or --server-base <url> to connect to a running llama-server"
            );
            return false;
        }

        spinner.emplace("\n\nLoading model...");

        server.emplace();
        if (!server->start(params)) {
            ui::show_error("server start failed");
            return false;
        }
        if (!server->wait_ready(should_stop)) {
            if (!should_stop()) {
                ui::show_error("the server exited before becoming ready");
            }
            return false;
        }
        client.server_base = server->address();
    }

    // for --server-base this is the main availability check; for a spawned
    // server it is a cheap sanity check on top of the ready signal
    auto is_aborted = [this]() {
        return should_stop() || (server && !server->alive());
    };
    bool healthy = false;
    try {
        healthy = client.wait_health(is_aborted);
    } catch (const std::exception & e) {
        client.last_error = e.what();
    }
    if (!healthy) {
        if (!should_stop()) {
            ui::show_error(client.last_error);
        }
        return false;
    }

    if (use_external_server) {
        spinner.reset();
        try {
            if (!list_and_ask_models()) {
                return false;
            }
        } catch (const common_json_error & e) {
            ui::show_error(e.what());
            ui::show_message("This might be caused by an incorrect server-base endpoint URL");
            return false;
        } catch (const std::exception & e) {
            ui::show_error(e.what());
            return false;
        }

        // restore the spinner for the next step
        spinner.emplace("Waiting for server...");
    }

    fetch_server_props();

    if (!params.out_file.empty()) {
        output_file.emplace(params.out_file);
        if (!output_file->is_open()) {
            ui::show_error(string_format("failed to open output file '%s'", params.out_file.c_str()));
            return false;
        }
    }

    return true;
}

void cli_context::fetch_server_props() {
    try {
        json props = json::parse(client.get("/props"));
        model_name = props.value("model_alias", "");
        if (model_name.empty()) {
            const std::string path = props.value("model_path", "");
            if (!path.empty()) {
                model_name = std::filesystem::path(path).filename().string();
            }
        }
        model_ftype = props.value("model_ftype", "");
        build_info = props.value("build_info", "");
        if (props.contains("modalities") && props.at("modalities").is_object()) {
            const auto & modalities = props.at("modalities");
            has_vision = modalities.value("vision", false);
            has_audio  = modalities.value("audio", false);
            has_video  = modalities.value("video", false);
        }
    } catch (const std::exception & e) {
        // /props can be disabled on remote servers; not fatal
        LOG_DBG("failed to fetch /props: %s\n", e.what());
    }
}

bool cli_context::list_and_ask_models() {
    json resp = json::parse(client.get("/v1/models"));
    if (!resp.contains("data") || !resp.at("data").is_array()) {
        throw std::runtime_error("invalid response from /v1/models");
    }
    std::vector<std::string> models;
    std::vector<std::string> models_display;
    for (const auto & m : resp.at("data")) {
        if (!m.contains("id") || !m.at("id").is_string()) {
            continue;
        }
        std::string name = m.at("id").get<std::string>();
        std::string display = name;
        if (m.contains("aliases") && m.at("aliases").is_array()) {
            std::vector<std::string> aliases;
            for (const auto & a : m.at("aliases")) {
                if (a.is_string()) {
                    aliases.push_back(a.get<std::string>());
                }
            }
            if (!aliases.empty()) {
                display += " (" + string_join(aliases, ", ") + ")";
            }
        }
        models.push_back(name);
        models_display.push_back(display);
    }

    // only one model: use it without asking
    if (models.size() == 1) {
        model_name = models[0];
        client.model = model_name;
        return true;
    }

    std::string message = "\nAvailable models:";
    for (size_t i = 0; i < models_display.size(); ++i) {
        message += "\n  " + std::to_string(i + 1) + ". " + models_display[i];
    }
    message += "\n";
    ui::show_message(message);
    std::string selection;
    while (selection.empty()) {
        if (should_stop()) {
            return false;
        }
        ui::user_turn user_turn;
        selection = user_turn.read_input(false, "Select model by number: ");
        if (selection.empty()) {
            return false;
        }
        try {
            size_t idx = std::stoul(selection);
            if (idx > 0 && idx <= models.size()) {
                model_name = models[idx - 1];
                client.model = model_name;
                ui::show_message("Selected model: " + model_name);
                break;
            }
        } catch (...) {
            // ignore
        }
        ui::show_error("Invalid selection. Please enter a valid number.");
        selection.clear();
        continue;
    }
    return true;
}

void cli_context::add_system_prompt() {
    if (!params.system_prompt.empty()) {
        impl->messages.push_back({
            {"role",    "system"},
            {"content", params.system_prompt}
        });
    }
}

void cli_context::push_user_message(const std::string & text) {
    json content;
    if (impl->pending_media.empty()) {
        content = text;
    } else {
        // multimodal message: media parts first, then the text
        content = impl->pending_media;
        content.push_back({
            {"type", "text"},
            {"text", text}
        });
        impl->pending_media = json::array();
    }
    impl->messages.push_back({
        {"role",    "user"},
        {"content", content}
    });
}

bool cli_context::stage_media_file(const std::string & fname, const std::string & type) {
    std::ifstream file(fname, std::ios::binary);
    if (!file) {
        return false;
    }
    std::string data((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    std::string encoded = base64::encode(data);

    if (type == "audio") {
        std::string ext = std::filesystem::path(fname).extension().string();
        std::transform(ext.begin(), ext.end(), ext.begin(), [](unsigned char c) { return std::tolower(c); });
        impl->pending_media.push_back({
            {"type", "input_audio"},
            {"input_audio", {
                {"data",   encoded},
                {"format", ext == ".mp3" ? "mp3" : "wav"}
            }}
        });
    } else if (type == "video") {
        impl->pending_media.push_back({
            {"type", "input_video"},
            {"input_video", {
                {"data", encoded}
            }}
        });
    } else {
        // the server detects the actual image type from the data
        impl->pending_media.push_back({
            {"type", "image_url"},
            {"image_url", {
                {"url", "data:image/unknown;base64," + encoded}
            }}
        });
    }
    return true;
}

void cli_context::write_output_file(const std::string & content) {
    if (output_file) {
        (*output_file) << content;
        output_file->flush();
    }
}

bool cli_context::generate_completion(generated_content & content_out, cli_timings & timings, bool disable_thinking, bool allow_file_creation) {
    json body = {
        {"messages",          impl->messages},
        {"stream",            true},
        // in order to get timings even when we cancel mid-way
        {"timings_per_token", true},
    };
    if (disable_thinking) {
        body["chat_template_kwargs"] = {{"enable_thinking", false}};
    }
    if (allow_file_creation) {
        body["parallel_tool_calls"] = false;
        body["tools"] = json::array({{
            {"type", "function"},
            {"function", {
                {"name", "create_file"},
                {"description", "Create a new file in the current directory after user confirmation. Never overwrites an existing file."},
                {"parameters", {
                    {"type", "object"},
                    {"properties", {
                        {"path", {{"type", "string"}, {"description", "Filename in the current directory"}}},
                        {"content", {{"type", "string"}, {"description", "File text; use an empty string for an empty file"}}}
                    }},
                    {"required", json::array({"path", "content"})}
                }}
            }}
        }});
        body["tools"].push_back({
            {"type", "function"},
            {"function", {
                {"name", "edit_file"},
                {"description", "Replace the complete content of an existing file loaded with /read, after user confirmation. Only filenames in the current directory are allowed."},
                {"parameters", {
                    {"type", "object"},
                    {"properties", {
                        {"path", {{"type", "string"}, {"description", "Filename previously loaded with /read"}}},
                        {"content", {{"type", "string"}, {"description", "Complete replacement file content, preserving unchanged parts"}}}
                    }},
                    {"required", json::array({"path", "content"})}
                }}
            }}
        });
    }
    if (!client.model.empty()) {
        body["model"] = client.model;
    }

    bool stream_error = false;

    ui::assistant_turn a;

    std::string err = client.post_sse("/v1/chat/completions", body.dump(), should_stop, [&](const std::string & payload) {
        json chunk = json::parse_no_throw(payload);
        if (chunk.is_discarded()) {
            return;
        }
        if (chunk.contains("error")) {
            stream_error = true;
            ui::show_error(format_error_message(chunk));
            return;
        }
        if (chunk.contains("timings")) {
            const auto & t = chunk.at("timings");
            timings.prompt_per_second    = t.value("prompt_per_second",    0.0);
            timings.predicted_per_second = t.value("predicted_per_second", 0.0);
        }
        if (!chunk.contains("choices") || !chunk.at("choices").is_array() || chunk.at("choices").empty()) {
            return;
        }
        const auto & choice = chunk.at("choices").at(0);
        if (!choice.contains("delta")) {
            return;
        }
        const auto & delta = choice.at("delta");
        if (delta.contains("tool_calls") && delta.at("tool_calls").is_array()) {
            if (!allow_file_creation) {
                stream_error = true;
                return;
            }
            for (const auto & call : delta.at("tool_calls")) {
                if (!call.is_object() || !call.contains("index") || !call.at("index").is_number_integer()) {
                    stream_error = true;
                    continue;
                }
                int index = call.at("index").get<int>();
                if (index < 0 || static_cast<size_t>(index) >= FILE_TOOL_MAX_CALLS) {
                    stream_error = true;
                    continue;
                }
                if (content_out.tool_calls.size() <= static_cast<size_t>(index)) {
                    content_out.tool_calls.resize(static_cast<size_t>(index) + 1);
                }
                auto & target = content_out.tool_calls[static_cast<size_t>(index)];
                if (call.contains("id") && call.at("id").is_string()) {
                    target.id += call.at("id").get<std::string>();
                }
                if (call.contains("function") && call.at("function").is_object()) {
                    const auto & function = call.at("function");
                    if (function.contains("name") && function.at("name").is_string()) {
                        target.name += function.at("name").get<std::string>();
                    }
                    if (function.contains("arguments") && function.at("arguments").is_string()) {
                        target.arguments += function.at("arguments").get<std::string>();
                    }
                }
            }
        }
        if (delta.contains("reasoning_content") && delta.at("reasoning_content").is_string()) {
            const std::string text = delta.at("reasoning_content").get<std::string>();
            if (!text.empty()) {
                content_out.reasoning += text;
                a.push(ui::ASSISTANT_DISPLAY_MODE_REASONING, text);
            }
        }
        if (delta.contains("content") && delta.at("content").is_string()) {
            const std::string text = delta.at("content").get<std::string>();
            if (!text.empty()) {
                content_out.content += text;
                a.push(ui::ASSISTANT_DISPLAY_MODE_CONTENT, text);
            }
        }
    });

    bool interrupted = should_stop();
    cli_context::interrupted().store(false);

    if (!err.empty()) {
        ui::show_error(format_error_message(err));
        return false;
    }
    if (stream_error) {
        ui::show_error("Invalid or failed completion stream.");
    }
    return !stream_error && !interrupted;
}

int cli_context::run() {
    add_system_prompt();

    std::string modalities = "text";
    if (has_vision) {
        modalities += ", vision";
    }
    if (has_audio) {
        modalities += ", audio";
    }
    if (has_video) {
        modalities += ", video";
    }

    std::string banner;
    banner += "\n";
    banner += LLAMA_ASCII_LOGO;
    banner += "\n";
    banner += "build      : " + build_info + "\n";
    banner += "model      : " + model_name + "\n";
    if (!model_ftype.empty()) {
        banner += "ftype      : " + model_ftype + "\n";
    }
    banner += "modalities : " + modalities + "\n";
    if (!params.system_prompt.empty()) {
        banner += "using custom system prompt\n";
    }
    banner += "\n";
    banner += "available commands:\n";
    banner += "  /exit or Ctrl+C      stop or exit\n";
    banner += "  /regen               regenerate the last response\n";
    banner += "  /clear               clear the chat history\n";
    banner += "  /read <file>         add a text file\n";
    banner += "  /run or /# <command> run a command\n";
    banner += "  /glob <pattern>      add text files using globbing pattern\n";
    if (has_vision) {
        banner += "  /image <file>       add an image file\n";
    }
    if (has_audio) {
        banner += "  /audio <file>       add an audio file\n";
    }
    if (has_video) {
        banner += "  /video <file>       add a video file\n";
    }
    banner += "\n";

    ui::show_message(banner);

    // interactive loop
    std::string cur_msg;
    std::map<std::string, std::string> loaded_files;

    auto add_text_file = [&](const std::string & fname) -> bool {
        std::ifstream file(fname, std::ios::binary);
        if (!file) {
            ui::show_error(string_format("file does not exist or cannot be opened: '%s'", fname.c_str()));
            return false;
        }
        std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        if (file.bad()) {
            ui::show_error(string_format("failed to read file: '%s'", fname.c_str()));
            return false;
        }
        std::string name = fname;
        if (name.compare(0, 2, "./") == 0) {
            name.erase(0, 2);
        }
        if (!name.empty() && name != "." && name != ".." && name.find('/') == std::string::npos) {
            loaded_files[name] = content;
        }
        cur_msg += "--- File: ";
        cur_msg += fname;
        cur_msg += " ---\n";
        cur_msg += content;
        ui::show_message(string_format("Loaded text from '%s'", fname.c_str()));
        return true;
    };

    while (true) {
        std::string buffer;
        {
            ui::user_turn user_turn;

            if (params.prompt.empty()) {
                buffer = user_turn.read_input(params.multiline_input);
            } else {
                // process input prompt from args
                for (auto & fname : params.image) {
                    if (!stage_media_file(fname, media_type_from_ext(fname))) {
                        ui::show_error(string_format("file does not exist or cannot be opened: '%s'", fname.c_str()));
                        break;
                    }
                    ui::show_message(string_format("Loaded media from '%s'", fname.c_str()));
                }
                buffer = params.prompt;
                user_turn.echo(buffer);
                params.prompt.clear(); // only use it once
            }
        }

        if (should_stop()) {
            cli_context::interrupted().store(false);
            break;
        }

        // a submitted line always ends with a newline, an empty read is EOF
        if (buffer.empty()) {
            break;
        }

        // remove trailing newline
        if (buffer.back() == '\n') {
            buffer.pop_back();
        }

        // skip empty messages
        if (buffer.empty()) {
            continue;
        }

        bool add_user_msg = true;
        bool disable_thinking = false;
        size_t run_pos = buffer.find('/');
        size_t run_length = 0;
        while (run_pos != std::string::npos) {
            run_length = buffer.compare(run_pos, 4, "/run") == 0 ? 4 :
                (buffer.compare(run_pos, 2, "/#") == 0 ? 2 : 0);
            if (run_length != 0 &&
                (run_pos == 0 || std::isspace(static_cast<unsigned char>(buffer[run_pos - 1]))) &&
                (run_pos + run_length == buffer.size() || std::isspace(static_cast<unsigned char>(buffer[run_pos + run_length])))) {
                break;
            }
            run_pos = buffer.find('/', run_pos + 1);
        }

        // process commands
        if (string_starts_with(buffer, "/exit")) {
            break;
        } else if (string_starts_with(buffer, "/regen")) {
            if (impl->messages.size() >= 2) {
                size_t last_idx = impl->messages.size() - 1;
                impl->messages.erase(last_idx);
                add_user_msg = false;
            } else {
                ui::show_error("No message to regenerate.");
                continue;
            }
        } else if (string_starts_with(buffer, "/clear")) {
            impl->messages.clear();
            add_system_prompt();

            impl->pending_media = json::array();
            loaded_files.clear();
            ui::show_message("Chat history cleared.");
            continue;
        } else if (
                (string_starts_with(buffer, "/image ") && has_vision) ||
                (string_starts_with(buffer, "/audio ") && has_audio) ||
                (string_starts_with(buffer, "/video ") && has_video)) {
            std::string type = buffer.substr(1, 5);
            // just in case (bad copy-paste for example), we strip all trailing/leading spaces
            std::string fname = string_strip(buffer.substr(7));
            if (!stage_media_file(fname, type)) {
                ui::show_error(string_format("file does not exist or cannot be opened: '%s'", fname.c_str()));
                continue;
            }
            ui::show_message(string_format("Loaded media from '%s'", fname.c_str()));
            write_output_file(string_format("User: Added media: %s\n", fname.c_str()));
            continue;
        } else if (string_starts_with(buffer, "/read ")) {
            std::string fname = string_strip(buffer.substr(6));
            add_text_file(fname);
            write_output_file(string_format("User: Added text file: %s\n", fname.c_str()));
            continue;
        } 
        else if (run_pos != std::string::npos && (run_pos == 0 || buffer.front() != '/')) {
            std::string fname = string_strip(buffer.substr(run_pos + run_length));
            std::string question = string_strip(buffer.substr(0, run_pos));
            if (!fname.empty() && fname.front() == '"') {
                std::string command;
                size_t end = 1;
                while (end < fname.size() && fname[end] != '"') {
                    if (fname[end] == '\\' && end + 1 < fname.size() &&
                        (fname[end + 1] == '"' || fname[end + 1] == '\\')) {
                        ++end;
                    }
                    command += fname[end++];
                }
                if (end == fname.size()) {
                    ui::show_error("Missing closing quote in command.");
                    continue;
                }
                std::string suffix = string_strip(fname.substr(end + 1));
                if (!question.empty() && !suffix.empty()) {
                    question += ' ';
                }
                question += suffix;
                fname = command;
            }
            RunHandler run;
            std::string result = run.Handle(fname);
            ui::show_message("Command output:\n" + result);
            ui::show_message(run.Status());
            size_t line_count = std::count(result.begin(), result.end(), '\n');
            if (!result.empty() && result.back() != '\n') {
                ++line_count;
            }
            ui::show_message("Command output lines: " + std::to_string(line_count));
            if (!cur_msg.empty() && cur_msg.back() != '\n') {
                cur_msg += '\n';
            }
            if (!question.empty()) {
                cur_msg += "User question: " + question + "\n";
            }
            cur_msg += "The following is the captured output (stdout and stderr) of the command: " + fname + "\n";
            cur_msg += run.Status() + "\n";
            cur_msg += "Output line count: " + std::to_string(line_count) + ". Lines are not necessarily files.\n";
            cur_msg += "Answer the user's question directly using the command and output. Do not invent errors or other facts.\n";
            cur_msg += result;
            if (question.empty()) {
                push_user_message(cur_msg);
                write_output_file(string_format("User:\n%s\n\n", cur_msg.c_str()));
                cur_msg.clear();
                std::string summary = "Command finished. " + run.Status();
                impl->messages.push_back({{"role", "assistant"}, {"content", summary}});
                if (result.empty() && run.Succeeded()) {
                    ui::show_message("Command completed successfully with no output.");
                }
                write_output_file("Assistant:\n" + summary + "\n\n");
                if (params.single_turn) {
                    break;
                }
                continue;
            }
            disable_thinking = true;
        } else if (string_starts_with(buffer, "/glob ")) {
            std::error_code ec;
            size_t count = 0;
            auto curdir = std::filesystem::current_path();
            std::string pattern = string_strip(buffer.substr(6));
            std::filesystem::path rel_path;

            auto startglob = pattern.find_first_of("![*?");
            if (startglob != std::string::npos && startglob != 0) {
                auto endpath = pattern.substr(0, startglob).find_last_of('/');
                if (endpath != std::string::npos) {
                    std::string rel_pattern = pattern.substr(0, endpath);
#if !defined(_WIN32)
                    if (string_starts_with(rel_pattern, '~')) {
                        const char * home = std::getenv("HOME");
                        if (home && home[0]) {
                            rel_pattern = home + rel_pattern.substr(1);
                        }
                    }
#endif
                    rel_path = rel_pattern;
                    pattern.erase(0, endpath + 1);
                    curdir /= rel_path;
                }
            }

            for (const auto & entry : std::filesystem::recursive_directory_iterator(curdir,
                    std::filesystem::directory_options::skip_permission_denied, ec)) {
                if (!entry.is_regular_file()) {
                    continue;
                }

                std::string rel = std::filesystem::relative(entry.path(), curdir, ec).string();
                if (ec) {
                    ec.clear();
                    continue;
                }
                std::replace(rel.begin(), rel.end(), '\\', '/');

                if (!glob_match(pattern, rel)) {
                    continue;
                }

                const std::string full_path = (curdir / rel).string();
                if (!add_text_file(full_path)) {
                    continue;
                }
                write_output_file(string_format("User: Added text file: %s\n", full_path.c_str()));

                if (++count >= FILE_GLOB_MAX_RESULTS) {
                    ui::show_error(string_format("Maximum number of globbed files allowed (%zu) reached.", FILE_GLOB_MAX_RESULTS));
                    break;
                }
            }
            continue;
        } else {
            // not a command
            cur_msg += buffer;
        }

        // generate response
        if (add_user_msg) {
            push_user_message(cur_msg);
            write_output_file(string_format("User:\n%s\n\n", cur_msg.c_str()));
            cur_msg.clear();
        }

        cli_timings timings;
        std::set<std::string> created_files;
        for (size_t tool_round = 0; tool_round <= FILE_TOOL_MAX_CALLS; ++tool_round) {
            generated_content content;
            if (!generate_completion(content, timings, disable_thinking,
                    !disable_thinking && tool_round < FILE_TOOL_MAX_CALLS)) {
                break;
            }

            json assistant_msg = {
                {"role",    "assistant"},
                {"content", content.content}
            };
            if (!content.reasoning.empty()) {
                assistant_msg["reasoning_content"] = content.reasoning;
            }
            if (!content.tool_calls.empty()) {
                assistant_msg["tool_calls"] = json::array();
                for (const auto & call : content.tool_calls) {
                    assistant_msg["tool_calls"].push_back({
                        {"id", call.id},
                        {"type", "function"},
                        {"function", {{"name", call.name}, {"arguments", call.arguments}}}
                    });
                }
            }
            impl->messages.push_back(std::move(assistant_msg));

            if (output_file) {
                std::string out_content = "Assistant:\n";
                if (!content.reasoning.empty()) {
                    out_content += "[Start thinking]\n\n";
                    out_content += content.reasoning;
                    out_content += "[End thinking]\n\n";
                }
                out_content += content.content;
                if (!out_content.empty() && out_content.back() != '\n') {
                    out_content += "\n";
                }
                out_content += "\n";
                write_output_file(out_content);
            }

            if (content.tool_calls.empty()) {
                break;
            }
            for (const auto & call : content.tool_calls) {
                std::string result;
                if (tool_round >= FILE_TOOL_MAX_CALLS || call.id.empty()) {
                    result = "Error: invalid tool call or tool call limit reached.";
                } else if (call.name == "create_file") {
                    result = create_file_from_tool(call.arguments, created_files, loaded_files);
                } else if (call.name == "edit_file") {
                    result = edit_file_from_tool(call.arguments, loaded_files);
                } else {
                    result = "Error: unknown tool.";
                }
                ui::show_message(result);
                impl->messages.push_back({
                    {"role", "tool"},
                    {"tool_call_id", call.id},
                    {"content", result}
                });
            }
            if (should_stop()) {
                cli_context::interrupted().store(false);
                break;
            }
        }

        if (params.show_timings) {
            ui::show_info(string_format(
                "\n[ Prompt: %.1f t/s | Generation: %.1f t/s ]",
                timings.prompt_per_second,
                timings.predicted_per_second
            ));
        }

        if (params.single_turn) {
            break;
        }
    }

    ui::show_message("\n\nExiting...");

    return 0;
}

void cli_context::shutdown() {
    if (server) {
        server->stop();
        server.reset();
    }
    if (output_file) {
        output_file->close();
        output_file.reset();
    }
}
