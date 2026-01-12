#include "Helpers/Logging/LogScope.hpp"

#include "Helpers/Logging/Log.hpp"

LogScope::LogScope()
{
    Log::init();
}

LogScope::~LogScope()
{
    Log::shutdown();
}
