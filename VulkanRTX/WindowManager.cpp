#include "WindowManager.hpp"
#include "Constants.hpp"
#include <stdexcept>

void WindowManager::init()
{
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    window = glfwCreateWindow(GLFW_WINDOW_WIDTH, GLFW_WINDOW_HEIGHT, GLFW_WINDOW_NAME, nullptr, nullptr);
    glfwSetWindowUserPointer(window, this);
    glfwSetFramebufferSizeCallback(window, WindowManager::framebufferResizeCallback);
}

GLFWwindow* WindowManager::getWindow()
{
    return window;
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