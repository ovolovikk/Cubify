#ifndef APP_STATE_HPP
#define APP_STATE_HPP

enum class AppState
{
    UNINITIALIZED,
    INITIALIZED,
    MENU,
    LOADING,
    PLAYING,
    PAUSED,
    SHUTTING_DOWN
};

#endif // APP_STATE_HPP