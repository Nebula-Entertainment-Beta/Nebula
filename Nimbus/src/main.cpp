#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cmath>
#include <iostream>
#include <vector>

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include "application.h"
#include "camera3D.h"
#include "index_buffer.h"
#include "renderer.h"
#include "shader.h"
#include "transform3D.h"
#include "vertex_array.h"
#include "vertex_buffer.h"

namespace {

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

struct Vertex3 {
    float x, y, z;
};

} // namespace

class NimbusApp final : public Nebula::Application {
public:
    explicit NimbusApp(const Nebula::ApplicationSpec& spec)
        : Nebula::Application(spec)
    {
        m_groundTransform.setPosition(glm::vec3(0.0f));
        m_groundTransform.setYaw(0.0f);
        m_groundTransform.setScale(1.0f);

        m_cubeTransform.setPosition(glm::vec3(0.0f, 0.5f, 0.0f));
        m_cubeTransform.setYaw(0.0f);
        m_cubeTransform.setScale(1.0f);

        m_camera.setTarget(m_cubeTransform.getPosition());
        m_camera.setPivotOffset(glm::vec3(0.0f, 0.35f, 0.0f));
        m_camera.setDistance(6.0f);
        m_camera.setYaw(0.7f);
        m_camera.setPitch(-0.3f);
        m_camera.setFOV(55.0f);
        m_camera.setNearPlane(0.1f);
        m_camera.setFarPlane(100.0f);

        GLFWwindow* window = getWindow().getGLFWwindow();
        if (window) {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            if (glfwRawMouseMotionSupported()) {
                glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
            }
        }
    }

protected:
    void onUpdate(float dt) override
    {
        Nebula::Input& input = getInput();

        const float lookSensitivity = 0.0035f;
        const float turnX = input.mouseDeltaX() * lookSensitivity;
        const float turnY = input.mouseDeltaY() * lookSensitivity;
        m_camera.setYaw(m_camera.getYaw() - turnX);
        m_camera.setPitch(std::clamp(m_camera.getPitch() - turnY, -1.2f, 0.65f));

        glm::vec3 moveDir(0.0f);
        if (input.isKeyDown(GLFW_KEY_W)) {
            moveDir.z += 1.0f;
        }
        if (input.isKeyDown(GLFW_KEY_S)) {
            moveDir.z -= 1.0f;
        }
        if (input.isKeyDown(GLFW_KEY_A)) {
            moveDir.x -= 1.0f;
        }
        if (input.isKeyDown(GLFW_KEY_D)) {
            moveDir.x += 1.0f;
        }

        if (moveDir.x != 0.0f || moveDir.z != 0.0f) {
            const float len = std::sqrt(moveDir.x * moveDir.x + moveDir.z * moveDir.z);
            moveDir /= len;
        }

        const float yaw = m_camera.getYaw();
        const glm::vec3 forward(std::sin(yaw), 0.0f, std::cos(yaw));
        const glm::vec3 right(std::cos(yaw), 0.0f, -std::sin(yaw));
        const glm::vec3 velocity = (forward * moveDir.z + right * moveDir.x) * (3.5f * dt);
        glm::vec3 cubePos = m_cubeTransform.getPosition();
        cubePos += velocity;
        m_cubeTransform.setPosition(cubePos);

        float dist = m_camera.getDistance();
        const float zoomSpeed = 6.0f * dt;
        if (input.isKeyDown(GLFW_KEY_Q)) {
            dist += zoomSpeed;
        }
        if (input.isKeyDown(GLFW_KEY_E)) {
            dist -= zoomSpeed;
        }
        dist -= input.mouseScrollDeltaY() * 0.6f;
        m_camera.setDistance(std::clamp(dist, 1.5f, 24.0f));
    }

