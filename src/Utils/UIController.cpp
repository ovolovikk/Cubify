#include "Utils/UIController.hpp"

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "Helpers/imgui/imgui.h"
#include "Helpers/imgui/imgui_impl_glfw.h"
#include "Helpers/imgui/imgui_impl_opengl3.h"

UIController::UIController(GLFWwindow* window)
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    
    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("version 430");
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
    static bool wireframeCheckBox = false;
    float frustumFarBoundary = 500.0f;
}