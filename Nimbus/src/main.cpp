#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cmath>
#include <filesystem>
#include <iostream>
#include <unordered_map>
#include <vector>

#include <application.h>
#include <component.h>
#include <camera3D.h>
#include <index_buffer.h>
#include <renderer.h>
#include "register_Script.h"
#include <shader.h>
#include <script_Registry.h>
#include <transform3D.h>
#include <vertex_array.h>
#include <vertex_buffer.h>

namespace
{

    static const std::string kVert3D = R"(
#version 410 core
layout (location = 0) in vec3 aPos;
uniform mat4 uMVP;
void main() {
    gl_Position = uMVP * vec4(aPos, 1.0);
}
)";

    static const std::string kFragSolidColor = R"(
#version 410 core
uniform vec3 uColor;
out vec4 FragColor;
void main() {
    FragColor = vec4(uColor, 1.0);
}
)";

    struct Vertex3
    {
        float x, y, z;
    };

} // namespace

class NimbusApp final : public Nebula::Application
{
public:
    explicit NimbusApp(const Nebula::ApplicationSpec &spec)
        : Nebula::Application(spec)
    {
        Nebula::Scene &scene = getScene();
        const bool loaded = scene.loadFromFile(m_scenePath);
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

        Nimbus::registerAllGameplayScripts(m_scriptRegistry);
        rebuildRuntimeScripts();
        initializeRuntimeScripts();
    }
    ~NimbusApp() override
    {
        shutdownRuntimeScripts();
    }

protected:
    void onUpdate(float dt) override
    {
        runInputPhase(dt);
        runSimulationPhase(dt);
        runScriptUpdatePhase(dt);
    }

    void onRender() override
    {
        if (!m_gpuReady)
        {
            if (!ensureGpuResources())
            {
                std::cerr << "GPU resource setup failed.\n";
                return;
            }
            m_gpuReady = true;
        }

        Nebula::Application::onRender();

        int fbw = 0;
        int fbh = 0;
        getWindow().getFramebufferSize(fbw, fbh);
        const float aspect =
            (fbh > 0) ? (static_cast<float>(fbw) / static_cast<float>(fbh)) : (16.0f / 9.0f);

        Nebula::Scene &scene = getScene();
        auto &groundTransform = scene.getComponent<Nebula::TransformComponent>(m_groundEntity);
        auto &cubeTransform = scene.getComponent<Nebula::TransformComponent>(m_cubeEntity);
        auto &cameraComponent = scene.getComponent<Nebula::CameraComponent>(m_cameraEntity);

        Nebula::Camera3D camera;
        camera.setTarget(cubeTransform.transform.getPosition());
        camera.setPivotOffset(cameraComponent.pivotOffset);
        camera.setDistance(cameraComponent.distance);
        camera.setYaw(cameraComponent.yaw);
        camera.setPitch(cameraComponent.pitch);
        camera.setFOV(cameraComponent.fov);
        camera.setNearPlane(cameraComponent.nearClip);
        camera.setFarPlane(cameraComponent.farClip);
        camera.setAspectRatio(aspect);

        const Nebula::Mat4 vp = camera.getViewProjectionMatrix();

        m_shader->bind();

        // Ground: large XZ plane at y = 0 (red)
        {
            const Nebula::Mat4 mvp = vp * groundTransform.transform.getModelMatrix();
            m_shader->setMat4("uMVP", mvp);
            m_shader->setVec3("uColor", Nebula::Vec3{0.85f, 0.12f, 0.1f});
            getRenderer().drawIndexed(m_groundVao, m_groundIndexCount);
        }

        // Cube "character" — unit cube, blue
        {
            const Nebula::Mat4 mvp = vp * cubeTransform.transform.getModelMatrix();
            m_shader->setMat4("uMVP", mvp);
            m_shader->setVec3("uColor", Nebula::Vec3{0.15f, 0.45f, 0.95f});
            getRenderer().drawIndexed(m_groundVao, m_groundIndexCount);
        }

        m_shader->unbind();
        runScriptRenderPhase(0.0f);
    }

private:
    Nebula::ScriptContext makeScriptContext()
    {
        return Nebula::ScriptContext{getScene(), &getInput(), &getActionMapping()};
    }

