#pragma once

#include <climits>
#include <string>
#include <utility>
#include <vector>

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

namespace text_file
{

inline bool is_utf8(const std::string & text)
{
    size_t i = 0;
    while (i < text.size()) {
        const unsigned char first = static_cast<unsigned char>(text[i++]);
        if (first < 0x80) {
            continue;
        }
        size_t remaining = 0;
        unsigned char second_min = 0x80;
        unsigned char second_max = 0xBF;
        if (first >= 0xC2 && first <= 0xDF) {
            remaining = 1;
        } else if (first >= 0xE0 && first <= 0xEF) {
            remaining = 2;
            if (first == 0xE0) second_min = 0xA0;
            if (first == 0xED) second_max = 0x9F;
        } else if (first >= 0xF0 && first <= 0xF4) {
            remaining = 3;
            if (first == 0xF0) second_min = 0x90;
            if (first == 0xF4) second_max = 0x8F;
        } else {
            return false;
        }
        if (remaining > text.size() - i) {
            return false;
        }
        const unsigned char second = static_cast<unsigned char>(text[i++]);
        if (second < second_min || second > second_max) {
            return false;
        }
        for (size_t j = 1; j < remaining; ++j) {
            const unsigned char next = static_cast<unsigned char>(text[i++]);
            if (next < 0x80 || next > 0xBF) {
                return false;
            }
        }
    }
    return true;
}

// Non-UTF-8 files are interpreted using the Windows system OEM code page.
// converted is true only when the original bytes were not UTF-8.
inline bool to_utf8(const std::string & input, std::string & output, bool & converted)
{
    converted = false;
    if (is_utf8(input)) {
        output = input;
        return true;
    }
#ifdef _WIN32
    if (input.size() > static_cast<size_t>(INT_MAX)) {
        return false;
    }
    const int input_size = static_cast<int>(input.size());
    const int wide_size = MultiByteToWideChar(GetOEMCP(), MB_ERR_INVALID_CHARS, input.data(), input_size, nullptr, 0);
    if (wide_size <= 0) {
        return false;
    }
    std::vector<wchar_t> wide(static_cast<size_t>(wide_size));
    if (MultiByteToWideChar(GetOEMCP(), MB_ERR_INVALID_CHARS, input.data(), input_size, wide.data(), wide_size) != wide_size) {
        return false;
    }
    const int utf8_size = WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, wide.data(), wide_size, nullptr, 0, nullptr, nullptr);
    if (utf8_size <= 0) {
        return false;
    }
    std::string result(static_cast<size_t>(utf8_size), '\0');
    if (WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, wide.data(), wide_size, result.data(), utf8_size, nullptr, nullptr) != utf8_size) {
        return false;
    }
    output = std::move(result);
    converted = true;
    return true;
#else
    return false;
#endif
}

} // namespace text_file
