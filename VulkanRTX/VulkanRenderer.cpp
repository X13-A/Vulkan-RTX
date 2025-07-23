#include "VulkanRenderer.hpp"
#include <iostream>
#include <chrono>
#include <array>
#include "GLM_defines.hpp"
#include "EventManager.hpp"
#include "AllEvents.hpp"
#include "Time.hpp"
#include "RunTimeSettings.hpp"

void VulkanRenderer::createSyncObjects(const VulkanContext& context, const VulkanSwapChainManager& swapChainManager)
{
    renderFinishedSemaphores.resize(swapChainManager.swapChainImages.size());
    imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    for (size_t i = 0; i < swapChainManager.swapChainImages.size(); i++)
    {
        if (vkCreateSemaphore(context.device, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create sync objects !");
        }
    }

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        if (vkCreateFence(context.device, &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS ||
            vkCreateSemaphore(context.device, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create sync objects !");
        }
    }
}

void VulkanRenderer::recordCommandBuffer(int nativeWidth, int nativeHeight, int scaledWidth, int scaledHeight, const VulkanContext& context, VulkanCommandBufferManager& commandBufferManager, const VulkanSwapChainManager swapChainManager, VulkanGraphicsPipelineManager& graphicsPipeline, uint32_t imageIndex, uint32_t currentFrame, const std::vector<VulkanModel>& models, const VulkanFullScreenQuad& fullScreenQuad)
{
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    VkCommandBuffer commandBuffer = commandBufferManager.commandBuffers[currentFrame];

    if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to begin recording command buffer!");
    }

    graphicsPipeline.geometryPipeline.recordDrawCommands(scaledWidth, scaledHeight, models, commandBuffer, currentFrame);
    VulkanUtils::Image::transition_depthRW_to_depthR_existingCmd(context, commandBuffer, graphicsPipeline.gBufferManager.depthImage, VK_FORMAT_D32_SFLOAT);
    graphicsPipeline.lightingPipeline.recordDrawCommands(nativeWidth, nativeHeight, swapChainManager, fullScreenQuad, commandBuffer, currentFrame, imageIndex);

    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to record command buffer!");
    }
}

void VulkanRenderer::triggerResize(GLFWwindow* window, const VulkanContext& context, VulkanSwapChainManager& swapChainManager, VulkanGraphicsPipelineManager& graphicsPipeline, VulkanCommandBufferManager& commandBufferManager, VulkanFullScreenQuad& fullScreenQuad)
{
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);

    WindowResizeEvent e;
    e.nativeWidth = width;
    e.nativeHeight = height;
    e.scaledWidth = static_cast<int> ((float)width * RunTimeSettings::renderScale);
    e.scaledHeight = static_cast<int> ((float)height * RunTimeSettings::renderScale);
    EventManager::get().trigger(e);
}

