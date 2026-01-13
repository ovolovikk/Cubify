#ifndef GAME_HPP
#define GAME_HPP

#include <string>
#include <memory>
#include <glm/vec3.hpp>

#include "Core/BlockType.hpp"

struct GLFWwindow;
class IInputController;
class Renderer;
class Camera;
class World;
class TerrainGenerator;
class Player;
class Window;
class UIController;

class Game
{
public:
    Game(int _width, int _height, const std::string& _title);
    ~Game();

    Game(const Game&) = delete;
    Game& operator=(const Game&) = delete;

    void run(); // main loop

    void onResize(int width, int height);
private:
    void init();
    
    void handleInput(float deltaTime);
    void update(float deltaTime);
    void render();

    std::unique_ptr<Window> window;
    std::unique_ptr<IInputController> input_controller;
    std::unique_ptr<Renderer> renderer;
    std::unique_ptr<Camera> camera;
    std::unique_ptr<World> world;
    std::unique_ptr<TerrainGenerator> terrain_generator;
    std::unique_ptr<Player> player;
    std::unique_ptr<UIController> ui_controller;
    
    bool free_cam_mode = false;
    bool f1_pressed = false;
    bool f3_pressed = false;
    bool cursor_visible = false;
    bool left_mouse_pressed = false;
    bool right_mouse_pressed = false;
    bool world_rendered = false;
    BlockType selectedBlock = BlockType::GRASS;

    static constexpr auto CAMERA_START_POS = glm::vec3(0.0f, 50.0f, 0.0f);
    static constexpr auto CAMERA_FOV = 60.f;
    static constexpr auto CAMERA_ASPECT = 16.f / 9.f;
};

#endif // GAME_HPP
