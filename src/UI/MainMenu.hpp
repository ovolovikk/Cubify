#pragma once

#include <GL/glew.h>

#include "World/WorldType.hpp"

struct GLFWwindow;
class IInputController;
class Window;

class MainMenu
{
public:
    using PlayCallback = std::function<void(WorldType, bool)>;
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

    GLuint m_bgTextureNormal = 0;
    GLuint m_bgTextureVoid = 0;

    GLuint loadSingleTexture(const char* path);
    int m_bgWidth = 0;
    int m_bgHeight = 0;

    // Input state
    bool m_cursor_visible = false;
    bool is_void_mode;

    PlayCallback m_onPlay;
    QuitCallback m_onQuit;
};
