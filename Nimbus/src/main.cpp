#include <iostream>

#include <Nebula.h>

#include "register_Script.h"
#include "sceneDefaults.h"

class NimbusApp final : public Nebula::Application
{
public:
  explicit NimbusApp(const Nebula::ApplicationSpec &spec)
      : Nebula::Application(spec)
  {
    Nebula::Scene &scene = getScene();
    if (!Nebula::SceneSerializer::load(scene, getAssets(), m_scenePath))
    {
      scene.clear();
      Nimbus::buildVerticalSliceScene(scene);
      Nebula::SceneSerializer::save(scene, getAssetManager(), getAssets(), m_scenePath);
      std::cout << "[Scene] Created vertical slice at " << m_scenePath << '\n';
    }
    else
    {
      std::cout << "[Scene] Loaded " << m_scenePath << '\n';
    }
  }

protected:
  void onStartup() override
  {
    setPlaying(true);
    Nimbus::registerAllGameplayScripts(getScriptRegistry(), getScriptFieldRegistry());
    Nebula::Application::onStartup();
  }

private:
  std::string m_scenePath = "scenes/vertical_slice.json";
};

int main()
{
  Nebula::ApplicationSpec spec;
  spec.title = "Nimbus";
  spec.width = 1280;
  spec.height = 720;

  NimbusApp app(spec);
  if (!app.getWindow().isValid())
  {
    std::cerr << "Window or OpenGL context failed to initialize. Exiting.\n";
    return 1;
  }

  app.run();
  return 0;
}
