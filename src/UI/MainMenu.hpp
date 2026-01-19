#ifndef MAIN_MENU_HPP
#define MAIN_MENU_HPP

#include <GL/glew.h>

#include <functional>

struct GLFWwindow;

class MainMenu
{
public:
    using PlayCallback = std::function<void()>;
    using QuitCallback = std::function<void()>;

    MainMenu(GLFWwindow* window);
    ~MainMenu();

    MainMenu(const MainMenu&) = delete;
    MainMenu& operator=(const MainMenu&) = delete;

    void render(int windowWidth, int windowHeight);

    void setPlayCallback(PlayCallback callback);
    void setQuitCallback(QuitCallback callback);

private:
    void loadBackgroundTexture();
    void renderBackground(int windowWidth, int windowHeight);

    GLuint m_backgroundTexture = 0;
    int m_bgWidth = 0;
    int m_bgHeight = 0;

    PlayCallback m_onPlay;
    QuitCallback m_onQuit;
};

#endif // MAIN_MENU_HPP
