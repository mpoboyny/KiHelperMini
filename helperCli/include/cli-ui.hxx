#pragma once

#include "common.h"
#include "console.h"

#include <array>
#include <algorithm>
#include <cctype>
#include <cstddef>
#include <cstdio>
#include <cwchar>
#include <cwctype>
#include <filesystem>
#include <string_view>

#if defined(_WIN32)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

// TODO?: Make this reusable, enums, docs
static const std::array<std::string_view, 10> cmds = {
    "/# ",
    "/audio ",
    "/clear",
    "/exit",
    "/glob ",
    "/image ",
    "/read ",
    "/regen",
    "/run ",
    "/video ",
};

static std::vector<std::pair<std::string, size_t>> auto_completion_callback(std::string_view line, size_t cursor_byte_pos) {
    std::vector<std::pair<std::string, size_t>> matches;
    std::string cmd;

    if (line.length() > 1 && line.front() == '/' && !std::any_of(cmds.begin(), cmds.end(), [line](std::string_view prefix) {
        return string_starts_with(line, prefix);
    })) {
        auto it = cmds.begin();

        while ((it = std::find_if(it, cmds.end(), [line](std::string_view cmd_line) {
            return string_starts_with(cmd_line, line);
        })) != cmds.end()) {
            matches.emplace_back(*it, it->length());
            ++it;
        }
    } else {
        auto it = std::find_if(cmds.begin(), cmds.end(), [line](std::string_view prefix) {
            return prefix.back() == ' ' && string_starts_with(line, prefix);
        });

        if (it != cmds.end()) {
            cmd = *it;
        }
    }

    if (!cmd.empty() && cmd != "/glob " && line.length() >= cmd.length() && cursor_byte_pos >= cmd.length()) {
        const std::string path_prefix  = std::string(line.substr(cmd.length(), cursor_byte_pos - cmd.length()));
        const std::string path_postfix = std::string(line.substr(cursor_byte_pos));
        auto cur_dir = std::filesystem::current_path();
        std::string cur_dir_str = cur_dir.string();
        std::string expanded_prefix = path_prefix;

#if !defined(_WIN32)
        if (string_starts_with(path_prefix, '~')) {
            const char * home = std::getenv("HOME");
            if (home && home[0]) {
                expanded_prefix = home + path_prefix.substr(1);
            }
        }
        if (string_starts_with(expanded_prefix, '/')) {
#else
        if (std::isalpha(static_cast<unsigned char>(expanded_prefix[0])) && expanded_prefix.find(':') == 1) {
#endif
            cur_dir = std::filesystem::path(expanded_prefix).parent_path();
            cur_dir_str.clear();
        } else if (!path_prefix.empty()) {
            cur_dir /= std::filesystem::path(path_prefix).parent_path();
        }

        std::error_code ec;
        for (const auto & entry : std::filesystem::directory_iterator(cur_dir, ec)) {
            if (ec) {
                break;
            }
            if (!entry.exists(ec)) {
                ec.clear();
                continue;
            }

            const std::string path_full = entry.path().string();
            std::string path_entry = !cur_dir_str.empty() && string_starts_with(path_full, cur_dir_str) ? path_full.substr(cur_dir_str.length() + 1) : path_full;

            if (entry.is_directory(ec)) {
                path_entry.push_back(std::filesystem::path::preferred_separator);
            }

            if (expanded_prefix.empty() || string_starts_with(path_entry, expanded_prefix)) {
                const std::string updated_line = cmd + path_entry;
                matches.emplace_back(updated_line + path_postfix, updated_line.length());
            }

            if (ec) {
                ec.clear();
            }
        }

        if (matches.empty()) {
            const std::string updated_line = cmd + path_prefix;
            matches.emplace_back(updated_line + path_postfix, updated_line.length());
        }

        // Add the longest common prefix
        if (!expanded_prefix.empty() && matches.size() > 1) {
            const std::string_view match0(matches[0].first);
            const std::string_view match1(matches[1].first);
            auto it = std::mismatch(match0.begin(), match0.end(), match1.begin(), match1.end());
            size_t len = it.first - match0.begin();

            for (size_t i = 2; i < matches.size(); ++i) {
                const std::string_view matchi(matches[i].first);
                auto cmp = std::mismatch(match0.begin(), match0.end(), matchi.begin(), matchi.end());
                len = std::min(len, static_cast<size_t>(cmp.first - match0.begin()));
            }

            const std::string updated_line = std::string(match0.substr(0, len));
            matches.emplace_back(updated_line + path_postfix, updated_line.length());
        }

        std::sort(matches.begin(), matches.end(), [](const auto & a, const auto & b) {
            return a.first.compare(0, a.second, b.first, 0, b.second) < 0;
        });
    }

    return matches;
}

//
// Dedicated input path for the /run and /# command.
//
// Unlike the generic console reader (Enter submits, Tab completes), this one:
//   - shows a #C3C3C3 background on the /run or /# command part of the line
//   - treats Tab as end of the command (same line, no submit); text typed
//     after the Tab is the question
//   - keeps Enter as the submit key
// The reader only works in the console's advanced (character) input mode; in
// simple_io mode it is not used and the normal path applies.
//
// When non-null, keys are taken from run_input_source (unit-test injection)
// instead of the real console.
static std::function<char32_t()> run_input_source;

struct run_input_command {
    size_t start;
    size_t end;
    size_t prefix_length;
};

static std::vector<run_input_command> run_input_commands;

#if defined(_WIN32)
static char32_t run_input_getchar32() {
    HANDLE hConsole = GetStdHandle(STD_INPUT_HANDLE);
    wchar_t high_surrogate = 0;
    for (;;) {
        INPUT_RECORD record;
        DWORD count = 0;
        if (!ReadConsoleInputW(hConsole, &record, 1, &count) || count == 0) {
            return WEOF;
        }
        if (record.EventType == KEY_EVENT && record.Event.KeyEvent.bKeyDown) {
            const wchar_t wc = record.Event.KeyEvent.uChar.UnicodeChar;
            if (wc == 0x1B) {
                return 0xE009;
            }
            if (wc == 0) {
                switch (record.Event.KeyEvent.wVirtualKeyCode) {
                    case VK_TAB:    return '\t';
                    case VK_RETURN: return '\r';
                    case VK_BACK:   return 0x08;
                    case VK_LEFT:   return 0xE000;
                    case VK_RIGHT:  return 0xE001;
                    case VK_UP:     return 0xE002;
                    case VK_DOWN:   return 0xE003;
                    case VK_HOME:   return 0xE004;
                    case VK_END:    return 0xE005;
                    case VK_DELETE: return 0xE008;
                    case VK_ESCAPE: return 0xE009;
                    default:        continue;
                }
            }
            if (wc >= 0xD800 && wc <= 0xDBFF) {
                high_surrogate = wc;
                continue;
            }
            if (wc >= 0xDC00 && wc <= 0xDFFF) {
                if (high_surrogate != 0) {
                    return ((high_surrogate - 0xD800) << 10) + (wc - 0xDC00) + 0x10000;
                }
            }
            high_surrogate = 0;
            return static_cast<char32_t>(wc);
        }
    }
}
#else
static char32_t run_input_getchar32() {
    wint_t wc = getwchar();
    if (wc == WEOF) {
        return WEOF;
    }
    return static_cast<char32_t>(wc);
}
#endif

static char32_t run_input_fetch_char32() {
    return run_input_source ? run_input_source() : run_input_getchar32();
}

static char32_t decode_run_utf8(const std::string & input, size_t pos, size_t & advance) {
    unsigned char c = static_cast<unsigned char>(input[pos]);
    if ((c & 0x80u) == 0u) {
        advance = 1;
        return c;
    }
    if ((c & 0xE0u) == 0xC0u && pos + 1 < input.size()) {
        unsigned char c1 = static_cast<unsigned char>(input[pos + 1]);
        if ((c1 & 0xC0u) != 0x80u) {
            advance = 1;
            return 0xFFFD;
        }
        advance = 2;
        return ((c & 0x1Fu) << 6) | (c1 & 0x3Fu);
    }
    if ((c & 0xF0u) == 0xE0u && pos + 2 < input.size()) {
        unsigned char c1 = static_cast<unsigned char>(input[pos + 1]);
        unsigned char c2 = static_cast<unsigned char>(input[pos + 2]);
        if ((c1 & 0xC0u) != 0x80u || (c2 & 0xC0u) != 0x80u) {
            advance = 1;
            return 0xFFFD;
        }
        advance = 3;
        return ((c & 0x0Fu) << 12) | ((c1 & 0x3Fu) << 6) | (c2 & 0x3Fu);
    }
    if ((c & 0xF8u) == 0xF0u && pos + 3 < input.size()) {
        unsigned char c1 = static_cast<unsigned char>(input[pos + 1]);
        unsigned char c2 = static_cast<unsigned char>(input[pos + 2]);
        unsigned char c3 = static_cast<unsigned char>(input[pos + 3]);
        if ((c1 & 0xC0u) != 0x80u || (c2 & 0xC0u) != 0x80u || (c3 & 0xC0u) != 0x80u) {
            advance = 1;
            return 0xFFFD;
        }
        advance = 4;
        return ((c & 0x07u) << 18) | ((c1 & 0x3Fu) << 12) | ((c2 & 0x3Fu) << 6) | (c3 & 0x3Fu);
    }
    advance = 1;
    return 0xFFFD;
}

static size_t run_prev_utf8(const std::string & line, size_t pos) {
    if (pos == 0) return 0;
    pos--;
    while (pos > 0 && (static_cast<unsigned char>(line[pos]) & 0xC0) == 0x80) {
        pos--;
    }
    return pos;
}

static size_t run_next_utf8(const std::string & line, size_t pos) {
    if (pos >= line.size()) return line.size();
    pos++;
    while (pos < line.size() && (static_cast<unsigned char>(line[pos]) & 0xC0) == 0x80) {
        pos++;
    }
    return pos;
}

static int run_char_width(char32_t cp) {
#if defined(_WIN32)
    (void)cp;
    return 1;
#else
    int w = wcwidth(static_cast<wint_t>(cp));
    return w > 0 ? w : 1;
#endif
}

static int run_line_width(const std::string & line) {
    int width = 0;
    size_t p = 0;
    while (p < line.size()) {
        size_t adv = 0;
        char32_t cp = decode_run_utf8(line, p, adv);
        width += run_char_width(cp);
        p += adv;
    }
    return width;
}

static int run_cursor_col(const std::string & line, size_t cursor) {
    int width = 0;
    size_t p = 0;
    while (p < cursor) {
        size_t adv = 0;
        char32_t cp = decode_run_utf8(line, p, adv);
        width += run_char_width(cp);
        p += adv;
    }
    return width;
}

static void run_move_cursor(int delta) {
    if (delta == 0) {
        return;
    }
#if defined(_WIN32)
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hConsole == NULL) {
        return;
    }
    CONSOLE_SCREEN_BUFFER_INFO bufferInfo;
    if (!GetConsoleScreenBufferInfo(hConsole, &bufferInfo)) {
        return;
    }
    COORD pos = bufferInfo.dwCursorPosition;
    int width = bufferInfo.dwSize.X;
    int newX = pos.X + delta;
    int newY = pos.Y;
    while (newX >= width) {
        newX -= width;
        newY++;
    }
    while (newX < 0) {
        newX += width;
        newY--;
    }
    pos.X = newX;
    pos.Y = newY;
    SetConsoleCursorPosition(hConsole, pos);
#else
    if (delta < 0) {
        for (int i = 0; i < -delta; i++) fputc('\b', stdout);
    } else {
        for (int i = 0; i < delta; i++) fputc('\033[C', stdout);
    }
#endif
}

