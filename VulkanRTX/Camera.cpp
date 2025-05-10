#include "Camera.hpp"

Camera::Camera() : m_fov(45.0f), m_aspectRatio(16.0f / 9.0f), m_nearPlane(0.1f), m_farPlane(100.0f) 
{
}

void Camera::setPerspective(float fovDegrees, float aspectRatio, float nearPlane, float farPlane) 
{
    m_fov = fovDegrees;
    m_aspectRatio = aspectRatio;
    m_nearPlane = nearPlane;
    m_farPlane = farPlane;
}

glm::mat4 Camera::getProjectionMatrix() const 
{
    glm::mat4 proj = glm::perspective(glm::radians(m_fov), m_aspectRatio, m_nearPlane, m_farPlane);
    proj[1][1] *= -1;
    return proj;
}

float Camera::getFOV() const { return m_fov; }
float Camera::getAspectRatio() const { return m_aspectRatio; }
float Camera::getNearPlane() const { return m_nearPlane; }
float Camera::getFarPlane() const { return m_farPlane; }
