#ifndef GAME_HPP
#define GAME_HPP

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
    Game(Window& window, Renderer& renderer);
    ~Game();

    Game(const Game&) = delete;
    Game& operator=(const Game&) = delete;

    // Called each frame by APP
    void onUpdate(float DeltaTime);
    void onRender();

    void onResize(int width, int height);
private:
    void init();
    
    void handleInput(float deltaTime);

    // Borrowed from APP
    Window& m_window;
    Renderer& m_renderer;

    // Owned by a Game
    std::unique_ptr<IInputController> input_controller;
    std::unique_ptr<Camera> camera;
    std::unique_ptr<World> world;
    std::unique_ptr<TerrainGenerator> terrain_generator;
    std::unique_ptr<Player> player;
    std::unique_ptr<UIController> ui_controller;
    

    // Input state
    bool free_cam_mode = false;
    bool f1_pressed = false;
    bool f3_pressed = false;
    bool cursor_visible = false;
    bool left_mouse_pressed = false;
    bool right_mouse_pressed = false;
    bool world_rendered = false;
    BlockType selectedBlock = BlockType::GRASS;

    // Camera Params
    static constexpr auto CAMERA_START_POS = glm::vec3(0.0f, 50.0f, 0.0f);
};

#endif // GAME_HPP
