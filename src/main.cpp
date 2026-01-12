#include "Core/Game.hpp"
#include "Helpers/Logging/LogScope.hpp"
#include "Helpers/Logging/Log.hpp"

#include <iostream>

int main()
{   
    LogScope log;
    LOGI("=== Cubify Start ===");

    std::string title = "Cubify!";
    std::cout << title << std::endl;

    Game game(1920, 1080, title);
    game.run();

    return 0;
}