    void rebuildRuntimeScripts()
    {
        shutdownRuntimeScripts();
        Nebula::Scene &scene = getScene();
        for (const Nebula::Entity entity : scene.getAllEntities())
        {
            if (!scene.hasComponent<Nebula::ScriptComponent>(entity))
            {
                continue;
            }

            const auto &scriptComponent = scene.getComponent<Nebula::ScriptComponent>(entity);
            Nebula::ScriptPtr script = m_scriptRegistry.createScript(scriptComponent.scriptName);
            if (!script)
            {
                continue;
            }
            m_runtimeScripts[entity.id] = std::move(script);
        }
    }

    void initializeRuntimeScripts()
    {
        Nebula::ScriptContext ctx = makeScriptContext();
        for (auto &[entityId, script] : m_runtimeScripts)
        {
            Nebula::Entity entity{entityId};
            script->onCreate(ctx, entity);
            script->onEnable(ctx, entity);
        }
    }

    void shutdownRuntimeScripts()
    {
        if (m_runtimeScripts.empty())
        {
            return;
        }

        Nebula::ScriptContext ctx = makeScriptContext();
        for (auto &[entityId, script] : m_runtimeScripts)
        {
            Nebula::Entity entity{entityId};
            script->onDisable(ctx, entity);
            script->onDestroy(ctx, entity);
        }
        m_runtimeScripts.clear();
    }

    void runScriptUpdatePhase(float dt)
    {
        if (m_runtimeScripts.empty())
        {
            return;
        }

        Nebula::ScriptContext ctx = makeScriptContext();
        for (auto &[entityId, script] : m_runtimeScripts)
        {
            Nebula::Entity entity{entityId};
            if (!ctx.scene.isValidEntity(entity))
            {
                continue;
            }
            script->onUpdate(ctx, entity, dt);
            script->onPhysicsUpdate(ctx, entity, dt);
        }
    }

    void runScriptRenderPhase(float dt)
    {
        if (m_runtimeScripts.empty())
        {
            return;
        }

        Nebula::ScriptContext ctx = makeScriptContext();
        for (auto &[entityId, script] : m_runtimeScripts)
        {
            Nebula::Entity entity{entityId};
            if (!ctx.scene.isValidEntity(entity))
            {
                continue;
            }
            script->onRender(ctx, entity, dt);
        }
    }

    void runInputPhase(float)
    {
        Nebula::Input &input = getInput();
        Nebula::ActionMapping &mapping = getActionMapping();

        if (mapping.wasActionPressed(Nebula::Action::Interact, input))
        {
            m_showInputDebug = !m_showInputDebug;
        }

        m_lookX = 0.0f;
        m_lookY = 0.0f;
        mapping.getAxisValue(Nebula::Axis::LookX, input, m_lookX, m_lookY);
        mapping.getAxisValue(Nebula::Axis::LookY, input, m_lookX, m_lookY);

        m_moveX = 0.0f;
        m_moveY = 0.0f;
        mapping.getAxisValue(Nebula::Axis::MoveX, input, m_moveX, m_moveY);
        mapping.getAxisValue(Nebula::Axis::MoveY, input, m_moveX, m_moveY);

        m_zoomX = 0.0f;
        m_zoomY = 0.0f;
        mapping.getAxisValue(Nebula::Axis::Scroll, input, m_zoomX, m_zoomY);
    }

