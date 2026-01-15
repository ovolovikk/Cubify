#include "Game.hpp"

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "Core/Application.hpp"
#include "Core/Window.hpp"
#include "Core/Input/GLFWInputController.hpp"
#include "Core/Logging/Log.hpp"
#include "Core/Camera.hpp"
#include "Math/Frustum.hpp"
#include "Graphics/Renderer.hpp"
#include "World/World.hpp"
#include "Player/Player.hpp"
#include "Utils/UIController.hpp"
#include "Utils/Config.hpp"

Game::Game(Window& window, Renderer& renderer)
    : m_window(window), m_renderer(renderer)
{  
    LOGI("[Game] Constructing...");
    init();
    LOGI("[Game] Ready");
}

Game::~Game()
{
    LOGI("[Game] Destroying...");
}

void Game::init()
{   
    GLFWwindow* nativeWindow = m_window.GetGLFWWindow();
    auto& cfg = Config::Get();

    input_controller = std::make_unique<GLFWInputController>(nativeWindow);
    float aspect = (float)m_window.getWidth() / (float)m_window.getHeight();
    camera = std::make_unique<Camera>(CAMERA_START_POS, cfg.cConfig.fov, aspect);
    world = std::make_unique<World>();
    player = std::make_unique<Player>(*camera, *input_controller, *world, world->getSpawnPoint());
    ui_controller = std::make_unique<UIController>(nativeWindow);
}

void Game::onUpdate(float deltaTime)
{
    input_controller->update();
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

    if (ui_controller) {
        ui_controller->update(*camera, *world);
    }
}

void Game::handleInput(float deltaTime)
{
    GLFWwindow* nativeWindow = m_window.GetGLFWWindow();

    if(input_controller->isKeyPressed(GLFW_KEY_ESCAPE)) {
        APP.quit();
    }
    // free cam
    if (input_controller->isKeyPressed(GLFW_KEY_F1)) {
        if (!f1_pressed) {
            free_cam_mode = !free_cam_mode;
            f1_pressed = true;
        }
    } else {
        f1_pressed = false;
    }

    // cursor
    if (input_controller->isKeyPressed(GLFW_KEY_F3)) {
        if (!f3_pressed) {
            cursor_visible = !cursor_visible;
            input_controller->setCursorEnabled(cursor_visible);
            f3_pressed = true;
        }
    } else {
        f3_pressed = false;
    }

    // block choose
    if (input_controller->isKeyPressed(GLFW_KEY_1)) selectedBlock = BlockType::GRASS;
    if (input_controller->isKeyPressed(GLFW_KEY_2)) selectedBlock = BlockType::DIRT;
    if (input_controller->isKeyPressed(GLFW_KEY_3)) selectedBlock = BlockType::STONE;
    if (input_controller->isKeyPressed(GLFW_KEY_4)) selectedBlock = BlockType::SAND;
    if (input_controller->isKeyPressed(GLFW_KEY_5)) selectedBlock = BlockType::WOODEN_PLANK;
    
    // destroy blocks
    if (input_controller->isMouseButtonPressed(GLFW_MOUSE_BUTTON_LEFT)) {
        if (!left_mouse_pressed) {
            world->rayCastBreakBlock(camera->GetPosition(), camera->GetFront(), 4.0f);
            left_mouse_pressed = true;
        }
    } else {
        left_mouse_pressed = false;
    }

    // place blocks
    if (input_controller->isMouseButtonPressed(GLFW_MOUSE_BUTTON_RIGHT)) {
        if (!right_mouse_pressed) {
            world->rayCastPlaceBlock(camera->GetPosition(), camera->GetFront(), 4.0f, selectedBlock);
            right_mouse_pressed = true;
        }
    } else {
        right_mouse_pressed = false;
    }

    // fly input
    if (free_cam_mode) {
        camera->processInput(*input_controller, deltaTime);
    }
}

void Game::onResize(int width, int height)
{
    m_window.setSize(width, height);
    m_renderer.resize(width, height);
    if (camera && height > 0) camera->SetAspect((float)width / (float)height);
}
