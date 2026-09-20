#pragma once


struct GLFWwindow;

// Window class for managing window for the application.
class Window
{
public:
    using ResizeCallbackFn = std::function<void(int, int)>;
    using ScrollCallbackFn = std::function<void(double, double)>;

    Window(const std::string& title, int width_ = 1920, int height_ = 1080);
    ~Window();

    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;

    bool isOpen() const;
    void pollEvents();

    GLFWwindow* GetGLFWWindow() const { return window.get(); }
    void* nativeWindowHandle() const;
    int getWidth() const{ return width; }
    int getHeight() const { return height; }

    void onFramebufferResize(int fbWidth, int fbHeight);
    void setResizeCallback(const ResizeCallbackFn& callback);
    static void framebuffer_size_callback(GLFWwindow* window, int width, int height);

    void onScroll(double xOffset, double yOffset);
    void setScrollCallback(const ScrollCallbackFn& callback);
    static void scroll_callback(GLFWwindow* window, double xOffset, double yOffset);

    void toggleFullscreen();
    bool isFullscreen() const { return m_isFullscreen; }

private:
    std::shared_ptr<GLFWwindow> window = nullptr;
    int width = 1920;
    int height = 1080;
    ResizeCallbackFn m_resizeCallBack;
    ScrollCallbackFn m_scrollCallBack;

    // Fullscreen state
    bool m_isFullscreen = false;
    int m_windowedPosX = 0;
    int m_windowedPosY = 0;
    int m_windowedWidth = 1920;
    int m_windowedHeight = 1080;
};
