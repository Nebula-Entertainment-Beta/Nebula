/**
 * @file openGL_Shader.h
 * @brief OpenGL shader program (`glCreateProgram`, `glUseProgram`).
 */
#pragma once

#include <cstdint>
#include <string>
#include <memory>

#include "../interface/shader.h"

namespace Nebula {

    class OpenGL_Shader : public Shader
    {
    public:
        OpenGL_Shader(const std::string& vertexSrc, const std::string& fragmentSrc);
        ~OpenGL_Shader() override;
        void bind() const override;
        void unbind() const override;

        /** @brief Returns nullptr if compile/link failed (logs errors to stderr). */
        static std::shared_ptr<Shader> create(const std::string& vertexSrc,
                                              const std::string& fragmentSrc);

    private:
        uint32_t m_rendererID = 0;
    };
}