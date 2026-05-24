#include "imGuiLayer.h"

#include "Window.h"

#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

namespace NebulaEditor
{

  bool ImGuiLayer::s_initialized = false;

  static GLFWwindow *nativeGlfwWindow(Nebula::Window &window)
  {
    // Window doesn't expose getNativeHandle() yet; this is the engine's internal accessor.
    return static_cast<GLFWwindow *>(Nebula::detail::nativeWindowHandleForInput(window));
  }

  void ImGuiLayer::init(Nebula::Window &window)
  {
    if (s_initialized)
    {
      return;
    }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGuiIO &io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    ImGui::StyleColorsDark();

    GLFWwindow *glfwWindow = nativeGlfwWindow(window);
    ImGui_ImplGlfw_InitForOpenGL(glfwWindow, true);
    ImGui_ImplOpenGL3_Init("#version 410"); // matches your GL 4.1 context; 330 also works

    s_initialized = true;
  }

  void ImGuiLayer::beginFrame()
  {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
  }

  void ImGuiLayer::endFrame()
  {
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
  }

  void ImGuiLayer::shutdown()
  {
    if (!s_initialized)
    {
      return;
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    s_initialized = false;
  }

} // namespace NebulaEditor