void VulkanRenderer::drawFrame(int nativeWidth, int nativeHeight, int scaledWidth, int scaledHeight, GLFWwindow* window, const VulkanContext& context, VulkanSwapChainManager& swapChainManager, VulkanGraphicsPipelineManager& graphicsPipeline, VulkanCommandBufferManager& commandBufferManager, const Camera& camera, const std::vector<VulkanModel>& models, VulkanFullScreenQuad& fullScreenQuad)
{
    vkWaitForFences(context.device, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

    // Get next image
    uint32_t imageIndex;
    VkResult result = vkAcquireNextImageKHR(context.device, swapChainManager.swapChain, UINT64_MAX, imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);

    // Handle resize
    if (result == VK_ERROR_OUT_OF_DATE_KHR)
    {
        triggerResize(window, context, swapChainManager, graphicsPipeline, commandBufferManager, fullScreenQuad);
        return;
    }
    else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
    {
        throw std::runtime_error("failed to acquire swap chain image!");
    }

    // Sync
    vkResetFences(context.device, 1, &inFlightFences[currentFrame]);
    vkResetCommandBuffer(commandBufferManager.commandBuffers[currentFrame], 0);
    recordCommandBuffer(nativeWidth, nativeHeight, scaledWidth, scaledHeight, context, commandBufferManager, swapChainManager, graphicsPipeline, imageIndex, currentFrame, models, fullScreenQuad);

    // Update uniforms
    updateUniformBuffers(camera, models, fullScreenQuad, swapChainManager, graphicsPipeline.rtPipeline, currentFrame);

    // Submit commands
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = { imageAvailableSemaphores[currentFrame] };
    VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT };
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;

    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBufferManager.commandBuffers[currentFrame];

    VkSemaphore signalSemaphores[] = { renderFinishedSemaphores[imageIndex] };
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    if (vkQueueSubmit(context.graphicsQueue, 1, &submitInfo, inFlightFences[currentFrame]) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to submit draw command buffer!");
    }

    if (RunTimeSettings::displayRayTracing)
    {
        // Trace rays
        VkCommandBuffer rtcmd = commandBufferManager.beginSingleTimeCommands(context.device);
        float startTime = Time::time();
        graphicsPipeline.rtPipeline.traceRays(rtcmd, Time::getFrameCount());
        commandBufferManager.endSingleTimeCommands(context.device, context.graphicsQueue, rtcmd);
        float hangTime = Time::time() - startTime;
        if (hangTime > 0.5f)
        {
            std::cerr << "Hang time: " << hangTime << std::endl;
        }

        // Blit ray traced image to swapchain
        VkCommandBuffer commandBuffer = commandBufferManager.beginSingleTimeCommands(context.device);
        VulkanUtils::Image::blitImage(
            commandBuffer,
            graphicsPipeline.rtPipeline.getStorageImage(),       
            swapChainManager.swapChainImages[imageIndex],        
            VK_FORMAT_R8G8B8A8_UNORM,                            
            swapChainManager.swapChainImageFormat,               
            VK_ACCESS_SHADER_WRITE_BIT,                          
            0,                                                   
            VK_IMAGE_LAYOUT_GENERAL,                             
            VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,                     
            graphicsPipeline.rtPipeline.getStorageImageWidth(),  
            graphicsPipeline.rtPipeline.getStorageImageHeight(), 
            swapChainManager.swapChainExtent.width,              
            swapChainManager.swapChainExtent.height,             
            VK_FILTER_LINEAR                                     
        );

        // Save last image for blending
        // TODO: skip useless transition, keep TRANSFER_SRC_BIT
        // TODO: just do a raw copy instead of blit since they have same size, might be faster
        VulkanUtils::Image::blitImage(
            commandBuffer,
            graphicsPipeline.rtPipeline.getStorageImage(),
            graphicsPipeline.rtPipeline.getLastStorageImage(),
            VK_FORMAT_R8G8B8A8_UNORM,
            VK_FORMAT_R8G8B8A8_UNORM,
            VK_ACCESS_SHADER_WRITE_BIT,
            VK_ACCESS_SHADER_READ_BIT,
            VK_IMAGE_LAYOUT_GENERAL,
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            graphicsPipeline.rtPipeline.getStorageImageWidth(),
            graphicsPipeline.rtPipeline.getStorageImageHeight(),
            graphicsPipeline.rtPipeline.getStorageImageWidth(),
            graphicsPipeline.rtPipeline.getStorageImageHeight(),
            VK_FILTER_LINEAR
        );

        commandBufferManager.endSingleTimeCommands(context.device, context.graphicsQueue, commandBuffer);
    }

    // Present image to swapchain
    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;
    VkSwapchainKHR swapChains[] = { swapChainManager.swapChain };
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &imageIndex;

    result = vkQueuePresentKHR(context.presentQueue, &presentInfo);
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || swapChainManager.framebufferResized)
    {
        swapChainManager.framebufferResized = false;
        triggerResize(window, context, swapChainManager, graphicsPipeline, commandBufferManager, fullScreenQuad);
    }
    else if (result != VK_SUCCESS)
    {
        throw std::runtime_error("failed to present swap chain image!");
    }

    currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void VulkanRenderer::updateUniformBuffers(const Camera& camera, const std::vector<VulkanModel>& models, const VulkanFullScreenQuad& fullScreenQuad, const VulkanSwapChainManager& swapChain, VulkanRayTracingPipeline& rtPipeline, uint32_t currentImage)
{
    for (int i = 0; i < models.size(); i++)
    {
        VulkanModelUBO ubo{};
        ubo.modelMat = models[i].transform.getTransformMatrix();
        ubo.normalMat = glm::transpose(glm::inverse(ubo.modelMat));
        
        ubo.viewMat = camera.getViewMatrix();
        ubo.projMat = camera.getProjectionMatrix();

        memcpy(models[i].uniformBuffersMapped[currentImage], &ubo, sizeof(ubo));
    }

    VulkanFullScreenQuadUBO fullScreenUBO{};
    fullScreenUBO.time = 0; // TODO ?
    memcpy(fullScreenQuad.uniformBuffersMapped[currentImage], &fullScreenUBO, sizeof(fullScreenUBO));

    SceneData camData;
    camData.proj = camera.getProjectionMatrix();
    camData.view = camera.getViewMatrix();
    camData.projInverse = glm::inverse(camera.getProjectionMatrix());
    camData.viewInverse = glm::inverse(camera.getViewMatrix());
    camData.cameraPos = camera.transform.getPosition();
    camData.recursionDepth = RT_RECURSION_DEPTH;
    camData.nearFar = glm::vec2(camera.getNearPlane(), camera.getFarPlane());
    camData.spp = RunTimeSettings::spp;
    rtPipeline.updateUniformBuffer(camData);
}

void VulkanRenderer::cleanup(VkDevice device)
{
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) 
    {
        vkDestroySemaphore(device, imageAvailableSemaphores[i], nullptr);
        vkDestroyFence(device, inFlightFences[i], nullptr);
    }

    for (size_t i = 0; i < renderFinishedSemaphores.size(); i++)
    {
        vkDestroySemaphore(device, renderFinishedSemaphores[i], nullptr);
    }
}