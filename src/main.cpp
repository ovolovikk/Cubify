#include "Core/Game.hpp"
#include "Core/Logging/LogScope.hpp"
#include "Core/Logging/Log.hpp"

int main()
{   
    LogScope log;
    LOGI("=== Cubify Start ===");

    std::string title = "Cubify!";

    Game game(1920, 1080, title);
    game.run();

    return 0;
}
