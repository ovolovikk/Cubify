#include "Game.hpp"

#include <iostream>

int main()
{
    std::string title = "Cubify!";
    std::cout << title << std::endl;

    Game game(1920, 1020, title);
}