#include "Game.hpp"
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "Core/Window.hpp"
#include "Core/Input/GLFWInputController.hpp"
#include "Math/Frustum.hpp"
#include "Graphics/Renderer.hpp"
#include "Core/Camera.hpp"
#include "World/World.hpp"
#include "Player/Player.hpp"
#include "Utils/UIController.hpp"

Game::Game(int width_, int height_, const std::string& title_)
{
    window = std::make_unique<Window>(title_, width_, height_);
    init();
}

Game::~Game() = default;

void Game::init()
{   
    GLFWwindow* nativeWindow = window->GetGLFWWindow(); 
    window->setResizeCallback([this](int w, int h){
        this->onResize(w, h);
    });

    input_controller = std::make_unique<GLFWInputController>(nativeWindow);
    renderer = std::make_unique<Renderer>(window->getWidth(), window->getHeight());
    camera = std::make_unique<Camera>(CAMERA_START_POS, CAMERA_FOV, CAMERA_ASPECT);
    world = std::make_unique<World>();
    player = std::make_unique<Player>(*camera, *input_controller, *world, world->getSpawnPoint());
    ui_controller = std::make_unique<UIController>(nativeWindow);
}

void Game::run()
{
    if (!window->isOpen()) return;

    double lastFrame = glfwGetTime();

    while(window->isOpen()) {
        double currentFrame = glfwGetTime();
        float deltaTime = float(currentFrame - lastFrame);
        lastFrame = currentFrame;

        input_controller->update();
        handleInput(deltaTime);
        update(deltaTime);
        render();

        window->swapBuffers();
        window->pollEvents();
    }
}

void Game::handleInput(float deltaTime)
{
    GLFWwindow* nativeWindow = window->GetGLFWWindow();

    if(input_controller->isKeyPressed(GLFW_KEY_ESCAPE)) {
        glfwSetWindowShouldClose(nativeWindow, true);
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

void Game::update(float deltaTime)
{
    if (!free_cam_mode && world_rendered) {
        player->update(deltaTime);
    }

    world->update(camera->GetPosition());
}

void Game::render()
{
    renderer->beginFrame();

    glm::mat4 view = camera->GetViewMatrix();
    glm::mat4 projection = camera->GetProjectionMatrix();
    glm::mat4 viewProj = projection * view;

    renderer->setViewProjection(view, projection);

    // frustum culling
    float maxDistance = (float)(world->GetRenderDistance() * CHUNK_SIZE);
    Frustum frustum(viewProj);

    world->draw(*renderer, frustum);
    world_rendered = true;

    if (ui_controller) {
        ui_controller->update(*camera, *world);
    }
}

void Game::onResize(int width_, int height_)
{
    if (window) window->setSize(width_, height_);
    if (renderer) renderer->resize(width_, height_);
    if (camera && height_ > 0) camera->SetAspect((float)width_ / (float)height_);
}
