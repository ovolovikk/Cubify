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
class DebugUI;

class Game
{
public:
    Game(Window& window, Renderer& renderer, IInputController& inputController);
    ~Game();

    Game(const Game&) = delete;
    Game& operator=(const Game&) = delete;

    // Called each frame by APP
    void onUpdate(float DeltaTime);
    void onRender();
    void onRenderDebug(DebugUI* debugUI);

    void onResize(int width, int height);

private:
    void init();
    
    void handleInput(float deltaTime);

    // Borrowed from APP
    Window& m_window;
    Renderer& m_renderer;
    IInputController& m_inputController;

    // Owned by a Game
    std::unique_ptr<Camera> camera;
    std::unique_ptr<World> world;
    std::unique_ptr<TerrainGenerator> terrain_generator;
    std::unique_ptr<Player> player;

private:
    // Input state
    bool free_cam_mode = false;
    bool cursor_visible = false;
    bool world_rendered = false;
    BlockType selectedBlock = BlockType::GRASS;

    // Camera Params
    static constexpr auto CAMERA_START_POS = glm::vec3(0.0f, 50.0f, 0.0f);
};

#endif // GAME_HPP
