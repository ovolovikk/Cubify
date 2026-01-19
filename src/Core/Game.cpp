#include "Game.hpp"

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <memory>

#include "Core/Application.hpp"
#include "Core/Window.hpp"
#include "Core/Input/IInputController.hpp"
#include "Core/Logging/Log.hpp"
#include "Core/Camera.hpp"
#include "Core/Sound/AudioEngine.hpp"
#include "Math/Frustum.hpp"
#include "Graphics/Renderer.hpp"
#include "World/World.hpp"
#include "Player/Player.hpp"
#include "UI/DebugUI.hpp"
#include "Utils/Config.hpp"

Game::Game(Window& window, Renderer& renderer, IInputController& inputController)
    : m_window(window), m_renderer(renderer), m_inputController(inputController)
{  
    LOGI("[Game] Constructing...");
    init();
    LOGI("[Game] Ready");
}

Game::~Game()
{
    LOGI("[Game] Destroying...");
}

void Game::initUI()
{
    if(!debug_ui)
    {
        GLFWwindow* nativeWindow = m_window.GetGLFWWindow();
        debug_ui = std::make_unique<DebugUI>(nativeWindow);
        LOGI("[Game] DebugUI initialized");
    }
}

void Game::init()
{   
    GLFWwindow* nativeWindow = m_window.GetGLFWWindow();
    auto& cfg = Config::Get();
    auto& audio_engine = AudioEngine::Instance();
    audio_engine.PlayMusic();

    float aspect = (float)m_window.getWidth() / (float)m_window.getHeight();
    camera = std::make_unique<Camera>(CAMERA_START_POS, cfg.cConfig.fov, aspect);
    world = std::make_unique<World>();
    player = std::make_unique<Player>(*camera, m_inputController, *world, world->getSpawnPoint());
    // UIController will be initialized later when game starts playing
}

void Game::onUpdate(float deltaTime)
{
    m_inputController.update();
    handleInput(deltaTime);

    if (!free_cam_mode && world_rendered) {
        player->update(deltaTime);
    }

    world->update(camera->GetPosition());
}

void Game::onRender()
{
    m_renderer.beginFrame();

    glm::mat4 view = camera->GetViewMatrix();
    glm::mat4 projection = camera->GetProjectionMatrix();
    glm::mat4 viewProj = projection * view;

    m_renderer.setViewProjection(view, projection);

    // frustum culling
    float maxDistance = (float)(world->GetRenderDistance() * CHUNK_SIZE);
    Frustum frustum(viewProj);

    world->draw(m_renderer, frustum);
    world_rendered = true;

    if (debug_ui) {
        debug_ui->update(*camera, *world);
    }
}

void Game::handleInput(float deltaTime)
{
    GLFWwindow* nativeWindow = m_window.GetGLFWWindow();

    if(m_inputController.isKeyPressed(GLFW_KEY_ESCAPE)) {
        APP.quit();
    }
    // free cam
    if (m_inputController.isKeyPressed(GLFW_KEY_F1)) {
        if (!f1_pressed) {
            free_cam_mode = !free_cam_mode;
            f1_pressed = true;
        }
    } else {
        f1_pressed = false;
    }

    // cursor
    if (m_inputController.isKeyPressed(GLFW_KEY_F3)) {
        if (!f3_pressed) {
            cursor_visible = !cursor_visible;
            m_inputController.setCursorEnabled(cursor_visible);
            f3_pressed = true;
        }
    } else {
        f3_pressed = false;
    }

    // fullscreen toggle
    if (m_inputController.isKeyPressed(GLFW_KEY_F11)) {
        if (!f11_pressed) {
            m_window.toggleFullscreen();
            f11_pressed = true;
        }
    } else {
        f11_pressed = false;
    }

    // block choose
    if (m_inputController.isKeyPressed(GLFW_KEY_1)) selectedBlock = BlockType::GRASS;
    if (m_inputController.isKeyPressed(GLFW_KEY_2)) selectedBlock = BlockType::DIRT;
    if (m_inputController.isKeyPressed(GLFW_KEY_3)) selectedBlock = BlockType::STONE;
    if (m_inputController.isKeyPressed(GLFW_KEY_4)) selectedBlock = BlockType::SAND;
    if (m_inputController.isKeyPressed(GLFW_KEY_5)) selectedBlock = BlockType::WOODEN_PLANK;
    if (m_inputController.isKeyPressed(GLFW_KEY_6)) selectedBlock = BlockType::BEDROCK;
    
    // destroy blocks
    if (m_inputController.isMouseButtonPressed(GLFW_MOUSE_BUTTON_LEFT)) {
        if (!left_mouse_pressed) {
            world->rayCastBreakBlock(camera->GetPosition(), camera->GetFront(), 4.0f);
            left_mouse_pressed = true;
        }
    } else {
        left_mouse_pressed = false;
    }

    // place blocks
    if (m_inputController.isMouseButtonPressed(GLFW_MOUSE_BUTTON_RIGHT)) {
        if (!right_mouse_pressed) {
            world->rayCastPlaceBlock(camera->GetPosition(), camera->GetFront(), 4.0f, selectedBlock);
            right_mouse_pressed = true;
        }
    } else {
        right_mouse_pressed = false;
    }

    // fly input
    if (free_cam_mode) {
        camera->processInput(m_inputController, deltaTime);
    }
}

void Game::onResize(int width, int height)
{
    m_window.setSize(width, height);
    m_renderer.resize(width, height);
    if (camera && height > 0) camera->SetAspect((float)width / (float)height);
}
