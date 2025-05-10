#include "Time.hpp"
#include "Vulkan_GLFW.hpp"

double Time::lastTime = 0;
double Time::_deltaTime = 0;

double Time::deltaTime()
{
    return Time::_deltaTime;
}

double Time::time()
{
    return glfwGetTime();
}

void Time::update()
{
    double currentTime = Time::time();
    _deltaTime = currentTime - lastTime;
    lastTime = currentTime;
}

double Time::FPS()
{
    return 1.0f / Time::deltaTime();
}