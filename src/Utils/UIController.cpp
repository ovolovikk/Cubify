#include "Utils/UIController.hpp"

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "Helpers/imgui/imgui.h"
#include "Helpers/imgui/imgui_impl_glfw.h"
#include "Helpers/imgui/imgui_impl_opengl3.h"

#include "Core/Camera.hpp"
#include "World/World.hpp"
#include "Helpers/BlockType.hpp"

#include <string>

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
        default: return "UNKNOWN";
    }
}
}

UIController::UIController(GLFWwindow* window)
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 430 core");

    wireframe = false;
}

UIController::~UIController()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void UIController::update(const Camera& camera, const World& world)
{
    if (wireframe) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    } else {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    showDebugWindow(camera, world);

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void UIController::showDebugWindow(const Camera& camera, const World& world)
{
    ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(300, 180), ImGuiCond_FirstUseEver);
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse;
    ImGui::Begin("Debug", nullptr, flags);

    ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
    ImGui::Checkbox("Wireframe", &wireframe);

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
        ImGui::Text("Prev: %d, %d, %d", res.previous_position.x, res.previous_position.y, res.previous_position.z);
        auto type = world.getBlock(res.block_position.x, res.block_position.y, res.block_position.z);
        ImGui::Text("Block: %s", blockTypeToString(type));
    } else {
        ImGui::Text("No hit within range");
    }

    ImGui::End();
}