static void run_append_utf8(char32_t ch, std::string & out) {
    if (ch <= 0x7F) {
        out.push_back(static_cast<char>(ch));
    } else if (ch <= 0x7FF) {
        out.push_back(static_cast<char>(0xC0 | ((ch >> 6) & 0x1F)));
        out.push_back(static_cast<char>(0x80 | (ch & 0x3F)));
    } else if (ch <= 0xFFFF) {
        out.push_back(static_cast<char>(0xE0 | ((ch >> 12) & 0x0F)));
        out.push_back(static_cast<char>(0x80 | ((ch >> 6) & 0x3F)));
        out.push_back(static_cast<char>(0x80 | (ch & 0x3F)));
    } else {
        out.push_back(static_cast<char>(0xF0 | ((ch >> 18) & 0x07)));
        out.push_back(static_cast<char>(0x80 | ((ch >> 12) & 0x1F)));
        out.push_back(static_cast<char>(0x80 | ((ch >> 6) & 0x3F)));
        out.push_back(static_cast<char>(0x80 | (ch & 0x3F)));
    }
}

// True while the line is a /run or /# command (the background + Tab
// end-of-command behavior only applies to these two commands).
static bool run_is_run_mode(const std::string & line) {
    if (line.length() >= 2 && string_starts_with(line, "/#")) {
        return true;
    }
    if (line.length() >= 4 && string_starts_with(line, "/run")) {
        return true;
    }
    return false;
}

