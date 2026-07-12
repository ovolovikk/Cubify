#include "Core/Application.hpp"

#include "Core/AppState.hpp"
#include "Core/Window.hpp"
#include "Core/Game.hpp"
#include "Core/Input/GLFWInputController.hpp"
#include "Core/Sound/AudioEngine.hpp"
#include "Graphics/GraphicsApi.hpp"
#include "Graphics/OpenGLBackend/GLRenderer.hpp"
#include "UI/MainMenu.hpp"
#include "Core/Logging/Log.hpp"
#include "Utils/Config.hpp"
#include "miniaudio.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include "PrecompilerHeader.hpp"

#include <GL/glew.h>
#include <GLFW/glfw3.h>

namespace
{
bool SaveBackbufferToPng(const char* filePath, int width, int height)
{
    if (width <= 0 || height <= 0)
    {
        return false;
    }

    std::vector<uint8_t> pixels(static_cast<size_t>(width) * static_cast<size_t>(height) * 4u);

    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glReadBuffer(GL_BACK);
    glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());

    stbi_flip_vertically_on_write(1);
    return stbi_write_png(filePath, width, height, 4, pixels.data(), width * 4) != 0;
}
}

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
            m_game->onUpdate(m_deltaTime);
            m_game->onRender();

            if (m_game->isReadyForTest())
            {
                endFrame();
                std::this_thread::sleep_for(std::chrono::milliseconds(500));

                beginFrame();
                m_game->onUpdate(m_deltaTime);
                m_game->onRender();

                glFinish();
                screenshotCaptured = SaveBackbufferToPng(outputScreenPath, m_window->getWidth(), m_window->getHeight());

                if (screenshotCaptured)
                {
                    LOGI("[Application][TestMode] Saved screenshot: %s", outputScreenPath);
                }
                else
                {
                    LOGE("[Application][TestMode] Failed to save screenshot: %s", outputScreenPath);
                }

                endFrame();
                break;
            }

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

    while(m_currentState != AppState::SHUTTING_DOWN && m_window->isOpen())
    {
        if(m_currentState == AppState::MENU)
        {
            LOGI("=== Application Menu started ===");
            AudioEngine::Instance().PlayMusic("assets/sounds/main_menu_theme.ogg");
            
            if(!m_main_menu)
            {
                m_main_menu = std::make_unique<MainMenu>(m_window->GetGLFWWindow(), *m_window, *m_inputController);
                m_main_menu->setPlayCallback([this](WorldType worldType, bool is_void_mode) {
                    m_selectedWorldType = worldType;
                    m_currentState = AppState::PLAYING;
                    LOGI("[Application] Selected world type: %d", static_cast<int>(worldType));

                    LOGI("[Subsystem] Initializing Renderer");
                    m_renderer = createRenderer(m_window->getWidth(), m_window->getHeight(), is_void_mode);

                    LOGI("[Subsystem] Initializing Game with world type: %d", static_cast<int>(worldType));
                    m_game = std::make_unique<Game>(*m_window, *m_renderer, *m_inputController, worldType);
                });
                m_main_menu->setQuitCallback([this]() {
                    m_currentState = AppState::SHUTTING_DOWN;
                });
            }
            m_inputController->update();
            
            while(m_currentState == AppState::MENU && m_window->isOpen())
            {
                beginFrame();

                if(m_inputController && m_inputController->wasKeyJustPressed(GLFW_KEY_F5)) {
                    if(m_debug_ui) m_debug_ui->toggleVisible();
                }

                if(m_debug_ui) m_debug_ui->begin();
                    
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                
                if(m_main_menu)
                {
                    m_main_menu->onUpdate(m_deltaTime);
                    m_main_menu->render(m_window->getWidth(), m_window->getHeight());
                }

                if(m_debug_ui) {
                    m_debug_ui->renderAppInfo();
                    m_debug_ui->end();
                }

                endFrame();
            }
            LOGI("=== Application Menu closed ===");
        }
        
        if(m_currentState == AppState::PLAYING)
        {
            m_main_menu.reset();
            LOGI("=== Application Main loop started ===");
            
            while(m_currentState == AppState::PLAYING && m_window->isOpen())
            {
                beginFrame();

                if(m_inputController && m_inputController->wasKeyJustPressed(GLFW_KEY_F5)) {
                    if(m_debug_ui) m_debug_ui->toggleVisible();
                }

                if(m_debug_ui) m_debug_ui->begin();

                if(m_game)
                {
                    m_game->onUpdate(m_deltaTime);
                }
                
                if(m_game)
                {
                    m_game->onRender();
                }

                if(m_debug_ui) {
                    m_debug_ui->renderAppInfo();
                    if(m_game) m_game->onRenderDebug(m_debug_ui.get());
                    m_debug_ui->end();
                }

                endFrame();
            }

            LOGI("=== Application Main loop ended ===");
            LOGI("[Application] Safe destruction of Game/Renderer starting");
            m_game.reset();
            m_renderer.reset();
            LOGI("[Application] Safe destruction complete");
        }
    }
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
    m_currentState = AppState::MENU;
    LOGI("[Application] State set to MENU (cleanup deferred)");
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
    GraphicsApi api = graphicsApiFromString(Config::Get().gConfig.rendererBackend);
    if (api == GraphicsApi::DirectX12) 
    {
        api = GraphicsApi::OpenGL;
    }
    m_window = std::make_unique<Window>(m_config.title, api, m_config.width, m_config.height);
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

    LOGI("[Subsystem] Initializing DebugUI");
    m_debug_ui = std::make_unique<DebugUI>(m_window->GetGLFWWindow());

    LOGI("[Subsystem] Initializing MainMenu");
    m_main_menu = std::make_unique<MainMenu>(m_window->GetGLFWWindow(), *m_window, *m_inputController);
    m_main_menu->setPlayCallback([this](WorldType worldType, bool is_void_mode) {
        m_selectedWorldType = worldType;
        m_currentState = AppState::PLAYING;
        LOGI("[Application] Selected world type: %d", static_cast<int>(worldType));

        LOGI("[Subsystem] Initializing Renderer");
        m_renderer = createRenderer(m_window->getWidth(), m_window->getHeight(), is_void_mode);

        LOGI("[Subsystem] Initializing Game with world type: %d", static_cast<int>(worldType));
        m_game = std::make_unique<Game>(*m_window, *m_renderer, *m_inputController, worldType);
    });
    m_main_menu->setQuitCallback([this]() {
        m_currentState = AppState::SHUTTING_DOWN;
    });

    m_currentState = AppState::MENU;
    m_lastFrameTime = getTime();

    LOGI("=== All Subsystems initialized ===");
}

std::unique_ptr<IRendererBackend> Application::createRenderer(int width, int height, bool isVoidMode)
{
    const std::string& backend = Config::Get().gConfig.rendererBackend;
    if (backend == "directx12")
    {
        LOGW("[Subsystem] DirectX12 backend is not implemented yet, falling back to OpenGL");
    }

    return std::make_unique<GLRenderer>(width, height, isVoidMode);
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
    m_window->swapBuffers();
    m_window->pollEvents();
}