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
    //Style config
    ImVec2 buttonSize(280, 55);
    float panelPadding = 40.f;
    float panelWidth = buttonSize.x + (panelPadding * 2.0f);
    float titleHeight = 30.f;
    float subtitleHeight = 25.f;
    float separatorHeight = 20.f;
    float spacingTotal = 80.f;
    float verticalPadding = 50.f;
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

    // --- Заголовок ---
    float titleWidth = ImGui::CalcTextSize("C U B I F Y").x;
    ImGui::SetCursorPosX((totalWidth - titleWidth) * 0.5f);
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.4f, 0.7f, 1.0f, 1.0f));
    ImGui::Text("C U B I F Y");
    ImGui::PopStyleColor();
    
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // --- Подзаголовок ---
    float subtitleWidth = ImGui::CalcTextSize("Select Your World").x;
    ImGui::SetCursorPosX((totalWidth - subtitleWidth) * 0.5f);
    ImGui::Text("Select Your World");
    
    ImGui::Spacing();

    // --- Кнопки миров ---
    float buttonOffsetX = (totalWidth - buttonSize.x) * 0.5f;

    auto drawWorldButton = [&](const char* label, WorldType type) {
        ImGui::SetCursorPosX(buttonOffsetX);
        if (ImGui::Button(label, buttonSize)) {
            if (m_onPlay) m_onPlay(type);
        }
    };

    drawWorldButton("Minecraft", WorldType::MINECRAFT);
    drawWorldButton("Edmund's Planet", WorldType::MINECRAFT);
    drawWorldButton("Dr. Mann's Planet", WorldType::MINECRAFT);
    drawWorldButton("Miller's Planet", WorldType::MINECRAFT);

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