static size_t run_command_prefix_length(const std::string & line, size_t end) {
    if (end >= 2 && line.compare(end - 2, 2, "/#") == 0 &&
        (end == 2 || std::isspace(static_cast<unsigned char>(line[end - 3])))) {
        return 2;
    }
    if (end >= 4 && line.compare(end - 4, 4, "/run") == 0 &&
        (end == 4 || std::isspace(static_cast<unsigned char>(line[end - 5])))) {
        return 4;
    }
    return 0;
}

static void run_put(const char * text) {
    fputs(text, stdout);
}

// Reads one line with the dedicated /run behavior. Returns a string that ends
// with '\n' (matching console::readline), or an empty string on EOF. The
// #C3C3C3 background covers the /run or /# command part while it is drawn.
static std::string read_run_input_raw(bool color_enabled) {
    const char * bg_on  = "\x1b[30;48;2;195;195;195m";
    const char * bg_off = "\x1b[0m";

    std::string line;
    size_t cursor = 0; // byte position of the cursor
    bool eof       = false;
    size_t command_start = std::string::npos;
    size_t command_prefix_length = 0;
    std::vector<run_input_command> commands;

    int prev_cursor_col = 0; // cursor offset (cols) from the line start at the last draw
    int prev_line_width = 0; // width of the line at the last draw

    auto redraw = [&]() {
        // Return the cursor to the start of the line content (just after the
        // "> " prompt) without disturbing the prompt itself.
        if (prev_cursor_col > 0) {
            run_move_cursor(-prev_cursor_col);
        }
        if (color_enabled) {
            run_put(bg_off);
        }
        const int new_width = run_line_width(line);
        if (command_start == std::string::npos) {
            fputs(line.c_str(), stdout);
        } else {
            fwrite(line.data(), 1, command_start, stdout);
            if (color_enabled) {
                run_put(bg_on);
            }
            fputs(line.c_str() + command_start, stdout);
        }
        // Clear stale characters left over from a longer previous line, keeping
        // them on the same background so the block stays uniform.
        if (prev_line_width > new_width) {
            for (int i = 0; i < prev_line_width - new_width; i++) {
                fputc(' ', stdout);
            }
        }
        if (color_enabled) {
            run_put(bg_off); // restore the base color
        }
        // Position the cursor at its column (never to the right of the line).
        const int target = run_cursor_col(line, cursor);
        const int drawn_width = std::max(new_width, prev_line_width);
        run_move_cursor(target - drawn_width);
        prev_cursor_col = target;
        prev_line_width = new_width;
        fflush(stdout);
    };

    if (color_enabled) {
        run_put(bg_off); // clear any color inherited from the console
    }
    run_put("\n> ");
    redraw();

    for (;;) {
        char32_t cp = run_input_fetch_char32();

        if (cp == WEOF) {
            eof = true;
            break;
        }

        // Enter submits the line; Ctrl+D submits it as EOF.
        if (cp == '\r' || cp == '\n' || cp == 0x04) {
            if (cp == 0x04) {
                eof = true;
            }
            // Note: the newline is printed by the caller so that the console
            // cursor is not advanced before the reader returns.
            break;
        }

        // Tab keeps the active command open; outside a command it auto-completes.
        if (cp == '\t') {
            if (command_start != std::string::npos) {
                if (line.back() != ' ') {
                    line.push_back(' ');
                }
                cursor = line.size();
                redraw();
                continue;
            }
            auto candidates = auto_completion_callback(line, cursor);
            if (!candidates.empty()) {
                if (candidates.size() > 1 || candidates[0].first != line) {
                    line = candidates[0].first;
                    cursor = std::min(candidates[0].second, line.size());
                }
                redraw();
                continue;
            }
            continue;
        }

        // Backspace
        if (cp == 0x08 || cp == 0x7F) {
            if (cursor > 0) {
                size_t prev = run_prev_utf8(line, cursor);
                line.erase(prev, cursor - prev);
                cursor = prev;
                if (command_start != std::string::npos && prev < command_start) {
                    command_start = std::string::npos;
                    command_prefix_length = 0;
                }
                redraw();
            }
            continue;
        }

        // Escape sequences (arrows, Home, End, Delete)
        if (cp == 0x1B) {
            char32_t code = run_input_fetch_char32();
            if (code == '[') {
                std::string params;
                for (;;) {
                    code = run_input_fetch_char32();
                    if ((code >= 'A' && code <= 'Z') || (code >= 'a' && code <= 'z') ||
                        code == '~' || code == WEOF) {
                        break;
                    }
                    params.push_back(static_cast<char>(code));
                }
                if (code == 'D') {
                    if (cursor > 0) {
                        cursor = run_prev_utf8(line, cursor);
                    }
                } else if (code == 'C') {
                    cursor = run_next_utf8(line, cursor);
                } else if (code == 'H') {
                    cursor = 0;
                } else if (code == 'F') {
                    cursor = line.size();
                } else if (code == '~') {
                    cursor = line.size();
                }
                redraw();
            } else if (code == 0x1B) {
                while ((code = run_input_fetch_char32()) != WEOF) {
                    if ((code >= 'A' && code <= 'Z') || (code >= 'a' && code <= 'z') || code == '~') {
                        break;
                    }
                }
            }
            continue;
        }

        // Windows special keys arrive as private-use code points
#if defined(_WIN32)
        switch (cp) {
            case 0xE000:
                if (cursor > 0) {
                    cursor = run_prev_utf8(line, cursor);
                    redraw();
                }
                continue;
            case 0xE001:
                cursor = run_next_utf8(line, cursor);
                redraw();
                continue;
            case 0xE004:
                cursor = 0;
                redraw();
                continue;
            case 0xE005:
                cursor = line.size();
                redraw();
                continue;
            case 0xE008:
                if (cursor < line.size()) {
                    size_t next = run_next_utf8(line, cursor);
                    line.erase(cursor, next - cursor);
                    redraw();
                }
                continue;
            case 0xE009:
                if (command_start != std::string::npos) {
                    commands.push_back({command_start, cursor, command_prefix_length});
                    line.insert(cursor, 1, ' ');
                    cursor++;
                    command_start = std::string::npos;
                    command_prefix_length = 0;
                    redraw();
                }
                continue;
            default:
                break;
        }
#endif

        // Insert a printable character at the cursor
        if (cp >= 0x20 && cp <= 0x10FFFF) {
            std::string ch;
            run_append_utf8(cp, ch);
            line.insert(cursor, ch);
            cursor += ch.size();
            if (command_start == std::string::npos) {
                command_prefix_length = run_command_prefix_length(line, cursor);
                if (command_prefix_length != 0) {
                    command_start = cursor - command_prefix_length;
                }
            }
            redraw();
        }
    }

    if (eof && line.empty()) {
        return std::string();
    }
    run_input_commands = std::move(commands);
    line += '\n';
    return line;
}

