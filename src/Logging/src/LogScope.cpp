#include "LogScope.hpp"

#include "Log.hpp"

LogScope::LogScope()
{
    Log::init();
}

LogScope::~LogScope()
{
    Log::shutdown();
}
