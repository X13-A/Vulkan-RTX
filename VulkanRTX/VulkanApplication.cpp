#include "VulkanApplication.hpp"
#include <iostream>
#include "GLM_defines.hpp"
#include "EventManager.hpp"
#include "Time.hpp"
#include "VulkanTLAS.hpp"
#include "Scene.hpp"
#include "RunTimeSettings.hpp"
#include "DescriptorSetLayoutManager.hpp"
#include "TextureManager.hpp"

void VulkanApplication::handleWindowResize(const WindowResizeEvent& e)
{
    if (e.scaledWidth <= 0 || e.scaledHeight <= 0 || e.nativeWidth <= 0 || e.nativeHeight <= 0)
    {
        return;
    }

    Time::resetFrameCount();
    scaledWidth = e.scaledWidth;
    scaledHeight = e.scaledHeight;
    nativeWidth = e.nativeWidth;
    nativeHeight = e.nativeHeight;

    std::cout << "Resizing resources..." << std::endl;
    camera.setPerspective(camera.getFOV(), e.scaledWidth / (float) e.scaledHeight, camera.getNearPlane(), camera.getFarPlane());
    swapChainManager.handleResize(e.nativeWidth, e.nativeHeight, context, commandBufferManager, graphicsPipelineManager.lightingPipeline.getRenderPass());
    graphicsPipelineManager.handleResize(e.nativeWidth, e.nativeHeight, e.scaledWidth, e.scaledHeight, context, commandBufferManager);
    fullScreenQuad.writeDescriptorSets(context, graphicsPipelineManager.gBufferManager.depthImageView, graphicsPipelineManager.gBufferManager.normalImageView, graphicsPipelineManager.gBufferManager.albedoImageView);
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
    context.loadFunctionPointers();

    DescriptorSetLayoutManager::createModelLayout(context);
    DescriptorSetLayoutManager::createMaterialLayout(context);
    DescriptorSetLayoutManager::createFullScreenQuadLayout(context);

    // CommandBuffers
    commandBufferManager.createCommandPool(context);
    commandBufferManager.createCommandBuffers(context);

    // Load textures
    TextureManager::loadTextures(context, commandBufferManager);

    // Swapchain, pipeline
    glfwGetFramebufferSize(windowManager.getWindow(), &nativeWidth, &nativeHeight);
    scaledWidth = static_cast<int> ((float)nativeWidth * RunTimeSettings::renderScale);
    scaledHeight = static_cast<int> ((float)nativeHeight * RunTimeSettings::renderScale);

    swapChainManager.init(nativeWidth, nativeHeight, context);
    graphicsPipelineManager.initPipelines(nativeWidth, nativeHeight, scaledWidth, scaledHeight, context, commandBufferManager, swapChainManager.swapChainImageFormat);

    // Swapchain ressources
    swapChainManager.createFramebuffers(context, graphicsPipelineManager.lightingPipeline.getRenderPass());
    
    Scene::fetchModels();
    
    graphicsPipelineManager.createDescriptorPool(context, Scene::getModelCount(), Scene::getMeshCount(), FULLSCREEN_QUAD_COUNT);

    Scene::loadModels(context, commandBufferManager, graphicsPipelineManager.descriptorPool);

    // Create TLAS
    sceneTLAS.createTLAS(context, Scene::getModels(), commandBufferManager);

    // Setup RT pipeline with scene info
    graphicsPipelineManager.rtPipeline.writeDescriptors(context, commandBufferManager, Scene::getModels(), sceneTLAS.getTLAS(), graphicsPipelineManager.gBufferManager.depthImageView, graphicsPipelineManager.gBufferManager.normalImageView, graphicsPipelineManager.gBufferManager.albedoImageView);

    // Init fullscreen quad
    fullScreenQuad.init(context, commandBufferManager, 
        graphicsPipelineManager.descriptorPool,
        graphicsPipelineManager.gBufferManager.depthImageView,
        graphicsPipelineManager.gBufferManager.normalImageView, 
        graphicsPipelineManager.gBufferManager.albedoImageView);

    // Renderer
    renderer.createSyncObjects(context, swapChainManager);

    std::cout << "VK initialization finished !" << std::endl;
}

void VulkanApplication::handleInputs()
{
    controls->update(inputManager);
    if (inputManager.isKeyJustPressed(KeyboardKey::R))
    {
        Time::resetFrameCount();
        RunTimeSettings::displayRayTracing = !RunTimeSettings::displayRayTracing;
        std::cout << "Ray tracing enabled: " << RunTimeSettings::displayRayTracing << std::endl;
    }
    if (inputManager.isKeyJustPressed(KeyboardKey::T))
    {
        RunTimeSettings::renderScale = std::max(std::fmod(RunTimeSettings::renderScale, 1.0f) + 0.1, 0.1);
        std::cout << "New render scale: " << RunTimeSettings::renderScale << std::endl;

        WindowResizeEvent e;
        glfwGetFramebufferSize(windowManager.getWindow(), &e.nativeWidth, &e.nativeHeight);
        e.scaledWidth = static_cast<int> ((float)nativeWidth * RunTimeSettings::renderScale);
        e.scaledHeight = static_cast<int> ((float)nativeHeight * RunTimeSettings::renderScale);
        EventManager::get().trigger(e);
    }
    if (inputManager.isKeyJustPressed(KeyboardKey::O))
    {
        RunTimeSettings::spp = std::max(0, (int) RunTimeSettings::spp - 1);
        std::cout << "Samples per pixel: " << RunTimeSettings::spp << std::endl;
    }
    if (inputManager.isKeyJustPressed(KeyboardKey::P))
    {
        RunTimeSettings::spp = std::max(0, (int)RunTimeSettings::spp + 1);
        std::cout << "Samples per pixel: " << RunTimeSettings::spp << std::endl;
    }
    if (inputManager.isKeyPressed(KeyboardKey::F))
    {
        Time::resetFrameCount();
        std::cout << "Reset frame count !" << std::endl;
    }
}

void VulkanApplication::mainLoop()
{
    while (!shouldTerminate())
    {
        try
        {
            glfwPollEvents();
            inputManager.retrieveInputs(windowManager.getWindow());
            handleInputs();

            Scene::update();
            renderer.drawFrame(nativeWidth, nativeHeight, scaledWidth, scaledHeight, windowManager.getWindow(), context, swapChainManager, graphicsPipelineManager, commandBufferManager, camera, Scene::getModels(), fullScreenQuad);
        
            Time::update();
            updateFPS();
        }
        catch (std::exception e)
        {
            std::cerr << e.what() << std::endl;
            break;
        }
    }

    vkDeviceWaitIdle(context.device);
}

void VulkanApplication::updateFPS()
{
    static int frameCount = 0;
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
    sceneTLAS.cleanup(context);
    controls->cleanup();
    delete controls;
    EventManager::get().sink<WindowResizeEvent>().disconnect<&VulkanApplication::handleWindowResize>(this);
    inputManager.cleanup();
    swapChainManager.cleanup(context.device);
    Scene::cleanup(context.device);
    fullScreenQuad.cleanup(context.device);
    graphicsPipelineManager.cleanup(context.device);
    renderer.cleanup(context.device);
    commandBufferManager.cleanup(context.device);
    DescriptorSetLayoutManager::cleanup(context.device);
    TextureManager::cleanup(context.device);
    context.cleanup();
    windowManager.cleanup();
    glfwTerminate();
}