// note: make this view implementation generic, so that we can move to TUI in the future if we want to
namespace ui {
    static void init(const common_params & params) {
        // TODO: avoid using atexit() here by making `console` a singleton
        console::init(params.simple_io, params.use_color);
        atexit([]() { console::cleanup(); });

        console::set_completion_callback(auto_completion_callback);
    }

    struct spinner {
        spinner(const std::string & message) {
            if (!message.empty()) {
                console::log("%s ", message.c_str());
            }
            console::spinner::start();
        }
        ~spinner() {
            console::spinner::stop();
        }
    };

    struct user_turn {
        user_turn() {
            console::set_display(DISPLAY_TYPE_USER_INPUT);
        }
        ~user_turn() {
            console::set_display(DISPLAY_TYPE_RESET);
        }
        void echo(const std::string & buffer) {
            if (buffer.size() > 500) {
                console::log("\n> %s ... (truncated)\n", buffer.substr(0, 500).c_str());
            } else {
                console::log("\n> %s\n", buffer.c_str());
            }
        }
        std::string read_input(bool multiline_input, const char * prompt = nullptr) {
            if (prompt) {
                console::log("%s", prompt);
            } else {
                console::log("\n> ");
            }
            std::string buffer;
            std::string line;
            bool another_line = true;
            do {
                another_line = console::readline(line, multiline_input);
                buffer += line;
            } while (another_line);
            return buffer;
        }
    };

