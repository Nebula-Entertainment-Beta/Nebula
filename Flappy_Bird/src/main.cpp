#include <iostream>
#include <vector>

#include <glm/glm.hpp>

#include "Window.h"
#include "renderer.h"
#include "vertex_buffer.h"
#include "vertex_array.h"
#include "index_buffer.h"
#include "shader.h"

namespace {

// Match your GLAD / driver. Your vendor glad is built for GL 4.1; "core" needs explicit context hints on some setups.
static const std::string kVert = R"(
#version 410 core
layout (location = 0) in vec2 aPos;
void main() {
    gl_Position = vec4(aPos, 0.0, 1.0);
}
)";

static const std::string kFrag = R"(
#version 410 core
out vec4 FragColor;
void main() {
    FragColor = vec4(1.0, 0.5, 0.2, 1.0);
}
)";

struct Vertex2D {
    float x, y;
};

} // namespace

int main()
{
    Nebula::Window window("Nebula", 800, 600);
    if (!window.isValid()) {
        std::cerr << "Window or OpenGL context failed to initialize. Exiting.\n";
        return 1;
    }

    Nebula::Renderer::init();

    // --- Triangle in normalized device coordinates (-1 .. 1) ---
    std::vector<Vertex2D> verts = {
        { -0.5f, -0.5f },
        { 0.5f, -0.5f },
        { 0.0f, 0.5f },
    };
    std::vector<uint32_t> indices = { 0, 1, 2 };

    Nebula::VertexBufferLayout layout{};
    layout.strideBytes = sizeof(Vertex2D);
    Nebula::VertexBufferElement pos{};
    pos.location = 0;
    pos.componentCount = 2;
    pos.type = Nebula::VertexAttributeType::Float;
    pos.offsetBytes = 0;
    pos.normalized = false;
    layout.elements.push_back(pos);

    auto vbo = Nebula::VertexBuffer::create(
        verts.data(),
        verts.size() * sizeof(Vertex2D),
        Nebula::BufferUsage::Static,
        layout);
    if (!vbo) {
        std::cerr << "VertexBuffer::create failed.\n";
        Nebula::Renderer::Shutdown();
        return 1;
    }

    auto vao = Nebula::VertexArray::create();
    if (!vao) {
        std::cerr << "VertexArray::create failed.\n";
        Nebula::Renderer::Shutdown();
        return 1;
    }
    vao->addVertexBuffer(vbo);

    auto ibo = Nebula::IndexBuffer::create(indices.data(), static_cast<uint32_t>(indices.size()));
    if (!ibo) {
        std::cerr << "IndexBuffer::create failed.\n";
        Nebula::Renderer::Shutdown();
        return 1;
    }
    vao->setIndexBuffer(ibo);

    auto shader = Nebula::Shader::create(kVert, kFrag);
    if (!shader) {
        std::cerr << "Shader::create failed (check OpenGL / GLSL version vs #version line).\n";
        Nebula::Renderer::Shutdown();
        return 1;
    }

    while (!window.shouldWindowClose()) {
        Nebula::Renderer::setViewport(0, 0, 800, 600);
        Nebula::Renderer::clear(glm::vec4(0.1f, 0.1f, 0.15f, 1.0f));

        shader->bind();
        Nebula::Renderer::drawIndexed(vao, static_cast<uint32_t>(indices.size()));
        shader->unbind();

        window.update();
    }

    Nebula::Renderer::Shutdown();
    return 0;
}
