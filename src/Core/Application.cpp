#include "Core/Application.hpp"

#include "Core/AppState.hpp"
#include "Core/Window.hpp"
#include "Core/Game.hpp"
#include "Core/Input/GLFWInputController.hpp"
#include "Core/Sound/AudioEngine.hpp"
#include "Graphics/DirectX12Backend/DX12Renderer.hpp"
#include "Core/Logging/Log.hpp"
#include "Utils/Config.hpp"
#include "miniaudio.h"

#include "stb_image_write.h"

#include "PrecompilerHeader.hpp"

#include <GLFW/glfw3.h>

Application* Application::s_instance = nullptr;

Application &Application::Get()
{
    return *s_instance;
}

bool Application::Exists()
{
    if(s_instance != nullptr) return true;
    LOGE("[Application] Tried access instance which not exists");
    return false;
}

Application& Application::Create(const ApplicationConfig& config)
{
    if(s_instance == nullptr)
    {
        LOGI("[Application] Created Application instance");
        s_instance = new Application(config);
    }
    return *s_instance;
}

void Application::Destroy()
{
    if(s_instance != nullptr)
    {
        LOGI("[Application] Destroyed Application instance");
        delete s_instance;
        s_instance = nullptr;
    }
    return;
}

void Application::run()
{
    if(m_currentState == AppState::UNINITIALIZED || !m_window || !m_window->isOpen())
    {
        LOGE("[Application] Can't run, not properly initialized");
        return;
    }

    if (m_config.testMode)
    {
        static constexpr const char* outputScreenPath = "test_output.png";

        LOGI("[Application][TestMode] Running screenshot capture flow");

        if (!m_renderer)
        {
            m_renderer = createRenderer(m_window->getWidth(), m_window->getHeight(), false);
        }

        if (!m_game)
        {
            m_game = std::make_unique<Game>(*m_window, *m_renderer, *m_inputController, m_selectedWorldType, true);
        }

        bool screenshotCaptured = false;

        while (true)
        {
            if (!m_window->isOpen())
            {
                break;
            }

            beginFrame();
            // Opened before the update so chunk uploads land in this frame's
            // command list, same as the normal loop does
            m_renderer->beginFrame();
            m_game->onUpdate(m_deltaTime);
            m_game->onRender();

            if (m_game->isReadyForTest())
            {
                m_renderer->endFrame();
                endFrame();
                std::this_thread::sleep_for(std::chrono::milliseconds(500));

                beginFrame();
                m_renderer->beginFrame();
                m_game->onUpdate(m_deltaTime);
                m_game->onRender();

                // Has to happen while the frame is still recording
                screenshotCaptured = m_renderer->captureBackbuffer(outputScreenPath);

                if (screenshotCaptured)
                {
                    LOGI("[Application][TestMode] Saved screenshot: %s", outputScreenPath);
                }
                else
                {
                    LOGE("[Application][TestMode] Failed to save screenshot: %s", outputScreenPath);
                }

                m_renderer->endFrame();
                endFrame();
                break;
            }

            m_renderer->endFrame();
            endFrame();
        }

        if (!screenshotCaptured)
        {
            LOGE("[Application][TestMode] Could not capture screenshot before window close");
            m_currentState = AppState::SHUTTING_DOWN;
            return;
        }

        m_currentState = AppState::SHUTTING_DOWN;
        return;
    }

    // TODO: Add a proper DX12 menu rendering flow
    LOGI("=== Application Main loop started ===");
    while (m_window->isOpen() && m_currentState == AppState::PLAYING)
    {
        beginFrame();

        m_renderer->beginFrame();
        m_game->onUpdate(m_deltaTime);
        m_game->onRender();
        m_renderer->endFrame();

        endFrame();
    }
    LOGI("=== Application Main loop ended ===");

    m_game.reset();
    m_renderer.reset();
    m_currentState = AppState::SHUTTING_DOWN;
}

