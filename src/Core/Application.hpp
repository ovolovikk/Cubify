#ifndef APPLICATION_HPP
#define APPLICATION_HPP

#include "Core/AppState.hpp"

#include <memory>
#include <string>
#include <vector>
#include <functional>

class Window;
class Renderer;
class Game;
class MainMenu;

struct ApplicationConfig
{
    std::string title = "Cubify";
    int width = 1920;
    int height = 1080;
    bool vsync = false;
};

// Only one APP => Singleton
class Application
{
public:
    using ShutdownCallback = std::function<void()>;

    static Application &Get();
    static bool Exists();
    static Application& Create(const ApplicationConfig& config);
    static void Destroy();

    Application(const Application&) = delete;
    Application& operator=(const Application&) = delete;
    Application(Application&&) = delete;
    Application& operator=(Application&&) = delete;

    // lifecycle
    void run();
    void quit();

    Window& getWindow();
    Renderer& getRenderer();
    bool is_running() const;
    float getDeltaTime() const;
    double getTime() const;

    void registerShutdownCallBack(ShutdownCallback callback);

private:
    explicit Application(const ApplicationConfig& config);
    ~Application();

    void initSubsystems();
    void shutdownSubsystems();

    void beginFrame();
    void endFrame();

private:
    static Application* s_instance;
     
    ApplicationConfig m_config;
    AppState m_currentState = AppState::UNINITIALIZED;
    float m_deltaTime = 0.f;
    double m_lastFrameTime = 0.0;

    std::unique_ptr<Window> m_window;
    std::unique_ptr<Renderer> m_renderer;
    std::unique_ptr<Game> m_game;
    std::unique_ptr<MainMenu> m_mainMenu;

    // delete in order which is reversed to initialization
    std::vector<ShutdownCallback> m_shutdownCallbacks;
};

#define APP Application::Get()

#endif // APPLICATION_HPP
