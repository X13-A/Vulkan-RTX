#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#define GLFW_EXPOSE_NATIVE_WIN32
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define STB_IMAGE_IMPLEMENTATION
#define GLM_ENABLE_EXPERIMENTAL

// TODO: Alignement requirements
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/hash.hpp>
#include <stb_image.h>
#include <tiny_obj_loader.h>

#include <iostream>
#include <stdexcept>
#include <cstdlib>
#include <vector>
#include <cstring>
#include <optional>
#include <set>
#include <cstdint> 
#include <limits>
#include <algorithm>
#include <fstream>
#include <string>
#include <chrono>
#include <array>
#include <unordered_map>

// Modules
#include "VulkanContext.hpp"
#include "VulkanSwapChain.hpp"
#include "VulkanCommandBufferManager.hpp"
#include "VulkanGraphicsPipeline.hpp"
#include "VulkanGeometry.hpp"
#include "VulkanModel.hpp"
#include "Constants.hpp"
#include "VulkanRenderer.hpp"

class VulkanApplication 
{
private:
    GLFWwindow* window;
    VulkanContext context;
    VulkanSwapChainManager swapChainManager;
    VulkanCommandBufferManager commandBufferManager;
    VulkanGraphicsPipeline graphicsPipeline;
    VulkanModel model;
    VulkanRenderer renderer;

    // Time
    std::chrono::time_point<std::chrono::high_resolution_clock> lastTime;
    int frameCount = 0;

public:
    void run() 
    {
        initWindow();
        initVulkan();
        mainLoop();
        cleanup();
    }

private:
    void initVulkan()
    {
        // TODO: Create header + source files instead of header only
        // TODO: Create init() methods
        // TODO: Pass either all context or specific data such as device etc
        context.createInstance();
        context.setupDebugMessenger();
        createSurface();
        context.pickPhysicalDevice();
        context.createLogicalDevice();
        swapChainManager.createSwapChain(window, context);
        swapChainManager.createImageViews(context);
        graphicsPipeline.createRenderPass(context, swapChainManager);
        graphicsPipeline.createDescriptorSetLayout(context);
        graphicsPipeline.createGraphicsPipeline(context, swapChainManager);
        commandBufferManager.createCommandPool(context);
        swapChainManager.createDepthResources(context, commandBufferManager);
        swapChainManager.createFramebuffers(context, graphicsPipeline.renderPass); // pass render pass directly ?
        model.texture.createTextureImage(context, commandBufferManager);
        model.texture.createTextureImageView(context);
        model.texture.createTextureSampler(context);
        model.load(MODEL_PATH);

        // TODO: make that a method of Model
        VulkanUtils::Buffers::createVertexBuffer(context, commandBufferManager, model.vertices, model.vertexBuffer, model.vertexBufferMemory);
        VulkanUtils::Buffers::createIndexBuffer(context, commandBufferManager, model.indices, model.indexBuffer, model.indexBufferMemory);

        graphicsPipeline.createUniformBuffers(context);
        graphicsPipeline.createDescriptorPool(context);
        graphicsPipeline.createDescriptorSets(context, graphicsPipeline.uniformBuffers, model.texture);
        commandBufferManager.createCommandBuffers(context);
        renderer.createSyncObjects(context);

        std::cout << "VK initialization finished !" << std::endl;
    }

    #pragma region window
    void initWindow()
    {
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
        window = glfwCreateWindow(GLFW_WINDOW_WIDTH, GLFW_WINDOW_HEIGHT, GLFW_WINDOW_NAME, nullptr, nullptr);
        glfwSetWindowUserPointer(window, this);
        glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
    }

    static void framebufferResizeCallback(GLFWwindow* window, int width, int height)
    {
        auto app = reinterpret_cast<VulkanApplication*>(glfwGetWindowUserPointer(window));
        app->swapChainManager.framebufferResized = true;
    }

    void createSurface()
    {
        if (glfwCreateWindowSurface(context.instance, window, nullptr, &context.surface) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create window surface!");
        }
    }

    #pragma endregion
    
    void mainLoop() 
    {
        while (!glfwWindowShouldClose(window))
        {
            glfwPollEvents();
            renderer.drawFrame(window, context, swapChainManager, graphicsPipeline, commandBufferManager, model);
            updateFPS();
        }

        vkDeviceWaitIdle(context.device);
    }

    void updateFPS()
    {
        frameCount++;

        auto currentTime = std::chrono::high_resolution_clock::now();
        float duration = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - lastTime).count();

        // Average over one second
        if (duration >= 1.0f)
        {
            float fps = frameCount / duration;

            std::string newTitle = std::string(GLFW_WINDOW_NAME) + " - FPS: " + std::to_string(static_cast<int>(fps));
            glfwSetWindowTitle(window, newTitle.c_str());

            frameCount = 0;
            lastTime = currentTime;
        }
    }

    void cleanup() 
    {
        //if (enableValidationLayers) 
        //{
        //    VulkanContext::DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
        //}
        //cleanupSwapChain();

        //vkDestroySampler(device, textureSampler, nullptr);
        //vkDestroyImageView(device, textureImageView, nullptr);
        //vkDestroyImage(device, textureImage, nullptr);
        //vkFreeMemory(device, textureImageMemory, nullptr);

        //vkDestroyPipeline(device, graphicsPipeline, nullptr);
        //vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
        //vkDestroyRenderPass(device, renderPass, nullptr);

        //for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) 
        //{
        //    vkDestroyBuffer(device, uniformBuffers[i], nullptr);
        //    vkFreeMemory(device, uniformBuffersMemory[i], nullptr);
        //}

        //vkDestroyDescriptorPool(device, descriptorPool, nullptr);
        //vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);
        //
        //vkDestroyBuffer(device, indexBuffer, nullptr);
        //vkFreeMemory(device, indexBufferMemory, nullptr);

        //vkDestroyBuffer(device, vertexBuffer, nullptr);
        //vkFreeMemory(device, vertexBufferMemory, nullptr);

        //for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) 
        //{
        //    vkDestroySemaphore(device, renderFinishedSemaphores[i], nullptr);
        //    vkDestroySemaphore(device, imageAvailableSemaphores[i], nullptr);
        //    vkDestroyFence(device, inFlightFences[i], nullptr);
        //}

        //vkDestroyCommandPool(device, commandPool, nullptr);

        //vkDestroyDevice(device, nullptr);

        //vkDestroySurfaceKHR(instance, surface, nullptr);
        //
        //vkDestroyInstance(instance, nullptr);
        //
        //glfwDestroyWindow(window);
        //glfwTerminate();
    }

    #pragma endregion
};

int main()
{
    VulkanApplication app;

    try 
    {
        app.run();
    }
    catch (const std::exception& e) 
    {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
