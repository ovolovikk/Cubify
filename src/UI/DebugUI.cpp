#include "UI/DebugUI.hpp"

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

#include "Core/Camera.hpp"
#include "Core/Logging/Log.hpp"
#include "World/World.hpp"
#include "Core/BlockType.hpp"
#include "Core/Sound/AudioEngine.hpp"

namespace {
static const char* blockTypeToString(BlockType type)
{
    switch(type) {
        case BlockType::AIR: return "AIR";
        case BlockType::DIRT: return "DIRT";
        case BlockType::STONE: return "STONE";
        case BlockType::GRASS: return "GRASS";
        case BlockType::SAND: return "SAND";
        case BlockType::WOODEN_PLANK: return "WOODEN_PLANK";
        case BlockType::WATER: return "WATER";
        case BlockType::ICE: return "ICE";
        default: return "UNKNOWN";
    }
}
}

DebugUI::DebugUI(GLFWwindow* window)
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    // Load font used by MainMenu
    ImFont* spaceFont = io.Fonts->AddFontFromFileTTF("assets/fonts/space_font.ttf");
    if(!spaceFont) {
        LOGW("[DebugUI] Failed to load space_font.ttf");
    }
    ImGui::StyleColorsDark();
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding = 8.0f;
    style.FrameRounding = 4.0f;

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 430 core");

    m_wireframe = false;
    m_visible = false;
}

DebugUI::~DebugUI()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void DebugUI::begin()
{
    if (m_wireframe) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    } else {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void DebugUI::end()
{
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void DebugUI::renderAppInfo()
{
    if(!m_visible) return;

    ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(300, 100), ImGuiCond_FirstUseEver);
    
    if(ImGui::Begin("Cubify Debug", &m_visible)) {
        ImGui::Text("FPS: %.0f", ImGui::GetIO().Framerate);
        ImGui::Checkbox("Wireframe", &m_wireframe);
        if(ImGui::SliderFloat("Volume:", &m_volume, 0, 100, "%.0f"))
        {
            AudioEngine::Instance().changeVolume(m_volume);
        }
    }
    ImGui::End();
}

void DebugUI::renderGameInfo(const Camera& camera, const World& world)
{
    if(!m_visible) return;

    if(ImGui::Begin("Cubify Debug")) {
        ImGui::Separator();
        ImGui::Text("Camera:");
        auto pos = camera.GetPosition();
        auto front = camera.GetFront();
        ImGui::Text("Pos:  %.2f, %.2f, %.2f", pos.x, pos.y, pos.z);
        ImGui::Text("Front: %.2f, %.2f, %.2f", front.x, front.y, front.z);

        ImGui::Separator();
        ImGui::Text("RayCast:");
        auto res = world.rayCast(camera.GetPosition(), camera.GetFront(), 4.0f);
        if (res.success) {
            ImGui::Text("Hit: %d, %d, %d", res.block_position.x, res.block_position.y, res.block_position.z);
            auto type = world.getBlock(res.block_position.x, res.block_position.y, res.block_position.z);
            ImGui::Text("Block: %s", blockTypeToString(type));
        } else {
            ImGui::Text("No hit");
        }
    }
    ImGui::End();
}
