#include "VulkanApplication.hpp"
#include <iostream>
#include "GLM_defines.hpp"
#include "EventManager.hpp"
#include "Time.hpp"

void VulkanApplication::handleWindowResize(const WindowResizeEvent& e)
{
    if (e.width <= 0 || e.height <= 0)
    {
        return;
    }
    camera.setPerspective(camera.getFOV(), e.width / (float) e.height, camera.getNearPlane(), camera.getFarPlane());
}

void VulkanApplication::run()
{
    inputManager.init();
    windowManager.init();

    camera.setPerspective(60, GLFW_WINDOW_WIDTH / (float) GLFW_WINDOW_HEIGHT, 0.1, 100.0f);
    controls = new CreativeControls(camera, 10.0, 100);
    camera.transform.setRotation(glm::vec3(0, 0, 0));
    camera.transform.setPosition(glm::vec3(0, 0, 5));
    EventManager::get().sink<WindowResizeEvent>().connect <&VulkanApplication::handleWindowResize>(this);

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
    VulkanModel vikingRoom;

    models.push_back(portalGun);
    models.push_back(portalGunGlass);
    models.push_back(vikingRoom);

    graphicsPipeline.createDescriptorPool(context, models.size(), 1); // 1 for the fullscreen quad

    models[0].init("models/portal_gun/portal_gun.obj", "models/portal_gun/PortalGun_Albedo.png", context, commandBufferManager, graphicsPipeline);
    models[1].init("models/portal_gun/portal_gun_glass.obj", "textures/white.jpg", context, commandBufferManager, graphicsPipeline);
    models[2].init("models/viking_room/viking_room.obj", "models/viking_room/viking_room.png", context, commandBufferManager, graphicsPipeline);

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


    // Portal gun
    float scale = 5.0f;
    for (int i = 0; i < 2; i++)
    {
        models[i].transform.setTransformMatrix(glm::mat4(1.0f));
        models[i].transform.scale(glm::vec3(scale, scale, scale));
        models[i].transform.setRotation(glm::vec3(0, Time::time() * 45.0, 0));
    }

    // Viking room
    scale = 5.0f;
    models[2].transform.setTransformMatrix(glm::mat4(1.0f));
    models[2].transform.scale(glm::vec3(scale, scale, scale));
    models[2].transform.translate(glm::vec3(0, -2, 0));
    models[2].transform.rotate(glm::vec3(-90, -90, 0));
}

void VulkanApplication::mainLoop()
{
    while (!shouldTerminate())
    {
        Time::update();
        glfwPollEvents();
        inputManager.retrieveInputs(windowManager.getWindow());
        controls->update(inputManager);
        updateScene();
        renderer.drawFrame(windowManager.getWindow(), context, swapChainManager, graphicsPipeline, commandBufferManager, camera, models, fullScreenQuad);
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
    controls->cleanup();
    delete controls;
    EventManager::get().sink<WindowResizeEvent>().disconnect<&VulkanApplication::handleWindowResize>(this);
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