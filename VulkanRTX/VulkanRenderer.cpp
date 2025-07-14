#include "VulkanRenderer.hpp"
#include <iostream>
#include <chrono>
#include <array>
#include "GLM_defines.hpp"
#include "EventManager.hpp"
#include "AllEvents.hpp"
#include "Time.hpp"

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

void VulkanRenderer::recordCommandBuffer(const VulkanContext& context, VulkanCommandBufferManager& commandBufferManager, const VulkanSwapChainManager& swapChainManager, VulkanGraphicsPipelineManager& graphicsPipeline, uint32_t imageIndex, uint32_t currentFrame, const std::vector<VulkanModel>& models, const VulkanFullScreenQuad& fullScreenQuad)
{
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    VkCommandBuffer commandBuffer = commandBufferManager.commandBuffers[currentFrame];

    if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to begin recording command buffer!");
    }

    // Geometry Pass
    VkRenderPassBeginInfo geometryRenderPassInfo{};
    geometryRenderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    geometryRenderPassInfo.renderPass = graphicsPipeline.geometryPipeline.getRenderPass();
    geometryRenderPassInfo.framebuffer = graphicsPipeline.geometryPipeline.getFrameBuffer();
    geometryRenderPassInfo.renderArea.offset = { 0, 0 };
    geometryRenderPassInfo.renderArea.extent = swapChainManager.swapChainExtent;

    std::array<VkClearValue, 3> geometryClearValues{};
    geometryClearValues[0].color = { {0.0f, 0.0f, 0.0f, 1.0f} };
    geometryClearValues[1].depthStencil = { 1.0f, 0 };
    geometryClearValues[2].color = { {0.0f, 0.0f, 0.0f, 1.0f} };

    geometryRenderPassInfo.clearValueCount = static_cast<uint32_t>(geometryClearValues.size());
    geometryRenderPassInfo.pClearValues = geometryClearValues.data();

    vkCmdBeginRenderPass(commandBuffer, &geometryRenderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline.geometryPipeline.getPipeline());

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(swapChainManager.swapChainExtent.width);
    viewport.height = static_cast<float>(swapChainManager.swapChainExtent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

    VkRect2D scissor{};
    scissor.offset = { 0, 0 };
    scissor.extent = swapChainManager.swapChainExtent;
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

    for (const VulkanModel& model : models)
    {
        VkBuffer vertexBuffers[] = { model.vertexBuffer };
        VkDeviceSize offsets[] = { 0 };
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
        vkCmdBindIndexBuffer(commandBuffer, model.indexBuffer, 0, VK_INDEX_TYPE_UINT32);
        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline.geometryPipeline.getPipelineLayout(), 0, 1, &model.descriptorSets[currentFrame], 0, nullptr);

        vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(model.indices.size()), 1, 0, 0, 0);
    }

    vkCmdEndRenderPass(commandBuffer);

    // Lighting Pass
    VulkanUtils::Image::transition_depthRW_to_depthR_existingCmd(context, commandBuffer, graphicsPipeline.gBufferManager.depthImage, VK_FORMAT_D32_SFLOAT);

    VkRenderPassBeginInfo lightingRenderPassInfo{};
    lightingRenderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    lightingRenderPassInfo.renderPass = graphicsPipeline.lightingPipeline.getRenderPass();
    lightingRenderPassInfo.framebuffer = swapChainManager.swapChainFramebuffers[imageIndex];
    lightingRenderPassInfo.renderArea.offset = { 0, 0 };
    lightingRenderPassInfo.renderArea.extent = swapChainManager.swapChainExtent;

    std::array<VkClearValue, 1> lightingClearValues{};
    lightingClearValues[0].color = { {0.0f, 0.0f, 0.0f, 1.0f} };

    lightingRenderPassInfo.clearValueCount = static_cast<uint32_t>(lightingClearValues.size());
    lightingRenderPassInfo.pClearValues = lightingClearValues.data();

    vkCmdBeginRenderPass(commandBuffer, &lightingRenderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline.lightingPipeline.getPipeline());

    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline.lightingPipeline.getPipelineLayout(), 0, 1, &fullScreenQuad.descriptorSets[currentFrame], 0, nullptr);

    VkBuffer vertexBuffers[] = { fullScreenQuad.vertexBuffer };
    VkDeviceSize offsets[] = { 0 };
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);

    vkCmdDraw(commandBuffer, 6, 1, 0, 0);

    vkCmdEndRenderPass(commandBuffer);

    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to record command buffer!");
    }
}

void VulkanRenderer::handleResize(GLFWwindow* window, const VulkanContext& context, VulkanSwapChainManager& swapChainManager, VulkanGraphicsPipelineManager& graphicsPipeline, VulkanCommandBufferManager& commandBufferManager, VulkanFullScreenQuad& fullScreenQuad)
{
    std::cout << "Resizing resources..." << std::endl;

    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    EventManager::get().trigger(WindowResizeEvent{ width, height });
    
    // TODO: use EventManager here
    swapChainManager.handleResize(window, context, commandBufferManager, graphicsPipeline.lightingPipeline.getRenderPass());
    graphicsPipeline.handleResize(window, context, commandBufferManager, swapChainManager);
    fullScreenQuad.writeDescriptorSets(context, graphicsPipeline);
}

void VulkanRenderer::drawFrame(GLFWwindow* window, const VulkanContext& context, VulkanSwapChainManager& swapChainManager, VulkanGraphicsPipelineManager& graphicsPipeline, VulkanCommandBufferManager& commandBufferManager, const Camera& camera, const std::vector<VulkanModel>& models, VulkanFullScreenQuad& fullScreenQuad)
{
    vkWaitForFences(context.device, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

    // Get next image
    uint32_t imageIndex;
    VkResult result = vkAcquireNextImageKHR(context.device, swapChainManager.swapChain, UINT64_MAX, imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);

    // Handle resize
    if (result == VK_ERROR_OUT_OF_DATE_KHR)
    {
        handleResize(window, context, swapChainManager, graphicsPipeline, commandBufferManager, fullScreenQuad);
        return;
    }
    else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
    {
        throw std::runtime_error("failed to acquire swap chain image!");
    }

    // Sync
    vkResetFences(context.device, 1, &inFlightFences[currentFrame]);
    vkResetCommandBuffer(commandBufferManager.commandBuffers[currentFrame], 0);
    recordCommandBuffer(context, commandBufferManager, swapChainManager, graphicsPipeline, imageIndex, currentFrame, models, fullScreenQuad);

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
        handleResize(window, context, swapChainManager, graphicsPipeline, commandBufferManager, fullScreenQuad);
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
    fullScreenUBO.time = 0; // TODO
    memcpy(fullScreenQuad.uniformBuffersMapped[currentImage], &fullScreenUBO, sizeof(fullScreenUBO));

    CameraData camData;
    camData.proj = camera.getProjectionMatrix();
    camData.view = camera.getViewMatrix();
    camData.projInverse = glm::inverse(camera.getProjectionMatrix());
    camData.viewInverse = glm::inverse(camera.getViewMatrix());
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