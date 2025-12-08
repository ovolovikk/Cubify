#include "Game.hpp"

#include <iostream>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "Core/Window.hpp"
#include "Core/Input/GLFWInputController.hpp"
#include "Helpers/Frustum.hpp"

Game::Game(int width_, int height_, const std::string& title_)
    : width(width_), height(height_)
{
    window = std::make_unique<Window>(title_, width, height);

    init();
    if (window->isOpen())
        update();
}

Game::~Game()
{
}

void Game::init()
{   
    GLFWwindow* nativeWindow = window->getGLFWWindow(); 

    glfwSetWindowUserPointer(nativeWindow, this);
    glfwSetFramebufferSizeCallback(nativeWindow, framebuffer_size_callback);

    input_controller = std::make_unique<GLFWInputController>(nativeWindow);

    renderer = std::make_unique<Renderer>();
    renderer->init(width, height);

    camera = std::make_unique<Camera>(glm::vec3(0.0f, 50.0f, 0.0f));
    world = std::make_unique<World>();
    
    renderer->init(width, height);

    terrain_generator = std::make_unique<TerrainGenerator>();
}

void Game::update()
{
    double lastFrame = 0.0f;
    double lastTime = 0.0f;
    int nbFrames = 0;

    GLFWwindow* nativeWindow = window->getGLFWWindow();

    while(window->isOpen()) {
        input_controller->update();

        if(input_controller->isKeyPressed(GLFW_KEY_ESCAPE)) {
            glfwSetWindowShouldClose(nativeWindow, true);
        }

        double currentFrame = glfwGetTime();
        float deltaTime = float(currentFrame - lastFrame);
        lastFrame = currentFrame;

        // FPS counter
        nbFrames++;
        if (currentFrame - lastTime >= 1.0) {
            std::string title = "Cubify - FPS: " + std::to_string(nbFrames) +
             " (" + std::to_string(1000.0 / double(nbFrames)) + " ms)";
            
            glfwSetWindowTitle(nativeWindow, title.c_str());
            nbFrames = 0;
            lastTime += 1.0;
        }

        glm::mat4 view = camera->GetViewMatrix();
        glm::mat4 projection = camera->GetProjectionMatrix();

        camera->processInput(*input_controller, deltaTime);

        world->update(camera->GetPosition());

        // Calculate max visible distance based on render distance
        float maxDistance = (float)(world->getRenderDistance() * CHUNK_SIZE);

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
    width = width_;
    height = height_;
    if (renderer) renderer->resize(width, height);
    if (camera && height > 0) camera->SetAspect((float)width / (float)height);
}

void Game::framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    Game* game = static_cast<Game*>(glfwGetWindowUserPointer(window));
    if (game) game->onResize(width, height);
}