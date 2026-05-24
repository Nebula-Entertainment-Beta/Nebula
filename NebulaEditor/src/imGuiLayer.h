#pragma once

namespace Nebula
{
  class Window;
}

namespace NebulaEditor
{

  class ImGuiLayer
  {
  public:
    static void init(Nebula::Window &window);
    static void shutdown();

    static void beginFrame(); // NewFrame chain
    static void endFrame();   // Render + draw to GL

  private:
    static bool s_initialized;
  };

} // namespace NebulaEditor