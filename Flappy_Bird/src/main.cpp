#include <iostream>
#include <vector>
#include <cstdint>

#include <glm/glm.hpp>

#include "Window.h"
#include "renderer.h"
#include "vertex_buffer.h"
#include "vertex_array.h"
#include "index_buffer.h"
#include "shader.h"
#include "texture.h"

namespace {

// Match your GLAD / driver. Your vendor glad is built for GL 4.1; "core" needs explicit context hints on some setups.
static const std::string kVert = R"(
#version 410 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aUV;
out vec2 vUV;
void main() {
    gl_Position = vec4(aPos, 0.0, 1.0);
    vUV = aUV;
}
)";

static const std::string kFrag = R"(
#version 410 core
in vec2 vUV;
uniform sampler2D uTexture;
out vec4 FragColor;
void main() {
    FragColor = texture(uTexture, vUV);
}
)";

struct Vertex2D {
    float x, y, u, v;
    
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
        { -0.5f, -0.5f, 0.0f, 0.0f },
        { 0.5f, -0.5f, 1.0f, 0.0f },
        { 0.0f, 0.5f, 0.5f, 1.0f },
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
    //add another vertexbuffer element for the uv coordinates if needed
    Nebula::VertexBufferElement uv{};
    uv.location = 1;
    uv.componentCount = 2;
    uv.type = Nebula::VertexAttributeType::Float;
    uv.offsetBytes = offsetof(Vertex2D, u);
    uv.normalized = false;
    layout.elements.push_back(uv);

    
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

    auto texture = Nebula::Texture::createFromFile("Flappy_Bird/Assets/spider1.png");
    if (!texture) {
        constexpr int texSize = 256;
        std::vector<uint8_t> pixels(texSize * texSize * 4, 255);
        for (int y = 0; y < texSize; ++y) {
            for (int x = 0; x < texSize; ++x) {
                const bool dark = ((x / 32) + (y / 32)) % 2 == 0;
                const uint8_t c = dark ? 40 : 220;
                const size_t i = static_cast<size_t>((y * texSize + x) * 4);
                pixels[i + 0] = c;
                pixels[i + 1] = c;
                pixels[i + 2] = c;
                pixels[i + 3] = 255;
            }
        }
        texture = Nebula::Texture::create(texSize, texSize, pixels.data());
    }
    if (!texture) {
        std::cerr << "Texture::create failed.\n";
        Nebula::Renderer::Shutdown();
        return 1;
    }


    while (!window.shouldWindowClose()) {
        Nebula::Renderer::setViewport(0, 0, 800, 600);
        Nebula::Renderer::clear(glm::vec4(0.1f, 0.1f, 0.15f, 1.0f));

        shader->bind();
        texture->bind(0); // Bind the texture to texture unit 0
        shader->setInt("uTexture", 0); // Tell the sampler to read from texture unit 0
        Nebula::Renderer::drawIndexed(vao, static_cast<uint32_t>(indices.size()));
        shader->unbind();

        window.update();
    }

    Nebula::Renderer::Shutdown();
    return 0;
}
