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
            buildDefaultScene();
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
        Nimbus::registerAllGameplayScripts(getScriptFieldRegistry(), fieldRegistry);
        Nebula::Application::onStartup();
    }

    void onRender() override
    {
        Nebula::Application::onRender();
    }

private:
    void buildDefaultScene()
    {
        Nebula::Scene &scene = getScene();

        m_groundEntity = scene.createEntity();
        scene.addComponent<Nebula::TagComponent>(m_groundEntity).tag = Nimbus::kGroundTag;
        auto &groundTransform = scene.addComponent<Nebula::TransformComponent>(m_groundEntity);
        groundTransform.transform.setPosition(Nebula::Vec3{0.0f, 0.0f, 0.0f});
        groundTransform.transform.setYaw(0.0f);
        groundTransform.transform.setScale(1.0f);
        auto &groundMesh = scene.addComponent<Nebula::MeshRendererComponent>(m_groundEntity);
        groundMesh.m_meshPath = "builtin/meshes/ground";
        groundMesh.m_materialPath = "builtin/materials/ground";
        scene.addComponent<Nebula::ScriptComponent>(m_groundEntity).scriptName = "Ground";

        m_cubeEntity = scene.createEntity();
        scene.addComponent<Nebula::TagComponent>(m_cubeEntity).tag = Nimbus::kPlayerTag;
        auto &cubeTransform = scene.addComponent<Nebula::TransformComponent>(m_cubeEntity);
        cubeTransform.transform.setPosition(Nebula::Vec3{0.0f, 0.5f, 0.0f});
        cubeTransform.transform.setYaw(0.0f);
        cubeTransform.transform.setScale(1.0f);
        auto &cubeMesh = scene.addComponent<Nebula::MeshRendererComponent>(m_cubeEntity);
        cubeMesh.m_meshPath = "builtin/meshes/cube";
        cubeMesh.m_materialPath = "builtin/materials/cube";
        scene.addComponent<Nebula::ScriptComponent>(m_cubeEntity).scriptName = "Player";

        m_cameraEntity = scene.createEntity();
        scene.addComponent<Nebula::TagComponent>(m_cameraEntity).tag = Nimbus::kMainCameraTag;
        scene.addComponent<Nebula::TransformComponent>(m_cameraEntity);
        auto &cameraComponent = scene.addComponent<Nebula::CameraComponent>(m_cameraEntity);
        cameraComponent.pivotOffset = Nebula::Vec3{0.0f, 0.35f, 0.0f};
        cameraComponent.distance = 6.0f;
        cameraComponent.yaw = 0.7f;
        cameraComponent.pitch = -0.3f;
        cameraComponent.fov = 55.0f;
        cameraComponent.nearClip = 0.1f;
        cameraComponent.farClip = 100.0f;
        cameraComponent.targetTag = Nimbus::kPlayerTag;
        cameraComponent.targetEntity = m_cubeEntity;
        scene.addComponent<Nebula::ScriptComponent>(m_cameraEntity).scriptName = "MainCamera";
    }

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
