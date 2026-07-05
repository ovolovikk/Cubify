#pragma once

// Logging system designed to be simple yet comfortable for usage
namespace Log
{
    void init();
    void shutdown();

    void info(const char* fmt, ...);
    void warn(const char* fmt, ...);
    void error(const char* fmt, ...);
}

#define LOGI(...) Log::info(__VA_ARGS__)
#define LOGW(...) Log::warn(__VA_ARGS__)
#define LOGE(...) Log::error(__VA_ARGS__)
