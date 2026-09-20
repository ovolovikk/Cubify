#include "Logging/LogScope.hpp"

#include "Logging/Log.hpp"

LogScope::LogScope()
{
    Log::init();
}

LogScope::~LogScope()
{
    Log::shutdown();
}
