/**
 * @file shader.h
 * @brief GPU **shader program**: vertex stage + fragment stage compiled from source strings.
 *
 * **Beginner angle:** Shaders are small programs that run on the GPU. The **vertex** shader runs per
 * vertex (positions, pass data along); the **fragment** shader runs per pixel (outputs color).
 * `bind()` installs the program before drawing.
 */
#pragma once

#include <memory>
#include <string>

namespace Nebula {

    class Shader
    {
    public:
        virtual ~Shader() = default;
        virtual void bind() const = 0;
        virtual void unbind() const = 0;

        /**
         * @brief Compiles/links GLSL for the active backend.
         * @return nullptr if compilation or linking failed (errors go to the console).
         */
        static std::shared_ptr<Shader> create(const std::string& vertexSrc,
                                              const std::string& fragmentSrc);
    };

}