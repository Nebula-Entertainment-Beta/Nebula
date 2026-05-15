/**
 * @file openGL_Shader.cpp
 * @brief Compiles GLSL sources, links a program, and validates success before returning a `Shader`.
 */
#include "openGL_Shader.h"
#include <glad/glad.h>
#include <iostream>
#include <vector>

namespace Nebula{

  static bool compileShader(GLuint shader, const std::string& source, const char* stageName) {
    const char* src = source.c_str();
    const GLint len = static_cast<GLint>(source.size());
    glShaderSource(shader, 1, &src, &len);
    glCompileShader(shader);
    GLint ok = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
    if (ok == GL_TRUE)
        return true;
    GLint logLength = 0;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength);
    std::vector<char> log(static_cast<size_t>(logLength));
    if (logLength > 0)
        glGetShaderInfoLog(shader, logLength, nullptr, log.data());
    std::cerr << "Shader compile error (" << stageName << "):\n"
              << (log.empty() ? "" : log.data()) << std::endl;
    return false;
}
  static bool linkProgram(GLuint program) {
      glLinkProgram(program);
      GLint ok = 0;
      glGetProgramiv(program, GL_LINK_STATUS, &ok);
      if (ok == GL_TRUE)
          return true;
      GLint logLength = 0;
      glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLength);
      std::vector<char> log(static_cast<size_t>(logLength));
      if (logLength > 0)
          glGetProgramInfoLog(program, logLength, nullptr, log.data());
      std::cerr << "Program link error:\n"
                << (log.empty() ? "" : log.data()) << std::endl;
      return false;
  }
  OpenGL_Shader::OpenGL_Shader(const std::string& vertexSource, const std::string& fragmentSource) {
      const GLuint vs = glCreateShader(GL_VERTEX_SHADER);
      const GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
      if (!compileShader(vs, vertexSource, "vertex") || !compileShader(fs, fragmentSource, "fragment")) {
          glDeleteShader(vs);
          glDeleteShader(fs);
          m_rendererID = 0;
          return;
      }
      const GLuint program = glCreateProgram();
      glAttachShader(program, vs);
      glAttachShader(program, fs);
      const bool linked = linkProgram(program);
      glDetachShader(program, vs);
      glDetachShader(program, fs);
      glDeleteShader(vs);
      glDeleteShader(fs);
      if (!linked) {
          glDeleteProgram(program);
          m_rendererID = 0;
          return;
      }
      m_rendererID = program;
  }
  OpenGL_Shader::~OpenGL_Shader() {
      if (m_rendererID != 0)
          glDeleteProgram(m_rendererID);
  }
  void OpenGL_Shader::bind() const {
      glUseProgram(m_rendererID);
  }
  void OpenGL_Shader::unbind() const {
      glUseProgram(0);
  }

  void OpenGL_Shader::setInt(const std::string& name, int value) const {
      const GLint loc = glGetUniformLocation(m_rendererID, name.c_str());
      if (loc >= 0)
          glUniform1i(loc, value);
  }

  void OpenGL_Shader::setMat4(const std::string& name, const Mat4& matrix) const {
      const GLint loc = glGetUniformLocation(m_rendererID, name.c_str());
      if (loc >= 0)
          glUniformMatrix4fv(loc, 1, GL_FALSE, matrix.data());
  }

  void OpenGL_Shader::setVec3(const std::string& name, const Vec3& value) const {
      const GLint loc = glGetUniformLocation(m_rendererID, name.c_str());
      if (loc >= 0)
          glUniform3f(loc, value.x, value.y, value.z);
  }

  // Static member of OpenGL_Shader — may access private m_rendererID on `impl`.
  std::shared_ptr<Shader> OpenGL_Shader::create(
      const std::string& vertexSrc,
      const std::string& fragmentSrc) {
    auto impl = std::make_shared<OpenGL_Shader>(vertexSrc, fragmentSrc);
    if (impl->m_rendererID == 0)
      return nullptr;
    return impl;
  }

}
