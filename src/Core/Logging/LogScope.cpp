#include "Core/Logging/LogScope.hpp"

#include "Core/Logging/Log.hpp"

LogScope::LogScope()
{
    Log::init();
}

LogScope::~LogScope()
{
    Log::shutdown();
}
