#ifndef LOG_SCOPE_HPP
#define LOG_SCOPE_HPP

// Implemented for usage Logs with RAII 
class LogScope
{
public:
    explicit LogScope();
    ~LogScope();
};

#endif // LOG_SCOPE_HPP
