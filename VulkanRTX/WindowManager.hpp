#include "Vulkan_GLFW.hpp"
#include "VulkanContext.hpp"
#include <functional>

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
    void createSurface(VulkanContext& context);
    void cleanup();
};