    void runSimulationPhase(float dt)
    {
        Nebula::Scene &scene = getScene();
        auto &cubeTransform = scene.getComponent<Nebula::TransformComponent>(m_cubeEntity);
        auto &cameraComponent = scene.getComponent<Nebula::CameraComponent>(m_cameraEntity);
        Nebula::ActionMapping &mapping = getActionMapping();

        const float lookSensitivity = 0.0035f;
        const float turnX = m_lookX * lookSensitivity;
        const float turnY = m_lookY * lookSensitivity;
        cameraComponent.yaw -= turnX;
        cameraComponent.pitch = std::clamp(cameraComponent.pitch - turnY, -1.2f, 0.65f);

        Nebula::Vec3 moveDir{m_moveX, 0.0f, m_moveY};
        if (moveDir.x != 0.0f || moveDir.z != 0.0f)
        {
            const float len = std::sqrt(moveDir.x * moveDir.x + moveDir.z * moveDir.z);
            moveDir.x /= len;
            moveDir.z /= len;
        }

        const float yaw = cameraComponent.yaw;
        const Nebula::Vec3 forward{std::sin(yaw), 0.0f, std::cos(yaw)};
        const Nebula::Vec3 right{std::cos(yaw), 0.0f, -std::sin(yaw)};
        Nebula::Vec3 velocity{
            forward.x * moveDir.z + right.x * moveDir.x,
            forward.y * moveDir.z + right.y * moveDir.x,
            forward.z * moveDir.z + right.z * moveDir.x};
        velocity = velocity * (3.5f * dt);
        Nebula::Vec3 cubePos = cubeTransform.transform.getPosition();
        cubePos += velocity;
        cubeTransform.transform.setPosition(cubePos);

        float dist = cameraComponent.distance;
        dist -= m_zoomY * 0.6f;
        cameraComponent.distance = std::clamp(dist, 1.5f, 24.0f);

        if (m_showInputDebug)
        {
            m_debugPrintTimer += dt;
            if (m_debugPrintTimer >= 0.25f)
            {
                m_debugPrintTimer = 0.0f;
                float sensX = 0.0f;
                float sensY = 0.0f;
                float zoomSens = 0.0f;
                bool invertY = false;
                mapping.getCameraSensitivity(sensX, sensY, zoomSens, invertY);
                std::cout
                    << "[Mapping] MoveX=" << m_moveX
                    << " MoveY=" << m_moveY
                    << " LookX=" << m_lookX
                    << " LookY=" << m_lookY
                    << " Scroll=" << m_zoomY
                    << " Sens=(" << sensX << ", " << sensY << ")"
                    << " ZoomSens=" << zoomSens
                    << " InvertY=" << invertY
                    << '\n';
            }
        }
    }

    void buildDefaultScene()
    {
        Nebula::Scene &scene = getScene();

        m_groundEntity = scene.createEntity();
        auto &groundTransform = scene.addComponent<Nebula::TransformComponent>(m_groundEntity);
        groundTransform.transform.setPosition(Nebula::Vec3{0.0f, 0.0f, 0.0f});
        groundTransform.transform.setYaw(0.0f);
        groundTransform.transform.setScale(1.0f);
        scene.addComponent<Nebula::MeshRendererComponent>(m_groundEntity);
        scene.addComponent<Nebula::ScriptComponent>(m_groundEntity).scriptName = "Ground";

        m_cubeEntity = scene.createEntity();
        auto &cubeTransform = scene.addComponent<Nebula::TransformComponent>(m_cubeEntity);
        cubeTransform.transform.setPosition(Nebula::Vec3{0.0f, 0.5f, 0.0f});
        cubeTransform.transform.setYaw(0.0f);
        cubeTransform.transform.setScale(1.0f);
        scene.addComponent<Nebula::MeshRendererComponent>(m_cubeEntity);
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
        const std::filesystem::path scenePath(m_scenePath);
        if (!scenePath.parent_path().empty())
        {
            std::error_code ec;
            std::filesystem::create_directories(scenePath.parent_path(), ec);
            if (ec)
            {
                return false;
            }
        }
        return getScene().saveToFile(m_scenePath);
    }

    static Nebula::VertexBufferLayout layoutPositionOnly()
    {
        Nebula::VertexBufferLayout layout{};
        layout.strideBytes = sizeof(Vertex3);
        Nebula::VertexBufferElement pos{};
        pos.location = 0;
        pos.componentCount = 3;
        pos.type = Nebula::VertexAttributeType::Float;
        pos.offsetBytes = 0;
        pos.normalized = false;
        layout.elements.push_back(pos);
        return layout;
    }