void Application::quit()
{
    LOGI("[Application] QUIT requested");
    m_currentState = AppState::SHUTTING_DOWN;
}

void Application::returnToMenu()
{
    LOGI("[Application] Returning to menu");
    AudioEngine::Instance().StopMusic();
    m_currentState = AppState::SHUTTING_DOWN;
    LOGI("[Application] No menu to return to yet, shutting down");
}

Window &Application::getWindow()
{
    if (m_window == nullptr)
    {
        LOGE("[Application] Window is nullptr");
    }
    return *m_window;
}

IRendererBackend &Application::getRenderer()
{
    if (m_renderer == nullptr)
    {
        LOGE("[Application] Renderer is nullptr");
    }
    return *m_renderer;
}

bool Application::is_running() const
{
    return m_currentState == AppState::PLAYING || m_currentState == AppState::PAUSED;
}

float Application::getDeltaTime() const
{
    return m_deltaTime;
}

double Application::getTime() const
{
    return glfwGetTime();
}

void Application::registerShutdownCallBack(ShutdownCallback callback)
{
    m_shutdownCallbacks.push_back(std::move(callback));
}

Application::Application(const ApplicationConfig& config)
    : m_config(config)
{
    LOGI("=== Application Initializing ===");
    initSubsystems();
}

Application::~Application()
{
    LOGI("=== Application Shutting down ===");
    shutdownSubsystems();
}

void Application::initSubsystems()
{
    LOGI("[Subsystem] Initializing Window");
    m_window = std::make_unique<Window>(m_config.title, m_config.width, m_config.height);
    if(!m_window->isOpen())
    {
        LOGE("[Subsystem] Failed to create Window. Aborting");
    }

    m_window->setResizeCallback([this](int w, int h) {
        if(m_renderer)
        {
            m_renderer->onResize(w, h);
        }
        if(m_game)
        {
            m_game->onResize(w, h);
        }
    });

    LOGI("[Subsystem] Initializing InputController");
    m_inputController = std::make_unique<GLFWInputController>(m_window->GetGLFWWindow());

    LOGI("[Subsystem] Initializing DirectX12 Renderer");
    m_renderer = createRenderer(m_window->getWidth(), m_window->getHeight(), false);

    LOGI("[Subsystem] Initializing Game with world type: %d", static_cast<int>(m_selectedWorldType));
    m_game = std::make_unique<Game>(*m_window, *m_renderer, *m_inputController, m_selectedWorldType);

    m_currentState = AppState::PLAYING;
    m_lastFrameTime = getTime();

    LOGI("=== All Subsystems initialized ===");
}

// TODO: Remove isVoidMode or make it real option in DX12
std::unique_ptr<IRendererBackend> Application::createRenderer(int width, int height, bool isVoidMode)
{
    return std::make_unique<Cubify::DX12::DX12Renderer>(m_window->nativeWindowHandle(), width, height);
}

void Application::shutdownSubsystems()
{
    // Shutting down in reverse order: Renderer->Game->Window

    // Call registered shutdown callbacks in reverse order
    LOGI("[Shutdown] Executing %zu shutdown callbacks...", m_shutdownCallbacks.size());
    for (auto it = m_shutdownCallbacks.rbegin(); it != m_shutdownCallbacks.rend(); ++it)
    {
        (*it)();
    }
    m_shutdownCallbacks.clear();

    LOGI("[Subsystem] Shutting down Renderer");
    m_renderer.reset();

    LOGI("[Subsystem] Shutting down Game");
    m_game.reset();

    LOGI("[Subsystem] Shutting down InputController");
    m_inputController.reset();

    LOGI("[Subsystem] Shutting down Window");
    m_window.reset();
}

void Application::beginFrame()
{
    double currentTime = glfwGetTime();
    m_deltaTime =  static_cast<float>(currentTime - m_lastFrameTime);
    m_lastFrameTime = currentTime;
}

void Application::endFrame()
{
    m_window->pollEvents();
}