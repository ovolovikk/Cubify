#include "UI/MainMenu.hpp"

#include <GLFW/glfw3.h>

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

#include "Core/Input/IInputController.hpp"
#include "Core/Window.hpp"
#include "Core/Logging/Log.hpp"
#include "stb_image.h"

MainMenu::MainMenu(GLFWwindow* window, Window& appWindow, IInputController& inputController)
    : m_window(appWindow), m_inputController(inputController)
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    ImFont* spaceFont = io.Fonts->AddFontFromFileTTF("fonts/space_font.ttf");
    if(!spaceFont)
    {
        LOGE("[MainMenu] Failed to load space font");
    }
    ImGui::PushFont(spaceFont);

    ImGui::StyleColorsDark();
    
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding = 8.0f;
    style.FrameRounding = 4.0f;
    style.FramePadding = ImVec2(12, 6);
    style.ItemSpacing = ImVec2(10, 10);

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 430 core");

    loadBackgroundTexture();

    LOGI("[MainMenu] Initialized");
}

MainMenu::~MainMenu()
{
    if (m_backgroundTexture != 0)
    {
        glDeleteTextures(1, &m_backgroundTexture);
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    LOGI("[MainMenu] Destroyed");
}

void MainMenu::loadBackgroundTexture()
{
    LOGI("[MainMenu] Loading background texture");
    int chanells = 0;
    stbi_set_flip_vertically_on_load(false);
    unsigned char* data = stbi_load("textures/backgrounds/menu_background.png", &m_bgWidth, &m_bgHeight, &chanells, 4);

    if (!data)
    {
        LOGE("[MainMenu] Failed to load background texture");
        return;
    }

    glGenTextures(1, &m_backgroundTexture);
    glBindTexture(GL_TEXTURE_2D, m_backgroundTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_bgWidth, m_bgHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_2D, 0);

    stbi_image_free(data);

    LOGI("[MainMenu] Background texture loaded (%dx%d)", m_bgWidth, m_bgHeight);
}

void MainMenu::onUpdate(float deltaTime)
{
    m_inputController.update();
    handleInput();
}

void MainMenu::handleInput()
{
    // Escape - quit
    if (m_inputController.isKeyPressed(GLFW_KEY_ESCAPE)) {
        if (m_onQuit) m_onQuit();
    }

    // F11 - fullscreen toggle
    if (m_inputController.isKeyPressed(GLFW_KEY_F11)) {
        if (!m_f11Pressed) {
            m_window.toggleFullscreen();
            m_f11Pressed = true;
        }
    } else {
        m_f11Pressed = false;
    }

    // F3 - enable cursor
    if (m_inputController.isKeyPressed(GLFW_KEY_F3)) {
        if (!m_f3Pressed) {
            m_cursor_visible = !m_cursor_visible;
            m_inputController.setCursorEnabled(m_cursor_visible);
            m_f3Pressed = true;
        }
    } else {
        m_f3Pressed = false;
    }

}

void MainMenu::render(int windowWidth, int windowHeight)
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    renderBackground(windowWidth, windowHeight);
    renderMenuButtons(windowWidth, windowHeight);

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void MainMenu::renderMenuButtons(int windowWidth, int windowHeight)
{
    ImVec2 buttonSize(200, 50);
    float centerX = (windowWidth - buttonSize.x) * 0.5f;
    float centerY = windowHeight * 0.5f;

    ImGui::SetNextWindowPos(ImVec2(centerX, centerY));
    ImGui::SetNextWindowSize(ImVec2(buttonSize.x + 20, buttonSize.y * 3 + 40));

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration 
                           | ImGuiWindowFlags_NoMove 
                           | ImGuiWindowFlags_NoSavedSettings
                           | ImGuiWindowFlags_NoBackground;

    ImGui::Begin("##MenuButtons", nullptr, flags);

    if (ImGui::Button("Play", buttonSize))
    {
        if (m_onPlay) m_onPlay();
    }

    if (ImGui::Button("Quit", buttonSize))
    {
        if (m_onQuit) m_onQuit();
    }

    ImGui::End();
}

void MainMenu::renderBackground(int windowWidth, int windowHeight)
{
    if (m_backgroundTexture == 0) return;

    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImVec2((float)windowWidth, (float)windowHeight));

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration 
                           | ImGuiWindowFlags_NoMove 
                           | ImGuiWindowFlags_NoSavedSettings
                           | ImGuiWindowFlags_NoBringToFrontOnFocus
                           | ImGuiWindowFlags_NoFocusOnAppearing
                           | ImGuiWindowFlags_NoBackground;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::Begin("##Background", nullptr, flags);
    
    ImGui::Image((ImTextureID)(intptr_t)m_backgroundTexture, ImVec2((float)windowWidth, (float)windowHeight));
    
    ImGui::End();
    ImGui::PopStyleVar();
}

void MainMenu::setPlayCallback(PlayCallback callback)
{
    m_onPlay = std::move(callback);
}

void MainMenu::setQuitCallback(QuitCallback callback)
{
    m_onQuit = std::move(callback);
}