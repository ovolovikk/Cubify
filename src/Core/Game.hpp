#ifndef GAME_HPP
#define GAME_HPP

#include <string>
#include <memory>

#include "Core/Window.hpp"
#include "Core/Input/IInputController.hpp"
#include "Graphics/Renderer.hpp"
#include "Core/Camera.hpp"
#include "World/World.hpp"
#include "World/TerrainGenerator.hpp"
#include "Player/Player.hpp"

struct GLFWwindow;
class IInputController;

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
    std::unique_ptr<IInputController> input_controller;
    std::unique_ptr<Renderer> renderer;
    std::unique_ptr<Camera> camera;
    std::unique_ptr<World> world;
    std::unique_ptr<TerrainGenerator> terrain_generator;
    std::unique_ptr<Player> player;

    int width;
    int height;
    
    bool free_cam_mode = false;
    bool f1_pressed = false;
    bool left_mouse_pressed = false;
};

#endif // GAME_HPP
