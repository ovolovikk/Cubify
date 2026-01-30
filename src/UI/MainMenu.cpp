#include "UI/MainMenu.hpp"

#include <GLFW/glfw3.h>

#include "imgui/imgui.h"


#include "Core/Input/IInputController.hpp"
#include "Core/Window.hpp"
#include "Core/Logging/Log.hpp"
#include "stb_image.h"

MainMenu::MainMenu(GLFWwindow* window, Window& appWindow, IInputController& inputController)
    : m_window(appWindow), m_inputController(inputController)
{
    loadBackgroundTexture();

    LOGI("[MainMenu] Initialized");
}

MainMenu::~MainMenu()
{
    if (m_bgTextureNormal != 0) glDeleteTextures(1, &m_bgTextureNormal);
    if (m_bgTextureVoid != 0) glDeleteTextures(1, &m_bgTextureVoid);

    LOGI("[MainMenu] Destroyed");
}

GLuint MainMenu::loadSingleTexture(const char* path)
{
    int channels = 0;
    int width, height;
    stbi_set_flip_vertically_on_load(false);
    
    unsigned char* data = stbi_load(path, &width, &height, &channels, 4);

    if (!data)
    {
        LOGE("[MainMenu] Failed to load texture: %s", path);
        return 0;
    }

    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    
    glBindTexture(GL_TEXTURE_2D, 0);
    stbi_image_free(data);

    m_bgWidth = width;
    m_bgHeight = height;

    LOGI("[MainMenu] Loaded texture: %s (%dx%d)", path, width, height);
    return textureID;
}

void MainMenu::loadBackgroundTexture()
{
    LOGI("[MainMenu] Loading background textures");
    m_bgTextureNormal = loadSingleTexture("assets/textures/backgrounds/non_void_menu.png");
    m_bgTextureVoid = loadSingleTexture("assets/textures/backgrounds/void_menu.png");
}
void MainMenu::onUpdate(float deltaTime)
{
    m_inputController.update();
    handleInput();
}

void MainMenu::handleInput()
{
    // Escape - quit
    if (m_inputController.wasKeyJustPressed(GLFW_KEY_ESCAPE)) {
        if (m_onQuit) m_onQuit();
    }

    // F11 - fullscreen toggle
    if (m_inputController.wasKeyJustPressed(GLFW_KEY_F11)) {
        m_window.toggleFullscreen();
    }

    // F3 - enable cursor
    if (m_inputController.wasKeyJustPressed(GLFW_KEY_F3)) {
        m_cursor_visible = !m_cursor_visible;
        m_inputController.setCursorEnabled(m_cursor_visible);
    }

}

void MainMenu::render(int windowWidth, int windowHeight)
{
    renderBackground(windowWidth, windowHeight);
    renderMenuButtons(windowWidth, windowHeight);
}

void MainMenu::renderMenuButtons(int windowWidth, int windowHeight)
{
    ImVec2 buttonSize(280, 55);
    float panelPadding = 40.f;
    float panelWidth = buttonSize.x + (panelPadding * 2.0f);
    float titleHeight = 30.f;
    float subtitleHeight = 25.f;
    float separatorHeight = 20.f;
    float spacingTotal = 80.f;
    float verticalPadding = 100.f;
    float panelHeight = titleHeight + subtitleHeight + (buttonSize.y * 5) + (separatorHeight * 2) + spacingTotal + verticalPadding;
    float centerX = (windowWidth - panelWidth) * 0.5f;
    float centerY = (windowHeight - panelHeight) * 0.5f;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 15.f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(panelPadding, 25.f));
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 10.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(20, 12));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(10, 12));

    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.05f, 0.05f, 0.1f, 0.85f));
    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.3f, 0.5f, 0.8f, 0.5f));
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.15f, 0.25f, 0.45f, 0.9f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.25f, 0.45f, 0.75f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.1f, 0.3f, 0.6f, 1.0f));

    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.9f, 0.9f, 0.95f, 1.f));

    ImGui::SetNextWindowPos(ImVec2(centerX, centerY));
    ImGui::SetNextWindowSize(ImVec2(panelWidth, panelHeight));

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration
                           | ImGuiWindowFlags_NoMove
                           | ImGuiWindowFlags_NoSavedSettings;

    ImGui::Begin("##MenuButtons", nullptr, flags);

    float totalWidth = ImGui::GetWindowSize().x;
    float titleWidth = ImGui::CalcTextSize("C U B I F Y").x;
    ImGui::SetCursorPosX((totalWidth - titleWidth) * 0.5f);
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.4f, 0.7f, 1.0f, 1.0f));
    ImGui::Text("C U B I F Y");
    ImGui::PopStyleColor();
    
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    float subtitleWidth = ImGui::CalcTextSize("Select Your World").x;
    ImGui::SetCursorPosX((totalWidth - subtitleWidth) * 0.5f);
    ImGui::Text("Select Your World");
    
    ImGui::Spacing();

    float buttonOffsetX = (totalWidth - buttonSize.x) * 0.5f;

    auto drawWorldButton = [&](const char* label, WorldType type) {
        ImGui::SetCursorPosX(buttonOffsetX);
        if (ImGui::Button(label, buttonSize)) {
            if (m_onPlay) m_onPlay(type, is_void_mode);
        }
    };

    drawWorldButton("Minecraft", WorldType::MINECRAFT);
    drawWorldButton("Sector-R", WorldType::SECTORR);
    drawWorldButton("Utopia", WorldType::UTOPIA);

    ImGui::Spacing();

    ImGui::SetCursorPosX(buttonOffsetX);
    
    bool show_void_style = is_void_mode;

    if (show_void_style)
    {
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.5f, 0.2f, 0.9f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.6f, 0.3f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.15f, 0.4f, 0.15f, 1.0f));
    }

    if (ImGui::Button(is_void_mode ? "Void Mode: [ON]" : "Void Mode: [OFF]", buttonSize))
    {
        is_void_mode = !is_void_mode;
    }

    if (show_void_style)
    {
        ImGui::PopStyleColor(3);
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.4f, 0.15f, 0.15f, 0.9f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.6f, 0.2f, 0.2f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.5f, 0.1f, 0.1f, 1.0f));
    ImGui::SetCursorPosX(buttonOffsetX);
    if (ImGui::Button("Quit", buttonSize))
    {
        if (m_onQuit) m_onQuit();
    }
    
    ImGui::PopStyleColor(3);

    ImGui::End();

    ImGui::PopStyleColor(6); 
    ImGui::PopStyleVar(5);
}

void MainMenu::renderBackground(int windowWidth, int windowHeight)
{
    GLuint activeTexture = is_void_mode ? m_bgTextureVoid : m_bgTextureNormal;
    if (activeTexture == 0) activeTexture = m_bgTextureNormal; 
    if (activeTexture == 0) return;

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
    
    ImGui::Image((ImTextureID)(intptr_t)activeTexture, ImVec2((float)windowWidth, (float)windowHeight));
    
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