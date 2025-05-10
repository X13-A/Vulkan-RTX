#include "Transform.hpp"

Transform::Transform() : m_position(0.0f), m_rotation(glm::quat_identity<float, glm::defaultp>()), m_scale(1.0f) 
{

}

void Transform::setPosition(const glm::vec3& position) 
{
    m_position = position;
}

void Transform::translate(const glm::vec3& delta) 
{
    m_position += m_rotation * delta;
}

glm::vec3 Transform::getPosition() const 
{
    return m_position;
}

void Transform::setRotation(const glm::vec3& eulerDegrees) 
{
    glm::vec3 radians = glm::radians(eulerDegrees);
    m_rotation = glm::quat(radians);
}

void Transform::rotate(const glm::vec3& deltaDegrees) 
{
    glm::vec3 radians = glm::radians(deltaDegrees);
    m_rotation = glm::normalize(glm::quat(radians) * m_rotation);
}

glm::vec3 Transform::getRotationEuler() const 
{
    return glm::degrees(glm::eulerAngles(m_rotation));
}

glm::quat Transform::getRotationQuat() const 
{
    return m_rotation;
}

void Transform::setScale(const glm::vec3& scale) 
{
    m_scale = scale;
}

void Transform::scale(const glm::vec3& factor) 
{
    m_scale *= factor;
}

glm::vec3 Transform::getScale() const 
{
    return m_scale;
}

glm::mat4 Transform::getTransformMatrix() const 
{
    glm::mat4 translation = glm::translate(glm::mat4(1.0f), m_position);
    glm::mat4 rotation = glm::toMat4(m_rotation);
    glm::mat4 scale = glm::scale(glm::mat4(1.0f), m_scale);
    return translation * rotation * scale;
}

void Transform::setTransformMatrix(const glm::mat4& matrix) 
{
    glm::vec3 skew;
    glm::vec4 perspective;

    // Decompose the matrix
    glm::decompose(matrix, m_scale, m_rotation, m_position, skew, perspective);

    // Ensure rotation quaternion is normalized
    m_rotation = glm::normalize(m_rotation);
}