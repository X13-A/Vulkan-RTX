#include "VulkanApplication.hpp"
#include <iostream>

void VulkanApplication::run()
{
    windowManager.init();
    initVulkan();
    mainLoop();
    cleanup();
}

void VulkanApplication::initVulkan()
{
    // Context
    context.init();
    windowManager.createSurface(context);
    context.initDevice();

    // Swapchain, pipeline
    swapChainManager.init(windowManager.getWindow(), context);
    graphicsPipeline.init(context, swapChainManager);

    // CommandBuffers
    commandBufferManager.createCommandPool(context);
    commandBufferManager.createCommandBuffers(context);

    // Swapchain ressources
    swapChainManager.createDepthResources(context, commandBufferManager);
    swapChainManager.createFramebuffers(context, graphicsPipeline.renderPass);

    // Model
    model.texture.init(context, commandBufferManager);
    model.load(MODEL_PATH, context, commandBufferManager);

    // Pipeline ressources
    graphicsPipeline.createUniformBuffers(context);
    graphicsPipeline.createDescriptorPool(context);
    graphicsPipeline.createDescriptorSets(context, graphicsPipeline.uniformBuffers, model.texture);

    // Renderer
    renderer.createSyncObjects(context);

    std::cout << "VK initialization finished !" << std::endl;
}

void VulkanApplication::mainLoop()
{
    while (!glfwWindowShouldClose(windowManager.getWindow()))
    {
        glfwPollEvents();
        renderer.drawFrame(windowManager.getWindow(), context, swapChainManager, graphicsPipeline, commandBufferManager, model);
        updateFPS();
    }

    vkDeviceWaitIdle(context.device);
}

void VulkanApplication::updateFPS()
{
    frameCount++;

    auto currentTime = std::chrono::high_resolution_clock::now();
    float duration = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - lastTime).count();

    // Average over one second
    if (duration >= 1.0f)
    {
        float fps = frameCount / duration;

        std::string newTitle = std::string(GLFW_WINDOW_NAME) + " - FPS: " + std::to_string(static_cast<int>(fps));
        glfwSetWindowTitle(windowManager.getWindow(), newTitle.c_str());

        frameCount = 0;
        lastTime = currentTime;
    }
}

void VulkanApplication::handleResize()
{
    swapChainManager.framebufferResized = true;
}

void VulkanApplication::cleanup()
{
    swapChainManager.cleanup(context.device);
    model.cleanup(context.device);
    graphicsPipeline.cleanup(context.device);
    renderer.cleanup(context.device);
    commandBufferManager.cleanup(context.device);
    context.cleanup();
    windowManager.cleanup();
    glfwTerminate();
}