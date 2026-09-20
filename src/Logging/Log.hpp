#pragma once

#if defined(__GNUC__) || defined(__clang__)
#   define LOG_PRINTF_LIKE(fmtIndex, firstArg) __attribute__((format(printf, fmtIndex, firstArg)))
#else
#   define LOG_PRINTF_LIKE(fmtIndex, firstArg)
#endif

namespace Log
{
    void init();
    void shutdown();

    void info(const char* fmt, ...) LOG_PRINTF_LIKE(1, 2);
    void warn(const char* fmt, ...) LOG_PRINTF_LIKE(1, 2);
    void error(const char* fmt, ...) LOG_PRINTF_LIKE(1, 2);
}

#define LOGI(...) Log::info(__VA_ARGS__)
#define LOGW(...) Log::warn(__VA_ARGS__)
#define LOGE(...) Log::error(__VA_ARGS__)
