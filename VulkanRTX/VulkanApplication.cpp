#include "VulkanApplication.hpp"
#include <iostream>
#include "GLM_defines.hpp"

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

    // Models
    VulkanModel model1;
    VulkanModel model2;

    models.push_back(model1);
    models.push_back(model2);

    // Pipeline ressources
    graphicsPipeline.createDescriptorPool(context, models.size());

    models[0].load(MODEL_PATH, context, commandBufferManager);
    models[1].load(MODEL_PATH, context, commandBufferManager);

    models[0].init(context, commandBufferManager, graphicsPipeline);
    models[1].init(context, commandBufferManager, graphicsPipeline);

    // Renderer
    renderer.createSyncObjects(context);

    std::cout << "VK initialization finished !" << std::endl;
}

void VulkanApplication::updateScene()
{
    static auto startTime = std::chrono::high_resolution_clock::now();
    auto currentTime = std::chrono::high_resolution_clock::now();
    float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

    models[0].modelMatrix = glm::mat4(1.0f);
    models[0].modelMatrix = glm::translate(models[0].modelMatrix, glm::vec3(-2, 0, 0));
    models[0].modelMatrix = glm::rotate(models[0].modelMatrix, time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    
    models[1].modelMatrix = glm::mat4(1.0f);
    models[1].modelMatrix = glm::translate(models[1].modelMatrix, glm::vec3(2, 0, 0));
    models[1].modelMatrix = glm::rotate(models[1].modelMatrix, -time * 2* glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
}

void VulkanApplication::mainLoop()
{
    while (!glfwWindowShouldClose(windowManager.getWindow()))
    {
        glfwPollEvents();

        updateScene();
        renderer.drawFrame(windowManager.getWindow(), context, swapChainManager, graphicsPipeline, commandBufferManager, models);
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
    for (VulkanModel model : models)
    {
        model.cleanup(context.device);
    }
    graphicsPipeline.cleanup(context.device);
    renderer.cleanup(context.device);
    commandBufferManager.cleanup(context.device);
    context.cleanup();
    windowManager.cleanup();
    glfwTerminate();
}