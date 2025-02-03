#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

int main() 
{
    // Initialize GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW!" << std::endl;
        return -1;
    }

    // Don't create OpenGL context
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    // Create window
    GLFWwindow* window = glfwCreateWindow(800, 600, "Vulkan Window", nullptr, nullptr);

    // Check Vulkan support
    if (!glfwVulkanSupported()) {
        std::cerr << "Vulkan not supported!" << std::endl;
        return -1;
    }
    std::cout << "Vulkan supported!" << std::endl;

    // Cleanup
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
