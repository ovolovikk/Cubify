#ifndef GAME_HPP
#define GAME_HPP

#include <string>
#include <memory>

#include "Renderer.hpp"
#include "Camera.hpp"
#include "World.hpp"
#include "Window.hpp"

struct GLFWwindow;

class Game
{
public:
    Game(int _width, int _height, const std::string& _title);
    ~Game();

    Game(const Game&) = delete;
    Game& operator=(const Game&) = delete;

    void update(); // main loop

    void onResize(int width, int height);
    static void framebuffer_size_callback(GLFWwindow* window, int width, int height);
private:
    void init();
    void processInput(float deltaTime);

    std::unique_ptr<Window> window;

    std::unique_ptr<Renderer> renderer;
    std::unique_ptr<Camera> camera;
    std::unique_ptr<World> world;

    int width;
    int height;
};

#endif // GAME_HPP
