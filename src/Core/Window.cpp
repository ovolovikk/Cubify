#include "Core/Window.hpp"

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <iostream>

Window::Window(const std::string& title, int width_, int height_)
    : width(width_), height(height_), window(nullptr)
{
    glewExperimental = true;
    if(!glfwInit())
    {
        std::cerr <<"glfw init error.\n";
        return;
    }

    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // for MacOS
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* raw_window = glfwCreateWindow(width, height, title.c_str(), NULL, NULL);
    if(raw_window == nullptr)
    {
        std::cerr << "window init error.\n";
        return;
    }
    window.reset(raw_window, glfwDestroyWindow);

    glfwMakeContextCurrent(window.get());
    glfwSwapInterval(0);

    if (glewInit() != GLEW_OK) { 
        std::cerr << "glew init error\n";
        return;
    }

    glfwSetInputMode(window.get(), GLFW_STICKY_KEYS, GL_TRUE);
    glfwSetInputMode(window.get(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetWindowUserPointer(window.get(), this);
}

Window::~Window()
{
    if (true)   glfwTerminate();
}

bool Window::isOpen() const
{
    return !glfwWindowShouldClose(window.get());
}

void Window::swapBuffers()
{
    glfwSwapBuffers(window.get());
}

void Window::pollEvents()
{
    glfwPollEvents();
}
