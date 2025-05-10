#pragma once

#include "Transform.hpp"
#include "GLM_defines.hpp"

class Camera 
{
public:
    Transform transform;

    Camera();

    void setPerspective(float fovDegrees, float aspectRatio, float nearPlane, float farPlane);

    glm::mat4 getProjectionMatrix() const;
    float getFOV() const;
    float getAspectRatio() const;
    float getNearPlane() const;
    float getFarPlane() const;

private:
    float m_fov;         // in degrees
    float m_aspectRatio;
    float m_nearPlane;
    float m_farPlane;
};
