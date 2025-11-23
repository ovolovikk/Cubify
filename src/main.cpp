#include "Game.hpp"

#include <iostream>

int main()
{
    std::string title = "Cubify!";
    std::cout << title << std::endl;

    Game game(1024, 768, title);
}