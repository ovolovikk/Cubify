#pragma once

#include "Graphics/GraphicsApi.hpp"

struct GLFWwindow;

// Window class for managing window for the application.
class Window
{
public:
    using ResizeCallbackFn = std::function<void(int, int)>;
    Window(const std::string& title, GraphicsApi api, int width_ = 1920, int height_ = 1080);
    ~Window();

    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;

    bool isOpen() const;
    void swapBuffers();
    void pollEvents();

    GLFWwindow* GetGLFWWindow() const { return window.get(); }
    void* nativeWindowHandle() const;
    int getWidth() const{ return width; }
    int getHeight() const { return height; }

    void setSize(int w, int h) { width = w; height = h; }

    void onFramebufferResize(int fbWidth, int fbHeight);
    void setResizeCallback(const ResizeCallbackFn& callback);
    static void framebuffer_size_callback(GLFWwindow* window, int width, int height);

    void toggleFullscreen();
    bool isFullscreen() const { return m_isFullscreen; }

private:
    GraphicsApi m_api = GraphicsApi::OpenGL;
    std::shared_ptr<GLFWwindow> window = nullptr;
    int width = 1920;
    int height = 1080;
    ResizeCallbackFn m_resizeCallBack;

    // Fullscreen state
    bool m_isFullscreen = false;
    int m_windowedPosX = 0;
    int m_windowedPosY = 0;
    int m_windowedWidth = 1920;
    int m_windowedHeight = 1080;
};