    bool ensureGpuResources()
    {
        // Unit cube [-0.5, 0.5] — 8 verts, 12 triangles
        const std::vector<Vertex3> cubeVerts = {
            {-0.5f, -0.5f, 0.5f},
            {0.5f, -0.5f, 0.5f},
            {0.5f, 0.5f, 0.5f},
            {-0.5f, 0.5f, 0.5f},
            {-0.5f, -0.5f, -0.5f},
            {0.5f, -0.5f, -0.5f},
            {0.5f, 0.5f, -0.5f},
            {-0.5f, 0.5f, -0.5f},
        };
        std::vector<uint32_t> cubeIndices = {
            0, 1, 2, 0, 2, 3, // +Z
            1, 5, 6, 1, 6, 2, // +X
            5, 4, 7, 5, 7, 6, // -Z
            4, 0, 3, 4, 3, 7, // -X
            3, 2, 6, 3, 6, 7, // +Y
            4, 5, 1, 4, 1, 0  // -Y
        };
        m_cubeIndexCount = static_cast<uint32_t>(cubeIndices.size());

        const Nebula::VertexBufferLayout layout = layoutPositionOnly();

        m_cubeVbo = Nebula::VertexBuffer::create(
            cubeVerts.data(),
            cubeVerts.size() * sizeof(Vertex3),
            Nebula::BufferUsage::Static,
            layout);
        if (!m_cubeVbo)
        {
            std::cerr << "Cube VertexBuffer::create failed.\n";
            return false;
        }
        m_cubeVao = Nebula::VertexArray::create();
        if (!m_cubeVao)
        {
            std::cerr << "Cube VertexArray::create failed.\n";
            return false;
        }
        m_cubeVao->addVertexBuffer(m_cubeVbo);
        m_cubeIbo = Nebula::IndexBuffer::create(cubeIndices.data(), m_cubeIndexCount);
        if (!m_cubeIbo)
        {
            std::cerr << "Cube IndexBuffer::create failed.\n";
            return false;
        }
        m_cubeVao->setIndexBuffer(m_cubeIbo);

        // Ground quad on XZ at y = 0
        const std::vector<Vertex3> groundVerts = {
            {-12.0f, 0.0f, -12.0f},
            {12.0f, 0.0f, -12.0f},
            {12.0f, 0.0f, 12.0f},
            {-12.0f, 0.0f, 12.0f},
        };
        std::vector<uint32_t> groundIndices = {0, 1, 2, 0, 2, 3};
        m_groundIndexCount = static_cast<uint32_t>(groundIndices.size());

        m_groundVbo = Nebula::VertexBuffer::create(
            groundVerts.data(),
            groundVerts.size() * sizeof(Vertex3),
            Nebula::BufferUsage::Static,
            layout);
        if (!m_groundVbo)
        {
            std::cerr << "Ground VertexBuffer::create failed.\n";
            return false;
        }
        m_groundVao = Nebula::VertexArray::create();
        if (!m_groundVao)
        {
            std::cerr << "Ground VertexArray::create failed.\n";
            return false;
        }
        m_groundVao->addVertexBuffer(m_groundVbo);
        m_groundIbo = Nebula::IndexBuffer::create(groundIndices.data(), m_groundIndexCount);
        if (!m_groundIbo)
        {
            std::cerr << "Ground IndexBuffer::create failed.\n";
            return false;
        }
        m_groundVao->setIndexBuffer(m_groundIbo);

        m_shader = Nebula::Shader::create(kVert3D, kFragSolidColor);
        if (!m_shader)
        {
            std::cerr << "Shader::create failed (check OpenGL / GLSL version vs #version line).\n";
            return false;
        }

        return true;
    }

    bool m_gpuReady = false;

    uint32_t m_cubeIndexCount = 0;
    uint32_t m_groundIndexCount = 0;
    std::shared_ptr<Nebula::VertexBuffer> m_cubeVbo;
    std::shared_ptr<Nebula::VertexArray> m_cubeVao;
    std::shared_ptr<Nebula::IndexBuffer> m_cubeIbo;
    std::shared_ptr<Nebula::VertexBuffer> m_groundVbo;
    std::shared_ptr<Nebula::VertexArray> m_groundVao;
    std::shared_ptr<Nebula::IndexBuffer> m_groundIbo;
    std::shared_ptr<Nebula::Shader> m_shader;

    Nebula::Entity m_groundEntity{};
    Nebula::Entity m_cubeEntity{};
    Nebula::Entity m_cameraEntity{};
    Nebula::ScriptRegistry m_scriptRegistry;
    std::unordered_map<Nebula::EntityID, Nebula::ScriptPtr> m_runtimeScripts;
    std::string m_scenePath = "assets/scenes/week2_scene.json";
    bool m_showInputDebug = false;
    float m_debugPrintTimer = 0.0f;
    float m_moveX = 0.0f;
    float m_moveY = 0.0f;
    float m_lookX = 0.0f;
    float m_lookY = 0.0f;
    float m_zoomX = 0.0f;
    float m_zoomY = 0.0f;
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
