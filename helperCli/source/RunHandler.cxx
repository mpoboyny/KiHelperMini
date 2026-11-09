#include "pre.hxx"
#include "RunHandler.hxx"

#include <cerrno>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <limits>
#include <set>
#if !defined(_WIN32)
#include <sys/wait.h>
#include <unistd.h>
#endif

static bool completion_starts_with(const std::string & text, const std::string & prefix)
{
    if (prefix.size() > text.size()) {
        return false;
    }
    for (size_t i = 0; i < prefix.size(); ++i) {
#if defined(_WIN32)
        if (std::tolower(static_cast<unsigned char>(text[i])) != std::tolower(static_cast<unsigned char>(prefix[i]))) {
#else
        if (text[i] != prefix[i]) {
#endif
            return false;
        }
    }
    return true;
}

static bool completion_safe_name(const std::string & name)
{
    for (size_t i = 0; i < name.size();) {
        const unsigned char first = static_cast<unsigned char>(name[i]);
        if (first < 0x80) {
            if (first < 0x20 || first == 0x7f) {
                return false;
            }
            ++i;
            continue;
        }
        size_t length = 0;
        if (first >= 0xc2 && first <= 0xdf) {
            length = 2;
        } else if (first >= 0xe0 && first <= 0xef) {
            length = 3;
        } else if (first >= 0xf0 && first <= 0xf4) {
            length = 4;
        }
        if (length == 0 || length > name.size() - i) {
            return false;
        }
        for (size_t j = 1; j < length; ++j) {
            if ((static_cast<unsigned char>(name[i + j]) & 0xc0) != 0x80) {
                return false;
            }
        }
        const unsigned char second = static_cast<unsigned char>(name[i + 1]);
        if ((first == 0xc2 && second < 0xa0) || (first == 0xe0 && second < 0xa0) ||
            (first == 0xed && second >= 0xa0) || (first == 0xf0 && second < 0x90) ||
            (first == 0xf4 && second >= 0x90)) {
            return false;
        }
        i += length;
    }
    return true;
}

static std::string completion_escape(const std::string & text, bool expand_tilde)
{
#if defined(_WIN32)
    (void) expand_tilde;
    if (text.find_first_of(" \t&|<>()\"") == std::string::npos) {
        return text;
    }
    return '"' + text + '"';
#else
    std::string result;
    for (size_t i = 0; i < text.size(); ++i) {
        const unsigned char ch = static_cast<unsigned char>(text[i]);
        if ((ch < 0x80 && !std::isalnum(ch) && text[i] != '_' && text[i] != '-' &&
             text[i] != '.' && text[i] != '/') && !(i == 0 && text[i] == '~' && expand_tilde)) {
            result += '\\';
        }
        result += text[i];
    }
    return result;
#endif
}

#if defined(_WIN32)
static bool completion_executable(const std::filesystem::path & path)
{
    const std::string extension = path.extension().u8string();
    const char * environment = std::getenv("PATHEXT");
    const std::string extensions = environment != nullptr ? environment : ".COM;.EXE;.BAT;.CMD";
    size_t start = 0;
    while (start <= extensions.size()) {
        const size_t end = extensions.find(';', start);
        const std::string candidate = extensions.substr(start, end == std::string::npos ? end : end - start);
        if (candidate.size() == extension.size() && completion_starts_with(candidate, extension)) {
            return true;
        }
        if (end == std::string::npos) {
            break;
        }
        start = end + 1;
    }
    return false;
}
#endif

std::vector<std::string> RunHandler::Complete(std::string & line, size_t & cursor, size_t command_start)
{
    std::vector<std::string> candidates;
    if (command_start > cursor || cursor > line.size()) {
        return candidates;
    }

    size_t token_start = command_start;
    bool command_word = true;
    bool token_started = false;
    bool escaped = false;
    char quote = 0;
    std::string token;
    for (size_t i = command_start; i < cursor; ++i) {
        const char ch = line[i];
#if !defined(_WIN32)
        if (escaped) {
            token += ch;
            escaped = false;
            token_started = true;
            continue;
        }
        if (ch == '\\' && quote != '\'' &&
            (quote == 0 || (i + 1 < cursor && std::strchr("$`\"\\\n", line[i + 1]) != nullptr))) {
            escaped = true;
            token_started = true;
            continue;
        }
#endif
        const bool quote_character = ch == '"'
#if !defined(_WIN32)
            || ch == '\''
#endif
            ;
        if (quote_character) {
            if (quote == 0) {
                quote = ch;
                token_started = true;
                continue;
            }
            if (quote == ch) {
                quote = 0;
                continue;
            }
        }
        if (quote == 0 && std::isspace(static_cast<unsigned char>(ch))) {
            if (token_started) {
                command_word = false;
            }
            token_start = i + 1;
            token.clear();
            token_started = false;
        } else if (quote == 0 && (ch == '|' || ch == ';' || ch == '&' || ch == '(' || ch == ')')) {
            command_word = true;
            token_start = i + 1;
            token.clear();
            token_started = false;
        } else if (quote == 0 && (ch == '<' || ch == '>')) {
            command_word = false;
            token_start = i + 1;
            token.clear();
            token_started = false;
        } else {
#if !defined(_WIN32)
            if ((ch == '$' || ch == '`') && quote != '\'') {
                return candidates;
            }
#else
            if (ch == '%') {
                return candidates;
            }
#endif
            token += ch;
            token_started = true;
        }
    }
    if (escaped) {
        return candidates;
    }

#if defined(_WIN32)
    const size_t separator = token.find_last_of("/\\");
    const bool expand_tilde = false;
#else
    const size_t separator = token.find_last_of('/');
    const bool expand_tilde = !token.empty() && token[0] == '~' &&
        token_start < line.size() && line[token_start] == '~' && (token.size() == 1 || token[1] == '/');
#endif
    if (expand_tilde && separator == std::string::npos) {
        const char * home = std::getenv("HOME");
        if (home != nullptr && home[0] != '\0') {
            line.replace(token_start, cursor - token_start, "~/");
            cursor = token_start + 2;
            candidates.emplace_back("~/");
        }
        return candidates;
    }
    const std::string directory_prefix = separator == std::string::npos ? "" : token.substr(0, separator + 1);
    const std::string name_prefix = separator == std::string::npos ? token : token.substr(separator + 1);
    std::set<std::string> matches;
    std::set<std::string> directories;

    auto add_directory = [&](const std::filesystem::path & directory, bool executables_only, const std::string & prefix) {
        std::error_code error;
        std::filesystem::directory_iterator entries(directory, error);
        const std::filesystem::directory_iterator end;
        while (!error && entries != end) {
            const auto & entry = *entries;
            const std::string name = entry.path().filename().u8string();
            const bool invalid_name = name.empty() || !completion_starts_with(name, name_prefix) ||
                (name[0] == '.' && (name_prefix.empty() || name_prefix[0] != '.')) ||
                !completion_safe_name(name)
#if defined(_WIN32)
                || name.find_first_of("%!\"") != std::string::npos
#endif
                ;
            if (invalid_name) {
                entries.increment(error);
                continue;
            }
            std::error_code type_error;
            const bool is_directory = entry.is_directory(type_error);
            if (executables_only) {
#if !defined(_WIN32)
                if (type_error || is_directory || !entry.is_regular_file(type_error) || type_error || access(entry.path().c_str(), X_OK) != 0) {
                    entries.increment(error);
                    continue;
                }
#else
                if (type_error || is_directory || !entry.is_regular_file(type_error) || type_error || !completion_executable(entry.path())) {
                    entries.increment(error);
                    continue;
                }
#endif
            }
            if (type_error) {
                entries.increment(error);
                continue;
            }
            std::string match = prefix + name;
            if (is_directory) {
#if defined(_WIN32)
                match += directory_prefix.find('/') != std::string::npos ? '/' : '\\';
#else
                match += '/';
#endif
                directories.insert(match);
            }
            matches.insert(std::move(match));
            entries.increment(error);
        }
    };

    if (command_word && separator == std::string::npos && !expand_tilde) {
#if defined(_WIN32)
        static const char * const builtins[] = {"cd", "cls", "copy", "del", "dir", "echo", "exit", "mkdir", "move", "ren", "rmdir", "set", "type"};
        add_directory(std::filesystem::path("."), true, "");
        constexpr char path_separator = ';';
#else
        static const char * const builtins[] = {"alias", "cd", "echo", "eval", "exec", "exit", "export", "jobs", "kill", "printf", "pwd", "read", "set", "source", "test", "type", "umask", "unalias", "unset", "wait"};
        constexpr char path_separator = ':';
#endif
        for (const char * name : builtins) {
            if (completion_starts_with(name, name_prefix)) {
                matches.insert(name);
            }
        }
        const char * path = std::getenv("PATH");
        if (path != nullptr) {
            const std::string paths(path);
            size_t start = 0;
            do {
                const size_t end = paths.find(path_separator, start);
                const std::string part = paths.substr(start, end == std::string::npos ? end : end - start);
                add_directory(std::filesystem::u8path(part.empty() ? "." : part), true, "");
                if (end == std::string::npos) {
                    break;
                }
                start = end + 1;
            } while (start <= paths.size());
        }
    } else {
        std::string path = directory_prefix.empty() ? "." : directory_prefix;
#if !defined(_WIN32)
        if (expand_tilde) {
            const char * home = std::getenv("HOME");
            if (home == nullptr || home[0] == '\0') {
                return candidates;
            }
            path = std::string(home) + directory_prefix.substr(1);
        }
#endif
        add_directory(std::filesystem::u8path(path), false, directory_prefix);
    }
    if (matches.empty()) {
        return candidates;
    }

    std::string common = *matches.begin();
    for (const std::string & match : matches) {
        size_t length = 0;
        while (length < common.size() && length < match.size() &&
#if defined(_WIN32)
               std::tolower(static_cast<unsigned char>(match[length])) == std::tolower(static_cast<unsigned char>(common[length]))
#else
               match[length] == common[length]
#endif
               ) {
            ++length;
        }
        common.resize(length);
    }
    while (!common.empty() && common.size() < matches.begin()->size() &&
           (static_cast<unsigned char>((*matches.begin())[common.size()]) & 0xc0) == 0x80) {
        common.pop_back();
    }
    for (const std::string & match : matches) {
        candidates.push_back(completion_escape(match, expand_tilde));
    }
    if (common.size() <= token.size() && matches.size() != 1) {
        return candidates;
    }
    std::string replacement = completion_escape(common, expand_tilde);
    const size_t token_end = quote != 0 && cursor < line.size() && line[cursor] == quote ? cursor + 1 : cursor;
    if (matches.size() == 1 && directories.count(common) == 0 && token_end == line.size()) {
        replacement += ' ';
    }
    line.replace(token_start, token_end - token_start, replacement);
    cursor = token_start + replacement.size();
    return candidates;
}

#if !defined(_WIN32)
std::string RunHandler::Handle(const std::string & args) {
    TrFu;
    status.clear();
    succeeded = false;
    if (args.empty()) {
        status = "No command was executed.";
        return "Hallo";
    }

    std::string command = "bash -c '";
    for (char ch : args) {
        command += ch == '\'' ? "'\\''" : std::string(1, ch);
    }
    command += "' 2>&1";
    FILE * pipe = popen(command.c_str(), "r");
    if (!pipe) {
        status = std::string("Failed to start process: ") + std::strerror(errno);
        return status;
    }

    std::string output;
    char buffer[4096];
    size_t count;
    while ((count = std::fread(buffer, 1, sizeof(buffer), pipe)) > 0) {
        output.append(buffer, count);
    }
    bool read_error = std::ferror(pipe);
    int result = pclose(pipe);
    if (result == -1) {
        status = std::string("Failed to wait for process: ") + std::strerror(errno);
    } else if (WIFEXITED(result)) {
        int exit_code = WEXITSTATUS(result);
        status = "Exit code: " + std::to_string(exit_code);
        succeeded = exit_code == 0 && !read_error;
    } else if (WIFSIGNALED(result)) {
        status = "Terminated by signal: " + std::to_string(WTERMSIG(result));
    } else {
        status = "Process ended with unknown status.";
    }
    if (read_error) {
        status += " (failed to read complete output)";
    }
    return output;
}
#else

namespace {

class unique_handle {
public:
    explicit unique_handle(HANDLE handle = nullptr) noexcept : handle_(handle) {}
    ~unique_handle() { reset(); }

    unique_handle(const unique_handle &) = delete;
    unique_handle & operator=(const unique_handle &) = delete;

    HANDLE get() const noexcept { return handle_; }
    void reset() noexcept {
        if (handle_ != nullptr && handle_ != INVALID_HANDLE_VALUE) {
            CloseHandle(handle_);
        }
        handle_ = nullptr;
    }

private:
    HANDLE handle_;
};

} // namespace

static std::wstring wide_from_utf8(const std::string & text)
{
    if (text.empty()) {
        return std::wstring();
    }
    const int size = MultiByteToWideChar(CP_UTF8, 0, text.data(), static_cast<int>(text.size()), nullptr, 0);
    std::wstring wide(static_cast<size_t>(size), L'\0');
    MultiByteToWideChar(CP_UTF8, 0, text.data(), static_cast<int>(text.size()), wide.data(), size);
    return wide;
}

static void read_pipe(HANDLE handle, std::string & output, bool & read_error)
{
    char buffer[4096];
    for (;;) {
        DWORD count = 0;
        if (!ReadFile(handle, buffer, sizeof(buffer), &count, nullptr)) {
            read_error = GetLastError() != ERROR_BROKEN_PIPE;
            break;
        }
        if (count == 0) {
            break;
        }
        output.append(buffer, count);
    }
}

static bool normalize_command_output(std::string & output)
{
    if (output.empty()) {
        return true;
    }
    if (output.size() > static_cast<size_t>(std::numeric_limits<int>::max())) {
        return false;
    }

    const int length = static_cast<int>(output.size());
    if (MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, output.data(), length, nullptr, 0) > 0) {
        return true;
    }

    const UINT code_page = GetOEMCP();
    if (code_page == CP_UTF8) {
        return false;
    }
    const int wide_length = MultiByteToWideChar(code_page, 0, output.data(), length, nullptr, 0);
    if (wide_length == 0) {
        return false;
    }
    std::wstring wide(static_cast<size_t>(wide_length), L'\0');
    if (MultiByteToWideChar(code_page, 0, output.data(), length, wide.data(), wide_length) == 0) {
        return false;
    }

    const int utf8_length = WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, wide.data(), wide_length,
                                                 nullptr, 0, nullptr, nullptr);
    if (utf8_length == 0) {
        return false;
    }
    std::string utf8(static_cast<size_t>(utf8_length), '\0');
    if (WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, wide.data(), wide_length,
                            utf8.data(), utf8_length, nullptr, nullptr) == 0) {
        return false;
    }
    output = std::move(utf8);
    return true;
}

