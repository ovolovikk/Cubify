#include "Game.hpp"

#include <iostream>
#include <GLFW/glfw3.h>
#include <GL/glew.h>

Game::Game(int _width, int _height, const std::string& _title)
    : width(_width), height(_height)
{
    init(_title);
    if (window)
        update();
}

Game::~Game()
{
    glfwTerminate();
}

void Game::init(const std::string& title)
{   
    // main initialization
    glewExperimental = true;
    if(!glfwInit())
    {
        std::cout << "glfw init error.\n";
        return;
    }

    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // for MacOS
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(width, height, "Cubify", NULL, NULL);
    if(window == nullptr) {
        std::cout << "window init error.\n";
        return;
    }
    glfwMakeContextCurrent(window);

    if (glewInit() != GLEW_OK) { 
        std::cout << "glew init error\n";
        return;
    }

    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    renderer = std::make_unique<Renderer>();
    renderer->init(width, height);

    camera = std::make_unique<Camera>(glm::vec3(0.0f, 0.0f, 3.0f));
    world = std::make_unique<World>();
    for(int x = 0; x < 3;++x)
    {
        for(int y = 0; y < 3;++y)
        {
            world->addChunk(x, y);
        }
    }

    glfwGetFramebufferSize(window, &width, &height);
    renderer->init(width, height);
}

void Game::update()
{
    double lastFrame = 0.0f;
    bool first_mouse = true;
    do {
        double currentFrame = glfwGetTime();
        float deltaTime = float(currentFrame - lastFrame);
        lastFrame = currentFrame;

        glm::mat4 view = camera->GetViewMatrix();
        glm::mat4 projection = camera->GetProjectionMatrix();

        camera->ProcessWASDMovement(window);
        if(first_mouse)
        {
            int width, height;
            glfwGetWindowSize(window, &width, &height);
            glfwSetCursorPos(window, width / 2.0, height / 2.0);
            first_mouse = false;
        }
        camera->ProcessMouseMovement(window, deltaTime);

        renderer->beginFrame();
        renderer->setViewProjection(view, projection);
        renderer->drawWorld(*world);
        renderer->endFrame();

        glfwSwapBuffers(window);
        glfwPollEvents();

    } while(glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS
            && glfwWindowShouldClose(window) == 0);
}
