/**
 * @file openGL_Shader.h
 * @brief OpenGL shader program (`glCreateProgram`, `glUseProgram`).
 */
#pragma once

#include <cstdint>
#include <memory>
#include <string>

#include "shader.h"

namespace Nebula
{

    class OpenGL_Shader : public Shader
    {
    public:
        OpenGL_Shader(const std::string &vertexSrc, const std::string &fragmentSrc);
        ~OpenGL_Shader() override;
        void bind() const override;
        void unbind() const override;
        void setInt(const std::string &name, int value) const override;
        void setMat4(const std::string &name, const Mat4 &matrix) const override;
        void setVec3(const std::string &name, const Vec3 &value) const override;
        void setFloat(const std::string &name, float value) const override;

        /** @brief Returns nullptr if compile/link failed (logs errors to stderr). */
        static std::shared_ptr<Shader> create(const std::string &vertexSrc,
                                              const std::string &fragmentSrc);

    private:
        uint32_t m_rendererID = 0;
    };
}
