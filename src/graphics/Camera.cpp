#include "../../include/graphics/Camera.h"

#include <algorithm>
#include <cmath>
#include <glm/gtc/matrix_transform.hpp>

Camera::Camera(const glm::vec3& target, float distance, float yawDeg, float pitchDeg)
    : m_target(target), m_distance(distance), m_yaw(yawDeg), m_pitch(pitchDeg) {}

void Camera::rotate(float deltaYawDeg, float deltaPitchDeg) {
    m_yaw += deltaYawDeg;
    m_pitch = std::clamp(m_pitch + deltaPitchDeg, 5.0f, 85.0f);
}

void Camera::zoom(float delta) {
    m_distance = std::clamp(m_distance - delta, 3.0f, 15.0f);
}

glm::vec3 Camera::position() const {
    const float yaw   = glm::radians(m_yaw);
    const float pitch = glm::radians(m_pitch);
    return m_target + m_distance * glm::vec3(std::cos(pitch) * std::sin(yaw),
                                             std::sin(pitch),
                                             std::cos(pitch) * std::cos(yaw));
}

glm::mat4 Camera::view() const {
    return glm::lookAt(position(), m_target, glm::vec3(0.0f, 1.0f, 0.0f));
}

glm::mat4 Camera::projection() const {
    return glm::perspective(glm::radians(m_fov), m_aspect, 0.1f, 100.0f);
}

Ray Camera::screenPointToRay(double x, double y, int width, int height) const {
    const float ndcX = static_cast<float>(2.0 * x / width - 1.0);
    const float ndcY = static_cast<float>(1.0 - 2.0 * y / height);

    const glm::mat4 inv = glm::inverse(projection() * view());
    glm::vec4 farPoint = inv * glm::vec4(ndcX, ndcY, 1.0f, 1.0f);
    farPoint /= farPoint.w;

    const glm::vec3 origin = position();
    return Ray{origin, glm::normalize(glm::vec3(farPoint) - origin)};
}
