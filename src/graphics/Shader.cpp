#include "../../include/graphics/Shader.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <glm/gtc/type_ptr.hpp>

std::string Shader::readFile(const std::string& path) {
    std::ifstream file(path, std::ios::in | std::ios::binary);
    if (!file) {
        std::cerr << "[Shader] Não foi possível abrir: " << path << std::endl;
        return "";
    }
    std::stringstream ss;
    ss << file.rdbuf();
    return ss.str();
}

unsigned int Shader::compile(GLenum type, const std::string& source, const std::string& label) {
    unsigned int shader = glCreateShader(type);
    const char* src = source.c_str();
    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);

    GLint ok = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        char log[1024];
        glGetShaderInfoLog(shader, sizeof(log), nullptr, log);
        std::cerr << "[Shader] Erro ao compilar " << label << ":\n" << log << std::endl;
        glDeleteShader(shader);
        return 0;
    }
    return shader;
}

Shader::Shader(const std::string& vertexPath, const std::string& fragmentPath) {
    const std::string vertexSrc   = readFile(vertexPath);
    const std::string fragmentSrc = readFile(fragmentPath);
    if (vertexSrc.empty() || fragmentSrc.empty()) return;

    unsigned int vert = compile(GL_VERTEX_SHADER, vertexSrc, vertexPath);
    unsigned int frag = compile(GL_FRAGMENT_SHADER, fragmentSrc, fragmentPath);
    if (vert == 0 || frag == 0) {
        glDeleteShader(vert);  // glDeleteShader(0) é ignorado silenciosamente
        glDeleteShader(frag);
        return;
    }

    m_id = glCreateProgram();
    glAttachShader(m_id, vert);
    glAttachShader(m_id, frag);
    glLinkProgram(m_id);

    GLint ok = 0;
    glGetProgramiv(m_id, GL_LINK_STATUS, &ok);
    if (!ok) {
        char log[1024];
        glGetProgramInfoLog(m_id, sizeof(log), nullptr, log);
        std::cerr << "[Shader] Erro ao linkar (" << vertexPath << ", " << fragmentPath << "):\n"
                  << log << std::endl;
        glDeleteProgram(m_id);
        m_id = 0;
    }

    glDeleteShader(vert);
    glDeleteShader(frag);
}

Shader::~Shader() {
    if (m_id != 0) glDeleteProgram(m_id);
}

void Shader::use() const { glUseProgram(m_id); }

GLint Shader::location(const std::string& name) const {
    return glGetUniformLocation(m_id, name.c_str());
}

void Shader::setInt(const std::string& name, int value) const {
    glUniform1i(location(name), value);
}

void Shader::setFloat(const std::string& name, float value) const {
    glUniform1f(location(name), value);
}

void Shader::setVec2(const std::string& name, const glm::vec2& value) const {
    glUniform2fv(location(name), 1, glm::value_ptr(value));
}

void Shader::setVec3(const std::string& name, const glm::vec3& value) const {
    glUniform3fv(location(name), 1, glm::value_ptr(value));
}

void Shader::setMat4(const std::string& name, const glm::mat4& value) const {
    glUniformMatrix4fv(location(name), 1, GL_FALSE, glm::value_ptr(value));
}
