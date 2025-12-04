#ifndef GAME_HPP
#define GAME_HPP

#include <string>
#include <memory>

#include "Renderer.hpp"
#include "Camera.hpp"
#include "World.hpp"

struct GLFWwindow;

class Game
{
public:
    Game(int _width, int _height, const std::string& _title);
    ~Game();

    void init(const std::string& title);
    void update(); // main loop

    void onResize(int width, int height);
    static void framebuffer_size_callback(GLFWwindow* window, int width, int height);
private:
    void processInput(float deltaTime);

    GLFWwindow* window;
    int width, height;

    std::unique_ptr<Renderer> renderer;
    std::unique_ptr<Camera> camera;
    std::unique_ptr<World> world;

};


#endif // GAME_HPP
