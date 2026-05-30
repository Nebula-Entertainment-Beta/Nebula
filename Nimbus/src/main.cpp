#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <vector>

#include <Nebula.h>

#include <variant>

#include "nimbus_config.h"

#include "register_Script.h"
#include "sceneDefaults.h"
#include "scriptFields.h"

class NimbusApp final : public Nebula::Application
{
public:
    explicit NimbusApp(const Nebula::ApplicationSpec &spec)
        : Nebula::Application(spec)
    {
        Nebula::Scene &scene = getScene();
        const bool loaded = Nebula::SceneSerializer::load(scene, getAssets(), m_scenePath);
        if (loaded && resolveRuntimeEntities())
        {
            std::cout << "[Scene] Loaded from " << m_scenePath << '\n';
        }
        else
        {
            scene.clear();
            Nimbus::buildDefaultScene(scene);
            if (saveScene())
            {
                std::cout << "[Scene] Created default scene at " << m_scenePath << '\n';
            }
            else
            {
                std::cout << "[Scene] Failed to save default scene to " << m_scenePath << '\n';
            }
        }
    }

protected:
    void onStartup() override
    {
        Nebula::ScriptFieldRegistry fieldRegistry;
        Nimbus::registerAllGameplayScripts(getScriptRegistry(), getScriptFieldRegistry());
        Nebula::Application::onStartup();
    }

    void onRender() override
    {
        Nebula::Application::onRender();
    }

private:
    bool resolveRuntimeEntities()
    {
        Nebula::Scene &scene = getScene();
        m_groundEntity = Nebula::findByTag(scene, Nimbus::kGroundTag);
        m_cubeEntity = Nebula::findByTag(scene, Nimbus::kPlayerTag);
        m_cameraEntity = Nebula::findByTag(scene, Nimbus::kMainCameraTag);

        if (m_cameraEntity.id == 0)
        {
            const std::vector<Nebula::Entity> cameras =
                Nebula::findAllWith<Nebula::CameraComponent>(scene);
            if (!cameras.empty())
            {
                m_cameraEntity = cameras.front();
            }
        }

        if (m_cameraEntity.id != 0 && scene.hasComponent<Nebula::CameraComponent>(m_cameraEntity))
        {
            auto &cameraComponent = scene.getComponent<Nebula::CameraComponent>(m_cameraEntity);
            if (cameraComponent.targetTag.empty() && m_cubeEntity.id != 0)
            {
                cameraComponent.targetTag = Nimbus::kPlayerTag;
            }
            if (!scene.isValidEntity(cameraComponent.targetEntity) && m_cubeEntity.id != 0)
            {
                cameraComponent.targetEntity = m_cubeEntity;
            }
        }

        return m_groundEntity.id != 0 && m_cubeEntity.id != 0 && m_groundEntity != m_cubeEntity &&
               m_cameraEntity.id != 0;
    }

    bool saveScene() const
    {
        return Nebula::SceneSerializer::save(getScene(), getAssetManager(), getAssets(), m_scenePath);
    }

    Nebula::Entity m_groundEntity{};
    Nebula::Entity m_cubeEntity{};
    Nebula::Entity m_cameraEntity{};

    std::string m_scenePath = "scenes/week2_scene.json";
};

int main()
{
    Nebula::ApplicationSpec spec;
    spec.title = "Nimbus";
    spec.width = 800;
    spec.height = 600;

    NimbusApp app(spec);
    if (!app.getWindow().isValid())
    {
        std::cerr << "Window or OpenGL context failed to initialize. Exiting.\n";
        return 1;
    }

    app.run();
    return 0;
}
