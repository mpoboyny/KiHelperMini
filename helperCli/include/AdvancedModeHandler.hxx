#pragma once

#include <condition_variable>
#include <cstddef>
#include <mutex>
#include <string>
#include <thread>

class AdvancedModeHandler
{
public:
    explicit AdvancedModeHandler(bool enabled);
    ~AdvancedModeHandler();

    AdvancedModeHandler(const AdvancedModeHandler &) = delete;
    AdvancedModeHandler & operator=(const AdvancedModeHandler &) = delete;

private:
    void Refresh();
    void Restore() noexcept;
    void Write(const char * text, size_t length) const noexcept;
    void Write(const std::string & text) const noexcept;

    std::mutex mutex;
    std::condition_variable wake;
    std::thread worker;
    bool stopping = false;
    bool enabled = false;
    bool region_active = false;
    unsigned int rows = 0;
    unsigned int columns = 0;
    std::string status;
#if defined(_WIN32)
    void * output_handle = nullptr;
    unsigned long original_mode = 0;
#endif
};