    void onRender() override
    {
        if (!m_gpuReady) {
            if (!ensureGpuResources()) {
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

        m_camera.setTarget(m_cubeTransform.getPosition());
        m_camera.setAspectRatio(aspect);
        const glm::mat4 vp = m_camera.getViewProjectionMatrix();

        m_shader->bind();

        // Ground: large XZ plane at y = 0 (red)
        {
            const glm::mat4 mvp = vp * m_groundTransform.getModelMatrix();
            m_shader->setMat4("uMVP", mvp);
            m_shader->setVec3("uColor", glm::vec3(0.85f, 0.12f, 0.1f));
            Nebula::Renderer::drawIndexed(m_groundVao, m_groundIndexCount);
        }

        // Cube "character" — unit cube, blue
        {
            const glm::mat4 mvp = vp * m_cubeTransform.getModelMatrix();
            m_shader->setMat4("uMVP", mvp);
            m_shader->setVec3("uColor", glm::vec3(0.15f, 0.45f, 0.95f));
            Nebula::Renderer::drawIndexed(m_cubeVao, m_cubeIndexCount);
        }

        m_shader->unbind();
    }

private:
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
            { -0.5f, -0.5f, 0.5f },
            { 0.5f, -0.5f, 0.5f },
            { 0.5f, 0.5f, 0.5f },
            { -0.5f, 0.5f, 0.5f },
            { -0.5f, -0.5f, -0.5f },
            { 0.5f, -0.5f, -0.5f },
            { 0.5f, 0.5f, -0.5f },
            { -0.5f, 0.5f, -0.5f },
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
        if (!m_cubeVbo) {
            std::cerr << "Cube VertexBuffer::create failed.\n";
            return false;
        }
        m_cubeVao = Nebula::VertexArray::create();
        if (!m_cubeVao) {
            std::cerr << "Cube VertexArray::create failed.\n";
            return false;
        }
        m_cubeVao->addVertexBuffer(m_cubeVbo);
        m_cubeIbo = Nebula::IndexBuffer::create(cubeIndices.data(), m_cubeIndexCount);
        if (!m_cubeIbo) {
            std::cerr << "Cube IndexBuffer::create failed.\n";
            return false;
        }
        m_cubeVao->setIndexBuffer(m_cubeIbo);

        // Ground quad on XZ at y = 0
        const std::vector<Vertex3> groundVerts = {
            { -12.0f, 0.0f, -12.0f },
            { 12.0f, 0.0f, -12.0f },
            { 12.0f, 0.0f, 12.0f },
            { -12.0f, 0.0f, 12.0f },
        };
        std::vector<uint32_t> groundIndices = { 0, 1, 2, 0, 2, 3 };
        m_groundIndexCount = static_cast<uint32_t>(groundIndices.size());

        m_groundVbo = Nebula::VertexBuffer::create(
            groundVerts.data(),
            groundVerts.size() * sizeof(Vertex3),
            Nebula::BufferUsage::Static,
            layout);
        if (!m_groundVbo) {
            std::cerr << "Ground VertexBuffer::create failed.\n";
            return false;
        }
        m_groundVao = Nebula::VertexArray::create();
        if (!m_groundVao) {
            std::cerr << "Ground VertexArray::create failed.\n";
            return false;
        }
        m_groundVao->addVertexBuffer(m_groundVbo);
        m_groundIbo = Nebula::IndexBuffer::create(groundIndices.data(), m_groundIndexCount);
        if (!m_groundIbo) {
            std::cerr << "Ground IndexBuffer::create failed.\n";
            return false;
        }
        m_groundVao->setIndexBuffer(m_groundIbo);

        m_shader = Nebula::Shader::create(kVert3D, kFragSolidColor);
        if (!m_shader) {
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

    Nebula::Transform3D m_groundTransform;
    Nebula::Transform3D m_cubeTransform;
    Nebula::Camera3D m_camera;
};

int main()
{
    Nebula::ApplicationSpec spec;
    spec.title = "Nimbus";
    spec.width = 800;
    spec.height = 600;

    NimbusApp app(spec);
    if (!app.getWindow().isValid()) {
        std::cerr << "Window or OpenGL context failed to initialize. Exiting.\n";
        return 1;
    }

    app.run();
    return 0;
}
