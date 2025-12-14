#include "Game.hpp"

#include <iostream>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "Core/Window.hpp"
#include "Core/Input/GLFWInputController.hpp"
#include "Helpers/Frustum.hpp"

Game::Game(int width_, int height_, const std::string& title_)
{
    window = std::make_unique<Window>(title_, width_, height_);

    init();
    if (window->isOpen())
        update();
}

Game::~Game() = default;

void Game::init()
{   
    GLFWwindow* nativeWindow = window->GetGLFWWindow(); 

    glfwSetWindowUserPointer(nativeWindow, this);
    glfwSetFramebufferSizeCallback(nativeWindow, framebuffer_size_callback);

    input_controller = std::make_unique<GLFWInputController>(nativeWindow);

    renderer = std::make_unique<Renderer>();
    renderer->init(window->getWidth(), window->getHeight());

    camera = std::make_unique<Camera>(glm::vec3(0.0f, 50.0f, 0.0f));
    world = std::make_unique<World>();
    
    player = std::make_unique<Player>(*camera, *input_controller, *world, world->getSpawnPoint());

    terrain_generator = std::make_unique<TerrainGenerator>();
}

void Game::update()
{
    double lastFrame = 0.0f;
    double lastTime = 0.0f;
    int nbFrames = 0;

    GLFWwindow* nativeWindow = window->GetGLFWWindow();

    while(window->isOpen()) {
        input_controller->update();

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

        if (input_controller->isKeyPressed(GLFW_KEY_1)) {
            selectedBlock = BlockType::GRASS;
        }
        if (input_controller->isKeyPressed(GLFW_KEY_2)) {
            selectedBlock = BlockType::DIRT;
        }

        // mouse detection
        if (input_controller->isMouseButtonPressed(GLFW_MOUSE_BUTTON_LEFT))
        {
            if (!left_mouse_pressed)
            {
                world->rayCastBreakBlock(camera->GetPosition(), camera->GetFront(), 4.0f);
                left_mouse_pressed = true;
            }
        } else left_mouse_pressed = false;

        if (input_controller->isMouseButtonPressed(GLFW_MOUSE_BUTTON_RIGHT))
        {
            if (!right_mouse_pressed)
            {
                world->rayCastPlaceBlock(camera->GetPosition(), camera->GetFront(), 4.0f, selectedBlock);
                right_mouse_pressed = true;
            }
        } else right_mouse_pressed = false;

        double currentFrame = glfwGetTime();
        float deltaTime = float(currentFrame - lastFrame);
        lastFrame = currentFrame;

        glm::mat4 view = camera->GetViewMatrix();
        glm::mat4 projection = camera->GetProjectionMatrix();

        if (free_cam_mode) {
            camera->processInput(*input_controller, deltaTime);
        } else {
            player->update(deltaTime);
        }

        world->update(camera->GetPosition());

        //max visible distance based on render distance
        float maxDistance = (float)(world->GetRenderDistance() * CHUNK_SIZE);

        Frustum frustum;
        frustum.update(camera->GetPosition(), camera->GetFront(), camera->GetFOV(), maxDistance);
        
        renderer->beginFrame();
        renderer->setViewProjection(view, projection);
        world->draw(*renderer, frustum);
        renderer->endFrame();

        window->swapBuffers();
        window->pollEvents();
    }
}

void Game::onResize(int width_, int height_)
{
    if (window) window->setSize(width_, height_);
    if (renderer) renderer->resize(width_, height_);
    if (camera && height_ > 0) camera->SetAspect((float)width_ / (float)height_);
}

void Game::framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    Game* game = static_cast<Game*>(glfwGetWindowUserPointer(window));
    if (game) game->onResize(width, height);
}
