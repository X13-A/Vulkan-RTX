#include "VulkanApplication.hpp"
#include <iostream>
#include "GLM_defines.hpp"

void VulkanApplication::run()
{
    inputManager.init();
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

    // CommandBuffers
    commandBufferManager.createCommandPool(context);
    commandBufferManager.createCommandBuffers(context);

    // Swapchain, pipeline
    swapChainManager.init(windowManager.getWindow(), context);
    graphicsPipeline.init(context, commandBufferManager, swapChainManager);

    // Swapchain ressources
    swapChainManager.createFramebuffers(context, graphicsPipeline.lightingRenderPass);

    // Models
    VulkanModel portalGun;
    VulkanModel portalGunGlass;

    models.push_back(portalGun);
    models.push_back(portalGunGlass);

    graphicsPipeline.createDescriptorPool(context, models.size(), 1); // 1 for the fullscreen quad

    models[0].init("models/portal_gun/portal_gun.obj", "models/portal_gun/PortalGun_Albedo.png", context, commandBufferManager, graphicsPipeline);
    models[1].init("models/portal_gun/portal_gun_glass.obj", "textures/white.jpg", context, commandBufferManager, graphicsPipeline);
    //models[0].init("models/sphere/sphere.obj", "models/portal_gun/PortalGun_Albedo.png", context, commandBufferManager, graphicsPipeline);

    fullScreenQuad.init(context, commandBufferManager, graphicsPipeline);

    // Renderer
    renderer.createSyncObjects(context);

    std::cout << "VK initialization finished !" << std::endl;
}

void VulkanApplication::updateScene()
{
    static auto startTime = std::chrono::high_resolution_clock::now();
    auto currentTime = std::chrono::high_resolution_clock::now();
    float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

    float scale = 10.0f;

    for (int i = 0; i < models.size(); i++)
    {
        models[i].modelMatrix = glm::mat4(1.0f);
        models[i].modelMatrix = glm::scale(models[i].modelMatrix, glm::vec3(scale, scale, scale));
        models[i].modelMatrix = glm::rotate(models[i].modelMatrix, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        models[i].modelMatrix = glm::rotate(models[i].modelMatrix, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        models[i].modelMatrix = glm::rotate(models[i].modelMatrix, time * glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    }

    //models[1].modelMatrix = glm::mat4(1.0f);
    //models[1].modelMatrix = glm::translate(models[1].modelMatrix, glm::vec3(2, 0, 0));
    //models[1].modelMatrix = glm::rotate(models[1].modelMatrix, -time * 0.5f * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
}

void VulkanApplication::mainLoop()
{
    while (!shouldTerminate())
    {
        glfwPollEvents();
        inputManager.retrieveInputs(windowManager.getWindow());
        updateScene();
        renderer.drawFrame(windowManager.getWindow(), context, swapChainManager, graphicsPipeline, commandBufferManager, models, fullScreenQuad);
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

bool VulkanApplication::shouldTerminate() const
{
    if (inputManager.isKeyPressed(KeyboardKey::Escape))
    {
        return true;
    }
    if (glfwWindowShouldClose(windowManager.getWindow()))
    {
        return true;
    }
    return false;
}

void VulkanApplication::cleanup()
{
    inputManager.cleanup();
    swapChainManager.cleanup(context.device);
    for (VulkanModel model : models)
    {
        model.cleanup(context.device);
    }
    fullScreenQuad.cleanup(context.device);
    graphicsPipeline.cleanup(context.device);
    renderer.cleanup(context.device);
    commandBufferManager.cleanup(context.device);
    context.cleanup();
    windowManager.cleanup();
    glfwTerminate();
}