std::string RunHandler::Handle(const std::string & args)
{
    TrFu;
    status.clear();
    succeeded = false;
    if (args.empty()) {
        status = "No command was executed.";
        return "Hallo";
    }

    std::string escaped;
    for (char ch : args) {
        if (ch == '"') {
            escaped += '\\';
        }
        escaped += ch;
    }
    std::wstring command = L"cmd.exe /d /s /c \"" + wide_from_utf8(escaped) + L"\"";

    SECURITY_ATTRIBUTES pipe_attrs;
    pipe_attrs.nLength = sizeof(pipe_attrs);
    pipe_attrs.bInheritHandle = TRUE;
    pipe_attrs.lpSecurityDescriptor = nullptr;
    HANDLE read_end = nullptr;
    HANDLE write_end = nullptr;
    if (!CreatePipe(&read_end, &write_end, &pipe_attrs, 0)) {
        status = std::string("Failed to start process: ") + std::to_string(GetLastError());
        return status;
    }
    unique_handle read_handle(read_end);
    unique_handle write_handle(write_end);
    if (!SetHandleInformation(read_handle.get(), HANDLE_FLAG_INHERIT, 0)) {
        status = std::string("Failed to configure process pipe: ") + std::to_string(GetLastError());
        return status;
    }

    STARTUPINFOW startup{};
    startup.cb = sizeof(startup);
    startup.dwFlags = STARTF_USESTDHANDLES;
    startup.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
    startup.hStdOutput = write_handle.get();
    startup.hStdError = write_handle.get();
    PROCESS_INFORMATION process{};
    if (!CreateProcessW(nullptr, command.data(), nullptr, nullptr, TRUE, CREATE_NO_WINDOW,
                        nullptr, nullptr, &startup, &process)) {
        const DWORD code = GetLastError();
        status = std::string("Failed to start process: ") + std::to_string(code);
        return status;
    }
    unique_handle process_handle(process.hProcess);
    unique_handle thread_handle(process.hThread);
    // EOF is observable only after every writer, including the parent's, closes.
    write_handle.reset();

    std::string output;
    bool read_error = false;
    read_pipe(read_handle.get(), output, read_error);
    read_handle.reset();
    const bool decode_error = !normalize_command_output(output);
    if (decode_error) {
        output = "Failed to decode command output as text.";
    }

    if (WaitForSingleObject(process_handle.get(), INFINITE) == WAIT_FAILED) {
        const DWORD code = GetLastError();
        status = std::string("Failed to wait for process: ") + std::to_string(code);
        if (read_error) {
            status += " (failed to read complete output)";
        }
        if (decode_error) {
            status += " (failed to decode command output)";
        }
        return output;
    }
    DWORD exit_code = 0;
    if (GetExitCodeProcess(process_handle.get(), &exit_code) == 0) {
        status = "Process ended with unknown status.";
    } else {
        status = "Exit code: " + std::to_string(static_cast<long>(exit_code));
        succeeded = exit_code == 0 && !read_error && !decode_error;
    }
    if (read_error) {
        status += " (failed to read complete output)";
    }
    if (decode_error) {
        status += " (failed to decode command output)";
    }
    return output;
}
#endif
