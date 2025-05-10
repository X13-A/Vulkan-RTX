#include "CreativeControls.hpp"
#include "InputManager.hpp"
#include "Time.hpp"
#include "EventManager.hpp"
#include <iostream>

CreativeControls::CreativeControls(Camera& camera, float moveSpeed, float rotateSpeed) : CameraControls(camera), moveSpeed(moveSpeed), rotateSpeed(rotateSpeed), pitch(0), yaw(0)
{
    EventManager::get().sink<MouseOffsetEvent>().connect<&CreativeControls::handleMouseOffset>(this);
    EventManager::get().sink<MouseScrollEvent>().connect<&CreativeControls::handleMouseScroll>(this);
}

void CreativeControls::handleMouseOffset(const MouseOffsetEvent& e)
{
    yaw += e.xOffset * rotateSpeed * Time::deltaTime();
    pitch += e.yOffset * rotateSpeed * Time::deltaTime();

    if (pitch > 89.0f) pitch = 89.0f;
    if (pitch < -89.0f) pitch = -89.0f;

    camera.transform.setRotation(glm::vec3(pitch, yaw, 0.0));

    std::cout << pitch << ", " << yaw << std::endl;
}

void CreativeControls::handleMouseScroll(const MouseScrollEvent& e)
{
    float fov = camera.getFOV() - e.yOffset;
    if (fov < 1.0f) fov = 1.0f;
    if (fov > 90.0f) fov = 90.0f;
    camera.setFOV(fov);
}

void CreativeControls::update(InputManager& inputManager)
{
    if (inputManager.isKeyPressed(KeyboardKey::Space))
    {
        camera.transform.translate(glm::vec3(0, moveSpeed * Time::deltaTime(), 0));
    }
    if (inputManager.isKeyPressed(KeyboardKey::LSHIFT))
    {
        camera.transform.translate(glm::vec3(0, -moveSpeed * Time::deltaTime(), 0));
    }
    if (inputManager.isKeyPressed(KeyboardKey::A))
    {
        camera.transform.translate(glm::vec3(-moveSpeed * Time::deltaTime(), 0, 0));
    }
    if (inputManager.isKeyPressed(KeyboardKey::D))
    {
        camera.transform.translate(glm::vec3(moveSpeed * Time::deltaTime(), 0, 0));
    }
    if (inputManager.isKeyPressed(KeyboardKey::W))
    {
        camera.transform.translate(glm::vec3(0, 0, -moveSpeed * Time::deltaTime()));
    }
    if (inputManager.isKeyPressed(KeyboardKey::S))
    {
        camera.transform.translate(glm::vec3(0, 0, moveSpeed * Time::deltaTime()));
    }
}

void CreativeControls::cleanup()
{
    EventManager::get().sink<MouseOffsetEvent>().disconnect<&CreativeControls::handleMouseOffset>(this);
    EventManager::get().sink<MouseScrollEvent>().disconnect<&CreativeControls::handleMouseScroll>(this);
}