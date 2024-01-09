#include <spectr/render_gl/ImguiUtils.h>

namespace spectr::render_gl
{
void ImguiUtils::initImgui(GLFWwindow* window)
{
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    auto context = ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    // Setup Platform/Renderer bindings
    auto res1 = ImGui_ImplGlfw_InitForOpenGL(window, true);
    auto res2 = ImGui_ImplOpenGL3_Init();

    io.Fonts->AddFontDefault();

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
}
}
