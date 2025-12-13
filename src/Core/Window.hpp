#ifndef WINDOW_HPP
#define WINDOW_HPP

#include <string>

struct GLFWwindow;

class Window
{
public:
    Window(const std::string& title, int width_ = 1920, int height_ = 1080);
    ~Window();

    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;

    bool isOpen() const;
    void swapBuffers();
    void pollEvents();

    GLFWwindow* GetGLFWWindow() const { return window; }
    int getWidth() const{ return width; }
    int getHeight() const { return height; } 
    void setSize(int w, int h) { width = w; height = h; }

private:
    GLFWwindow* window;
    int width;
    int height;
};

#endif // WINDOW_HPP