    // Dedicated reader for the /run and /# command: Enter submits the whole
    // line, Tab marks the end of the command (same line), and the command part
    // gets a #C3C3C3 background. Only used in advanced input mode.
    // The result keeps the trailing '\n' of the reader (an empty line yields
    // "\n", a real EOF yields ""); cli_context::run() strips the newline, so
    // an empty line is skipped instead of being treated as EOF.
    static std::string read_run_input(bool color_enabled) {
        run_input_commands.clear();
        std::string line = read_run_input_raw(color_enabled);
        // The reader leaves the cursor at the end of the line; commit it.
        if (!line.empty()) {
            fputc('\n', stdout);
        }
        fflush(stdout);
        return line;
    }

    static std::vector<run_input_command> take_run_input_commands() {
        return std::move(run_input_commands);
    }

    enum assistant_display_mode {
        ASSISTANT_DISPLAY_MODE_REASONING,
        ASSISTANT_DISPLAY_MODE_CONTENT,
    };
    struct assistant_turn {
        assistant_display_mode mode = ASSISTANT_DISPLAY_MODE_CONTENT;
        bool trailing_newline = true;
        bool is_inside_reasoning = false;
        assistant_turn() {
            console::set_display(DISPLAY_TYPE_RESET);
        }
        ~assistant_turn() {
            console::set_display(DISPLAY_TYPE_RESET);
            add_newline_if_needed();
            if (is_inside_reasoning) {
                console::log("[End thinking]\n\n");
            }
        }
        void push(assistant_display_mode m, const std::string & buffer) {
            if (m != mode) {
                add_newline_if_needed();
                switch (m) {
                    case ASSISTANT_DISPLAY_MODE_CONTENT:
                        {
                            if (is_inside_reasoning) {
                                console::log("[End thinking]\n\n");
                                is_inside_reasoning = false;
                            }
                            console::set_display(DISPLAY_TYPE_RESET);
                        } break;
                    case ASSISTANT_DISPLAY_MODE_REASONING:
                        {
                            console::set_display(DISPLAY_TYPE_REASONING);
                            is_inside_reasoning = true;
                            console::log("\n[Start thinking]\n\n");
                        } break;
                }
            }
            mode = m;
            if (buffer.empty()) {
                return;
            }
            trailing_newline = buffer.back() == '\n';
            console::log("%s", buffer.c_str());
            console::flush();
        }
        void add_newline_if_needed() {
            if (!trailing_newline) {
                console::log("\n");
                console::flush();
            }
        }
    };

    static void show_error(const std::string & title, const std::string & message = "") {
        console::spinner::stop();
        console::error("Error: %s\n", title.c_str());
        if (!message.empty()) {
            console::log("%s\n", message.c_str());
        }
    }

    static void show_message(const std::string & message) {
        console::log("%s\n", message.c_str());
    }

    static void show_info(const std::string & message) {
        console::set_display(DISPLAY_TYPE_INFO);
        console::log("%s\n", message.c_str());
        console::set_display(DISPLAY_TYPE_RESET);
    }
}
