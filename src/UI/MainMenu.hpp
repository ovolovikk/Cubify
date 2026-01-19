#ifndef MAIN_MENU_HPP
#define MAIN_MENU_HPP

#include <GL/glew.h>

#include <functional>

struct GLFWwindow;
class IInputController;
class Window;

class MainMenu
{
public:
    using PlayCallback = std::function<void()>;
    using QuitCallback = std::function<void()>;

    MainMenu(GLFWwindow* window, Window& appWindow, IInputController& inputController);
    ~MainMenu();

    MainMenu(const MainMenu&) = delete;
    MainMenu& operator=(const MainMenu&) = delete;

    void onUpdate(float deltaTime);
    void render(int windowWidth, int windowHeight);

    void setPlayCallback(PlayCallback callback);
    void setQuitCallback(QuitCallback callback);

private:
    void loadBackgroundTexture();
    void renderBackground(int windowWidth, int windowHeight);
    void renderMenuButtons(int windowWidth, int windowHeight);
    void handleInput();

    Window& m_window;
    IInputController& m_inputController;

    GLuint m_backgroundTexture = 0;
    int m_bgWidth = 0;
    int m_bgHeight = 0;

    // Input state
    bool m_f3Pressed = false;
    bool m_f11Pressed = false;
    bool m_cursor_visible = false;

    PlayCallback m_onPlay;
    QuitCallback m_onQuit;
};

#endif // MAIN_MENU_HPP
