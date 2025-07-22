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
    swapChainManager.init(windowManager.getWindow(), context);
    graphicsPipelineManager.initPipelines(context, commandBufferManager, swapChainManager);

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
        RunTimeSettings::displayRayTracing = !RunTimeSettings::displayRayTracing;
        std::cout << "Ray tracing enabled: " << RunTimeSettings::displayRayTracing << std::endl;
    }
    if (inputManager.isKeyJustPressed(KeyboardKey::T))
    {
        RunTimeSettings::renderScale = std::max(std::fmod(RunTimeSettings::renderScale, 1.0f) + 0.1, 0.1);
        std::cout << "New render scale: " << RunTimeSettings::renderScale << std::endl;
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
}

void VulkanApplication::mainLoop()
{
    while (!shouldTerminate())
    {
        try
        {
            Time::update();
            glfwPollEvents();
            inputManager.retrieveInputs(windowManager.getWindow());
            handleInputs();

            Scene::update();
            renderer.drawFrame(windowManager.getWindow(), context, swapChainManager, graphicsPipelineManager, commandBufferManager, camera, Scene::getModels(), fullScreenQuad);
        
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