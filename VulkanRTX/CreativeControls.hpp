#ifndef CREATIVE_CONTROLS_H
#define CREATIVE_CONTROLS_H

#include "CameraControls.hpp"
#include "InputManager.hpp"
#include "AllEvents.hpp"

class CreativeControls : public CameraControls
{
private:
    float moveSpeed;
    float rotateSpeed;
    float yaw;
    float pitch;

public:
    CreativeControls(Camera& camera, float moveSpeed, float rotateSpeed);

    void handleMouseOffset(const MouseOffsetEvent& e) override;

    void handleMouseScroll(const MouseScrollEvent& e) override;

    void update(InputManager& inputManager) override;

    void cleanup();
};

#endif