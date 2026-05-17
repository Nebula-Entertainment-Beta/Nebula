#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <vector>

#include <application.h>
#include <camera3D.h>
#include <component.h>
#include <renderer.h>
#include <sceneSerializer.h>
#include <systemScheduler.h>
#include <transform3D.h>

#include "playerController.h"
#include "register_Script.h"

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
        Nimbus::registerAllGameplayScripts(getScriptRegistry());
        Nebula::Application::onStartup();
    }

    void registerGameSystems() override
    {
        getScheduler().add(Nebula::SystemPhase::Update, [this](float dt)
                           { Nimbus::runPlayerController(getWorld(), m_cubeEntity, m_cameraEntity, m_showInputDebug, m_debugPrintTimer, dt); });

        getScheduler().add(Nebula::SystemPhase::PostUpdate, [this](float)
                           {
                               if (!getWorld().frameInput().saveScene)
                               {
                                   return;
                               }
                               if (saveScene())
                               {
                                   std::cout << "[Scene] Saved to " << m_scenePath << '\n';
                               }
                               else
                               {
                                   std::cerr << "[Scene] Failed to save to " << m_scenePath << '\n';
                               } });
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
        auto &groundTransform = scene.addComponent<Nebula::TransformComponent>(m_groundEntity);
        groundTransform.transform.setPosition(Nebula::Vec3{0.0f, 0.0f, 0.0f});
        groundTransform.transform.setYaw(0.0f);
        groundTransform.transform.setScale(1.0f);
        auto &groundMesh = scene.addComponent<Nebula::MeshRendererComponent>(m_groundEntity);
        groundMesh.m_meshID = Nebula::kBuiltinMeshGround;
        groundMesh.m_materialID = Nebula::kBuiltinMaterialGround;
        scene.addComponent<Nebula::ScriptComponent>(m_groundEntity).scriptName = "Ground";

        m_cubeEntity = scene.createEntity();
        auto &cubeTransform = scene.addComponent<Nebula::TransformComponent>(m_cubeEntity);
        cubeTransform.transform.setPosition(Nebula::Vec3{0.0f, 0.5f, 0.0f});
        cubeTransform.transform.setYaw(0.0f);
        cubeTransform.transform.setScale(1.0f);
        auto &cubeMesh = scene.addComponent<Nebula::MeshRendererComponent>(m_cubeEntity);
        cubeMesh.m_meshID = Nebula::kBuiltinMeshCube;
        cubeMesh.m_materialID = Nebula::kBuiltinMaterialCube;
        scene.addComponent<Nebula::ScriptComponent>(m_cubeEntity).scriptName = "Player";

        m_cameraEntity = scene.createEntity();
        scene.addComponent<Nebula::TransformComponent>(m_cameraEntity);
        auto &cameraComponent = scene.addComponent<Nebula::CameraComponent>(m_cameraEntity);
        cameraComponent.pivotOffset = Nebula::Vec3{0.0f, 0.35f, 0.0f};
        cameraComponent.distance = 6.0f;
        cameraComponent.yaw = 0.7f;
        cameraComponent.pitch = -0.3f;
        cameraComponent.fov = 55.0f;
        cameraComponent.nearClip = 0.1f;
        cameraComponent.farClip = 100.0f;
        scene.addComponent<Nebula::ScriptComponent>(m_cameraEntity).scriptName = "MainCamera";
    }

    bool resolveRuntimeEntities()
    {
        Nebula::Scene &scene = getScene();
        m_groundEntity = {};
        m_cubeEntity = {};
        m_cameraEntity = {};

        std::vector<Nebula::Entity> meshEntities;
        std::vector<Nebula::Entity> cameraEntities;
        for (const Nebula::Entity entity : scene.getAllEntities())
        {
            if (scene.hasComponent<Nebula::ScriptComponent>(entity))
            {
                const std::string &role = scene.getComponent<Nebula::ScriptComponent>(entity).scriptName;
                if (role == "Ground" && scene.hasComponent<Nebula::TransformComponent>(entity) && scene.hasComponent<Nebula::MeshRendererComponent>(entity))
                {
                    m_groundEntity = entity;
                }
                else if (role == "Player" && scene.hasComponent<Nebula::TransformComponent>(entity) && scene.hasComponent<Nebula::MeshRendererComponent>(entity))
                {
                    m_cubeEntity = entity;
                }
                else if (role == "MainCamera" && scene.hasComponent<Nebula::CameraComponent>(entity))
                {
                    m_cameraEntity = entity;
                }
            }

            if (scene.hasComponent<Nebula::CameraComponent>(entity))
            {
                cameraEntities.push_back(entity);
            }
            if (scene.hasComponent<Nebula::TransformComponent>(entity) && scene.hasComponent<Nebula::MeshRendererComponent>(entity))
            {
                meshEntities.push_back(entity);
            }
        }

        if (m_cameraEntity.id == 0 && !cameraEntities.empty())
        {
            m_cameraEntity = cameraEntities.front();
        }

        if (meshEntities.size() >= 2)
        {
            if (m_cubeEntity.id == 0)
            {
                m_cubeEntity = meshEntities.front();
                for (const Nebula::Entity entity : meshEntities)
                {
                    const auto &transform = scene.getComponent<Nebula::TransformComponent>(entity);
                    const auto &cubeTransform = scene.getComponent<Nebula::TransformComponent>(m_cubeEntity);
                    if (transform.transform.getPosition().y > cubeTransform.transform.getPosition().y)
                    {
                        m_cubeEntity = entity;
                    }
                }
            }

            if (m_groundEntity.id == 0)
            {
                m_groundEntity = meshEntities.front();
                if (m_groundEntity.id == m_cubeEntity.id && meshEntities.size() > 1)
                {
                    m_groundEntity = meshEntities[1];
                }
            }
        }

        return m_groundEntity.id != 0 && m_cubeEntity.id != 0 && m_groundEntity.id != m_cubeEntity.id;
    }

    bool saveScene() const
    {
        return Nebula::SceneSerializer::save(getScene(), getAssets(), m_scenePath);
    }

    Nebula::Entity m_groundEntity{};
    Nebula::Entity m_cubeEntity{};
    Nebula::Entity m_cameraEntity{};

    std::string m_scenePath = "scenes/week2_scene.json";
    bool m_showInputDebug = false;
    float m_debugPrintTimer = 0.0f;
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
