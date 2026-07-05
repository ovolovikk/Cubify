#ifndef APPLICATION_HPP
#define APPLICATION_HPP

#include "Core/AppState.hpp"
#include "UI/DebugUI.hpp"
#include "World/WorldType.hpp"


class Window;
class IRendererBackend;
class Game;
class MainMenu;
class IInputController;
class DebugUI;

struct ApplicationConfig
{
    std::string title = "Cubify";
    int width = 1920;
    int height = 1080;
    bool vsync = false;
    bool testMode = false;
};

// Singleton for splitting engine and application level
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
    void returnToMenu();

    Window& getWindow();
    IRendererBackend& getRenderer();
    DebugUI& getDebugUI() { return *m_debug_ui; }
    bool is_running() const;
    float getDeltaTime() const;
    double getTime() const;

    void registerShutdownCallBack(ShutdownCallback callback);

private:
    explicit Application(const ApplicationConfig& config);
    ~Application();

    void initSubsystems();
    void shutdownSubsystems();

    std::unique_ptr<IRendererBackend> createRenderer(int width, int height, bool isVoidMode);

    void beginFrame();
    void endFrame();

private:
    static Application* s_instance;
     
    ApplicationConfig m_config;
    AppState m_currentState = AppState::UNINITIALIZED;
    WorldType m_selectedWorldType = WorldType::MINECRAFT;
    float m_deltaTime = 0.f;
    double m_lastFrameTime = 0.0;

    std::unique_ptr<Window> m_window;
    std::unique_ptr<IRendererBackend> m_renderer;
    std::unique_ptr<IInputController> m_inputController;
    std::unique_ptr<Game> m_game;
    std::unique_ptr<MainMenu> m_main_menu;
    std::unique_ptr<DebugUI> m_debug_ui;

    // delete in order which is reversed to initialization
    std::vector<ShutdownCallback> m_shutdownCallbacks;
};

#define APP Application::Get()

#endif // APPLICATION_HPP
