#include "WindowManager.hpp"
#include "Constants.hpp"
#include <stdexcept>
#include "EventManager.hpp"
#include "AllEvents.hpp"

void WindowManager::init()
{
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    window = glfwCreateWindow(GLFW_WINDOW_WIDTH, GLFW_WINDOW_HEIGHT, GLFW_WINDOW_NAME, nullptr, nullptr);
    glfwSetWindowUserPointer(window, this);

    // Set callbacks
    glfwSetFramebufferSizeCallback(window, WindowManager::framebufferResizeCallback);
    glfwSetCursorPosCallback(window, WindowManager::mouseCallback);
    glfwSetScrollCallback(window, WindowManager::scrollCallbaack);
}

GLFWwindow* WindowManager::getWindow() const
{
    return window;
}

void WindowManager::mouseCallback(GLFWwindow* window, double xpos, double ypos)
{
    EventManager::get().trigger(MouseMoveEvent{ xpos, ypos });
}

void WindowManager::scrollCallbaack(GLFWwindow* window, double xoffset, double yoffset)
{
    EventManager::get().trigger(MouseScrollEvent{ xoffset, yoffset });
}

void WindowManager::setResizeCallback(std::function<void()> callback)
{
    resizeCallback = std::move(callback);
}

void WindowManager::framebufferResizeCallback(GLFWwindow* window, int width, int height)
{
    auto windowManager = static_cast<WindowManager*>(glfwGetWindowUserPointer(window));
    if (windowManager->resizeCallback)
    {
        windowManager->resizeCallback();
    }
}

void WindowManager::createSurface(VulkanContext& context)
{
    if (glfwCreateWindowSurface(context.instance, window, nullptr, &context.surface) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create window surface!");
    }
}

void WindowManager::cleanup()
{
    glfwDestroyWindow(window);
}