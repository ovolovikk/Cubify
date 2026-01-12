#include "Core/Game.hpp"

#include <iostream>

int main()
{
    std::string title = "Cubify!";
    std::cout << title << std::endl;

    Game game(1280, 960, title);
    game.run();
}
