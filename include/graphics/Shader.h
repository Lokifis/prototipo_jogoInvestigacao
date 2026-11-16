#pragma once

#include "../glad/glad.h"
#include <glm/glm.hpp>
#include <string>

// Carrega, compila e linka um programa de shader (vertex + fragment) a partir de arquivos.
// Lembre de chamar use() antes de usar os setters.
class Shader {
public:
    Shader(const std::string& vertexPath, const std::string& fragmentPath);
    ~Shader();

    Shader(const Shader&) = delete;
    Shader& operator=(const Shader&) = delete;

    bool isValid() const { return m_id != 0; }
    void use() const;

    void setInt(const std::string& name, int value) const;
    void setFloat(const std::string& name, float value) const;
    void setVec2(const std::string& name, const glm::vec2& value) const;
    void setVec3(const std::string& name, const glm::vec3& value) const;
    void setMat4(const std::string& name, const glm::mat4& value) const;

private:
    unsigned int m_id = 0;

    static std::string  readFile(const std::string& path);
    static unsigned int compile(GLenum type, const std::string& source, const std::string& label);
    GLint location(const std::string& name) const;
};
