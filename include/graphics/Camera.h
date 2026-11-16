#pragma once

#include <glm/glm.hpp>

// Raio no espaço do mundo (usado para clicar em objetos da cena)
struct Ray {
    glm::vec3 origin;
    glm::vec3 direction;  // normalizado
};

// Câmera orbital: gira em torno de um ponto (target), com zoom.
// Boa para investigar um cômodo olhando de fora. Para primeira pessoa,
// troque por uma câmera com posição + yaw/pitch (TODO).
class Camera {
public:
    Camera(const glm::vec3& target = glm::vec3(0.0f, 1.0f, 0.0f),
           float distance = 9.0f,
           float yawDeg = 40.0f,
           float pitchDeg = 28.0f);

    void setAspect(float aspect) { m_aspect = aspect; }

    void rotate(float deltaYawDeg, float deltaPitchDeg);
    void zoom(float delta);  // delta positivo aproxima

    glm::vec3 position() const;
    glm::mat4 view() const;
    glm::mat4 projection() const;

    // (x, y) em pixels com origem no canto superior esquerdo; (width, height) é o
    // tamanho da imagem renderizada.
    Ray screenPointToRay(double x, double y, int width, int height) const;

private:
    glm::vec3 m_target;
    float m_distance;
    float m_yaw;    // graus, em torno do eixo Y
    float m_pitch;  // graus, limitado entre 5 e 85
    float m_fov = 45.0f;
    float m_aspect = 16.0f / 9.0f;
};
