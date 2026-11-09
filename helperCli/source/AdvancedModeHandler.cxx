#include "pre.hxx"

#include "AdvancedModeHandler.hxx"

#include <chrono>
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <system_error>
#include <utility>
#include <vector>

#if !defined(_WIN32)
#include <sys/ioctl.h>
#include <unistd.h>
#elif !defined(ENABLE_VIRTUAL_TERMINAL_PROCESSING)
#define ENABLE_VIRTUAL_TERMINAL_PROCESSING 0x0004
#endif

static std::string status_text(const std::string & path, unsigned int columns)
{
    std::vector<std::pair<std::string, unsigned int>> characters;
    size_t width = 0;
    const unsigned int limit = columns > 0 ? columns - 1 : 0;
    for (size_t i = 0; i < path.size();) {
        const unsigned char first = static_cast<unsigned char>(path[i]);
        if (first < 0x80) {
            characters.emplace_back(std::string(1, first < 0x20 || first == 0x7f ? '?' : static_cast<char>(first)), 1);
            ++width;
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
        bool valid = length != 0 && length <= path.size() - i;
        for (size_t j = 1; valid && j < length; ++j) {
            valid = (static_cast<unsigned char>(path[i + j]) & 0xc0) == 0x80;
        }
        if (valid) {
            const unsigned char second = static_cast<unsigned char>(path[i + 1]);
            valid = !(first == 0xe0 && second < 0xa0) && !(first == 0xed && second >= 0xa0) &&
                !(first == 0xf0 && second < 0x90) && !(first == 0xf4 && second >= 0x90) &&
                !(first == 0xc2 && second < 0xa0);
        }
        if (!valid) {
            characters.emplace_back("?", 1);
            ++width;
            ++i;
        } else {
            characters.emplace_back(path.substr(i, length), 2);
            // Reserve two columns for non-ASCII characters to avoid wrapping.
            width += 2;
            i += length;
        }
    }

    std::string result = limit > 6 ? "pwd: " : "";
    const size_t available = limit - result.size();
    size_t first = 0;
    if (width > available) {
        const size_t marker_width = available >= 4 ? 3 : (available > 0 ? 1 : 0);
        while (first < characters.size() && width > available - marker_width) {
            width -= characters[first++].second;
        }
        result += marker_width == 3 ? "..." : (marker_width == 1 ? "~" : "");
    }
    for (; first < characters.size(); ++first) {
        result += characters[first].first;
    }
    return result;
}

AdvancedModeHandler::AdvancedModeHandler(bool use_advanced_mode)
{
    if (!use_advanced_mode) {
        return;
    }
#if defined(_WIN32)
    const HANDLE input = GetStdHandle(STD_INPUT_HANDLE);
    const HANDLE output = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD input_mode = 0;
    DWORD output_mode = 0;
    if (input == nullptr || input == INVALID_HANDLE_VALUE || output == nullptr || output == INVALID_HANDLE_VALUE ||
        !GetConsoleMode(input, &input_mode) || !GetConsoleMode(output, &output_mode) ||
        !SetConsoleMode(output, output_mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING)) {
        return;
    }
    output_handle = output;
    original_mode = output_mode;
#else
    const char * term = std::getenv("TERM");
    if (!isatty(STDIN_FILENO) || !isatty(STDOUT_FILENO) || term == nullptr || std::string(term) == "dumb") {
        return;
    }
#endif
    enabled = true;
    try {
        std::fflush(stdout);
        Refresh();
        worker = std::thread([this]() {
            std::unique_lock<std::mutex> lock(mutex);
            while (!wake.wait_for(lock, std::chrono::milliseconds(250), [this]() { return stopping; })) {
                lock.unlock();
                try {
                    Refresh();
                } catch (...) {
                    // Keep the terminal usable if the directory cannot be displayed.
                }
                lock.lock();
            }
        });
    } catch (...) {
        Restore();
        enabled = false;
    }
}

AdvancedModeHandler::~AdvancedModeHandler()
{
    if (!enabled) {
        return;
    }
    {
        std::lock_guard<std::mutex> lock(mutex);
        stopping = true;
    }
    wake.notify_one();
    if (worker.joinable()) {
        worker.join();
    }
    std::fflush(stdout);
    Restore();
}

void AdvancedModeHandler::Write(const char * text, size_t length) const noexcept
{
    size_t written = 0;
    while (written < length) {
#if defined(_WIN32)
        DWORD count = 0;
        if (!WriteFile(static_cast<HANDLE>(output_handle), text + written,
                static_cast<DWORD>(length - written), &count, nullptr) || count == 0) {
            break;
        }
        written += count;
#else
        const ssize_t count = write(STDOUT_FILENO, text + written, length - written);
        if (count < 0 && errno == EINTR) {
            continue;
        }
        if (count <= 0) {
            break;
        }
        written += static_cast<size_t>(count);
#endif
    }
}

void AdvancedModeHandler::Write(const std::string & text) const noexcept
{
    Write(text.data(), text.size());
}

void AdvancedModeHandler::Refresh()
{
    unsigned int new_rows = 0;
    unsigned int new_columns = 0;
#if defined(_WIN32)
    CONSOLE_SCREEN_BUFFER_INFO info{};
    if (!GetConsoleScreenBufferInfo(static_cast<HANDLE>(output_handle), &info)) {
        return;
    }
    new_rows = static_cast<unsigned int>(info.srWindow.Bottom - info.srWindow.Top + 1);
    new_columns = static_cast<unsigned int>(info.srWindow.Right - info.srWindow.Left + 1);
#else
    struct winsize size{};
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &size) != 0) {
        return;
    }
    new_rows = size.ws_row;
    new_columns = size.ws_col;
#endif
    if (new_rows < 5 || new_columns < 2) {
        if (region_active) {
            Write("\x1b" "7\x1b[r\x1b" "8");
            region_active = false;
        }
        return;
    }
    std::error_code error;
    const std::filesystem::path path = std::filesystem::current_path(error);
    const std::string new_status = status_text(error ? "unavailable" : path.u8string(), new_columns);
    const bool resized = !region_active || rows != new_rows || columns != new_columns;
    if (!resized && status == new_status) {
        return;
    }

    std::string output;
    if (resized) {
        if (region_active) {
            output += "\x1b" "7";
        }
        output += "\x1b[1;" + std::to_string(new_rows - 3) + "r";
        if (region_active) {
            output += "\x1b" "8";
        } else {
            output += "\x1b[" + std::to_string(new_rows - 3) + ";1H";
        }
    }
    output += "\x1b" "7\x1b[0m\x1b[" + std::to_string(new_rows - 2) + ";1H\x1b[2K";
    output += "\x1b[" + std::to_string(new_rows - 1) + ";1H\x1b[2K\x1b[38;2;159;159;159m" + new_status;
    output += "\x1b[" + std::to_string(new_rows) + ";1H\x1b[0m\x1b[2K\x1b" "8";
    Write(output);
    rows = new_rows;
    columns = new_columns;
    status = new_status;
    region_active = true;
}

void AdvancedModeHandler::Restore() noexcept
{
    if (region_active) {
        char output[64];
        const int length = std::snprintf(output, sizeof(output), "\x1b" "7\x1b[%u;1H\x1b[2K\x1b[r\x1b" "8", rows - 1);
        if (length > 0 && static_cast<size_t>(length) < sizeof(output)) {
            Write(output, static_cast<size_t>(length));
        }
        region_active = false;
    }
#if defined(_WIN32)
    if (output_handle != nullptr) {
        SetConsoleMode(static_cast<HANDLE>(output_handle), original_mode);
    }
#endif
}
