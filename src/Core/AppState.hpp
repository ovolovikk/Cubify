#pragma once

//TODO MAKE USE OF PAUSED & LOADING
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
