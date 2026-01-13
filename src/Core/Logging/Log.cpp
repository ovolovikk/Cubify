#include "Core/Logging/Log.hpp"
#include <fstream>
#include <cstdarg>
#include <stdio.h>
#include <ctime>
#include <sstream>
#include <iomanip>
#include <filesystem>

namespace fs = std::filesystem;

static std::ofstream logFile;

static void write(const char* level, const char* fmt, va_list args)
{
    char buffer[1024];
    vsnprintf(buffer, sizeof(buffer), fmt, args);

    std::time_t t = std::time(nullptr);
    char timeStr[32];
    std::strftime(timeStr, sizeof(timeStr), "%H:%M:%S", std::localtime(&t));

    logFile << "[" << timeStr << "] [" << level << "] " << buffer << "\n";
    logFile.flush();
}

void Log::init()
{
    if (!fs::exists("logs")) {
        fs::create_directory("logs");
    }

    std::time_t t = std::time(nullptr);
    std::tm* now = std::localtime(&t);

    std::stringstream ss;
    ss << "logs/session_" << std::put_time(now, "%Y-%m-%d_%H-%M-%S") << ".log";

    logFile.open(ss.str());
}

void Log::shutdown()
{
    logFile.close();
}

void Log::info(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    write("INFO", fmt, args);
    va_end(args);
}

void Log::warn(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    write("WARN", fmt, args);
    va_end(args);
}

void Log::error(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    write("ERROR", fmt, args);
    va_end(args);
}
