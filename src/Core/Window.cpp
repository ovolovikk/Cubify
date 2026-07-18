#include "Core/Window.hpp"

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#include "Core/Logging/Log.hpp"
#include "Utils/Config.hpp"

#include <gl/gl.h>

#include "stb_image.h"

Window::Window(const std::string& title, GraphicsApi api, int width_, int height_)
    : m_api(api), window(nullptr), width(width_), height(height_)
{
    if(!glfwInit())
    {
        LOGE("[Window][GLFW] Initialization failed");
        return;
    }

    glfwWindowHint(GLFW_SAMPLES, 4);
    if (m_api == GraphicsApi::OpenGL)
    {
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    }
    else
    {
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    }

    GLFWwindow* raw_window = glfwCreateWindow(width, height, title.c_str(), NULL, NULL);
    if(raw_window == nullptr)
    {
        LOGE("[Window] Initialization failed");
        return;
    }
    window.reset(raw_window, glfwDestroyWindow);
    LOGI("[Window] Title: %s", title.c_str());
    LOGI("[Window] Size: %dx%d", width, height);

    GLFWimage icon;
    icon.pixels = stbi_load("assets/textures/icon/icon.png", &icon.width, &icon.height, nullptr, 4);
    if (icon.pixels) {
        glfwSetWindowIcon(window.get(), 1, &icon);
        stbi_image_free(icon.pixels);
        LOGI("[Window] Icon loaded");
    } else {
        LOGW("[Window] Failed to load icon.png");
    }

    if (m_api == GraphicsApi::OpenGL)
    {
        glfwMakeContextCurrent(window.get());
        glfwSwapInterval(Config::Get().wConfig.vsync ? 1 : 0);

        glewExperimental = true;
        if (glewInit() != GLEW_OK) {
            LOGE("[Window][GLEW] Initialization failed");
            return;
        }
        LOGI("[OpenGL] Vendor  : %s", glGetString(GL_VENDOR));
        LOGI("[OpenGL] Renderer: %s", glGetString(GL_RENDERER));
        LOGI("[OpenGL] Version : %s", glGetString(GL_VERSION));
    }

    glfwSetInputMode(window.get(), GLFW_STICKY_KEYS, GLFW_TRUE);
    glfwSetInputMode(window.get(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetWindowUserPointer(window.get(), this);
    glfwSetFramebufferSizeCallback(window.get(), framebuffer_size_callback);
}

Window::~Window()
{
    glfwTerminate();
}

bool Window::isOpen() const
{
    return !glfwWindowShouldClose(window.get());
}

void Window::swapBuffers()
{
    if (m_api == GraphicsApi::OpenGL)
    {
        glfwSwapBuffers(window.get());
    }
}

void* Window::nativeWindowHandle() const
{
    return static_cast<void*>(glfwGetWin32Window(window.get()));
}

void Window::pollEvents()
{
    glfwPollEvents();
}

void Window::onFramebufferResize(int fbWidth, int fbHeight)
{
    width = fbWidth;
    height = fbHeight;
    if (m_resizeCallBack) m_resizeCallBack(width, height);
}

void Window::setResizeCallback(const ResizeCallbackFn& callback)
{
    m_resizeCallBack = callback;
}

void Window::framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
    Window* win = static_cast<Window*>(glfwGetWindowUserPointer(window));
    if (win)
    {
        win->onFramebufferResize(width, height);
    }
}

void Window::toggleFullscreen()
{
    if (!window) return;

    if (m_isFullscreen)
    {
        glfwSetWindowMonitor(window.get(), nullptr, m_windowedPosX, m_windowedPosY, m_windowedWidth, m_windowedHeight, 0);
        m_isFullscreen = false;
        LOGI("[Window] Switched to windowed mode");
    }
    else
    {
        glfwGetWindowPos(window.get(), &m_windowedPosX, &m_windowedPosY);
        glfwGetWindowSize(window.get(), &m_windowedWidth, &m_windowedHeight);

        GLFWmonitor* monitor = glfwGetPrimaryMonitor();
        const GLFWvidmode* mode = glfwGetVideoMode(monitor);

        // Switch to fullscreen
        glfwSetWindowMonitor(window.get(), monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
        m_isFullscreen = true;
        LOGI("[Window] Switched to fullscreen mode");
    }
}