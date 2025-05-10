#include "Vulkan_GLFW.hpp"
#include "VulkanContext.hpp"
#include <functional>
#include "entt.hpp"

class WindowManager
{
private:
    GLFWwindow* window;
    std::function<void()> resizeCallback;

public:
    GLFWwindow* getWindow();

    void init();
    void setResizeCallback(std::function<void()> callback);
    static void framebufferResizeCallback(GLFWwindow* window, int width, int height);
    static void mouseCallback(GLFWwindow* window, double xpos, double ypos);
    static void scrollCallbaack(GLFWwindow* window, double xoffset, double yoffset);
    void createSurface(VulkanContext& context);
    void cleanup();
};