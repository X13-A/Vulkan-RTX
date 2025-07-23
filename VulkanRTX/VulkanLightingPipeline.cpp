#include "VulkanLightingPipeline.hpp"
#include <stdexcept>
#include "Utils.hpp"
#include "DescriptorSetLayoutManager.hpp"
#include "RunTimeSettings.hpp"

void VulkanLightingPipeline::init(const VulkanContext& context, VulkanCommandBufferManager& commandBufferManager, VkFormat swapChainImageFormat)
{
    createRenderPasses(context, swapChainImageFormat);
    createPipelineLayouts(context);
    createPipeline(context);
}

void VulkanLightingPipeline::createRenderPasses(const VulkanContext& context, VkFormat swapChainImageFormat)
{
    VkAttachmentDescription colorAttachment = {};
    colorAttachment.format = swapChainImageFormat;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colorAttachmentRef = {};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription lightingSubpass = {};
    lightingSubpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    lightingSubpass.colorAttachmentCount = 1;
    lightingSubpass.pColorAttachments = &colorAttachmentRef;

    std::array<VkAttachmentDescription, 1> lightingAttachments = { colorAttachment };
    VkRenderPassCreateInfo lightingRenderPassInfo{};
    lightingRenderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    lightingRenderPassInfo.attachmentCount = static_cast<uint32_t>(lightingAttachments.size());
    lightingRenderPassInfo.pAttachments = lightingAttachments.data();
    lightingRenderPassInfo.subpassCount = 1;
    lightingRenderPassInfo.pSubpasses = &lightingSubpass;

    if (vkCreateRenderPass(context.device, &lightingRenderPassInfo, nullptr, &renderPass) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create lighting pass render pass!");
    }
}

void VulkanLightingPipeline::createPipelineLayouts(const VulkanContext& context)
{
    std::array<VkDescriptorSetLayout, 1> lightingDescriptorSetLayouts = 
    {
        DescriptorSetLayoutManager::getFullScreenQuadLayout() 
    };

    VkPipelineLayoutCreateInfo lightingPipelineLayoutInfo{};
    lightingPipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    lightingPipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(lightingDescriptorSetLayouts.size());
    lightingPipelineLayoutInfo.pSetLayouts = lightingDescriptorSetLayouts.data();
    lightingPipelineLayoutInfo.pushConstantRangeCount = 0;

    if (vkCreatePipelineLayout(context.device, &lightingPipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create lighting pass pipeline layout!");
    }
}

void VulkanLightingPipeline::createPipeline(const VulkanContext& context)
{
    std::vector<char> lightingVertShaderCode = readFile("shaders/lighting_vert.spv");
    std::vector<char> lightingFragShaderCode = readFile("shaders/lighting_frag.spv");

    VkShaderModule lightingVertShaderModule = VulkanUtils::Shaders::createShaderModule(context, lightingVertShaderCode);
    VkShaderModule lightingFragShaderModule = VulkanUtils::Shaders::createShaderModule(context, lightingFragShaderCode);

    VkPipelineShaderStageCreateInfo lightingVertShaderStageInfo{};
    lightingVertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    lightingVertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    lightingVertShaderStageInfo.module = lightingVertShaderModule;
    lightingVertShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo lightingFragShaderStageInfo{};
    lightingFragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    lightingFragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    lightingFragShaderStageInfo.module = lightingFragShaderModule;
    lightingFragShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo lightingShaderStages[] = { lightingVertShaderStageInfo, lightingFragShaderStageInfo };

    VkPipelineDepthStencilStateCreateInfo depthStencilLighting{};
    depthStencilLighting.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencilLighting.depthTestEnable = VK_FALSE;
    depthStencilLighting.depthWriteEnable = VK_FALSE;
    depthStencilLighting.depthCompareOp = VK_COMPARE_OP_LESS;
    depthStencilLighting.depthBoundsTestEnable = VK_FALSE;
    depthStencilLighting.stencilTestEnable = VK_FALSE;

    VkPipelineColorBlendAttachmentState lightingBlendAttachment{};
    lightingBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    lightingBlendAttachment.blendEnable = VK_FALSE;

    VkPipelineColorBlendAttachmentState lightingBlendAttachments[]{ lightingBlendAttachment };

    VkPipelineColorBlendStateCreateInfo lightingPassColorBlending{};
    lightingPassColorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    lightingPassColorBlending.logicOpEnable = VK_FALSE;
    lightingPassColorBlending.attachmentCount = 1;
    lightingPassColorBlending.pAttachments = lightingBlendAttachments;

    // Define vertex input state (vertex binding and attributes)
    VkVertexInputBindingDescription bindingDescription = VulkanVertex::getBindingDescription();
    std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions = VulkanVertex::getAttributeDescriptions();

    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount = 1;
    vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
    vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
    vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

    // Input assembly state
    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    // Multisampling state (no anti-aliasing for now)
    VkPipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    VkPipelineViewportStateCreateInfo viewportState{};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.scissorCount = 1;

    // Dynamic state (viewport and scissor)
    std::vector<VkDynamicState> dynamicStates =
    {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR
    };

    // Rasterizer state
    VkPipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;

    VkPipelineDynamicStateCreateInfo dynamicState{};
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
    dynamicState.pDynamicStates = dynamicStates.data();

    // Set up the lighting pass pipeline
    VkGraphicsPipelineCreateInfo lightingPipelineInfo{};
    lightingPipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    lightingPipelineInfo.stageCount = 2;
    lightingPipelineInfo.pStages = lightingShaderStages;
    lightingPipelineInfo.pVertexInputState = &vertexInputInfo;
    lightingPipelineInfo.pInputAssemblyState = &inputAssembly;
    lightingPipelineInfo.pViewportState = &viewportState;
    lightingPipelineInfo.pRasterizationState = &rasterizer;
    lightingPipelineInfo.pMultisampleState = &multisampling;
    lightingPipelineInfo.pColorBlendState = &lightingPassColorBlending;
    lightingPipelineInfo.pDynamicState = &dynamicState;
    lightingPipelineInfo.layout = pipelineLayout;
    lightingPipelineInfo.renderPass = renderPass;
    lightingPipelineInfo.subpass = 0;
    lightingPipelineInfo.pDepthStencilState = &depthStencilLighting;

    if (vkCreateGraphicsPipelines(context.device, VK_NULL_HANDLE, 1, &lightingPipelineInfo, nullptr, &pipeline) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create lighting pass graphics pipeline!");
    }

    vkDestroyShaderModule(context.device, lightingVertShaderModule, nullptr);
    vkDestroyShaderModule(context.device, lightingFragShaderModule, nullptr);
}

void VulkanLightingPipeline::handleResize(const VulkanContext& context, VulkanCommandBufferManager& commandBufferManager)
{
    // TODO: this is not necessary because of dynamic state
    vkDestroyPipeline(context.device, pipeline, nullptr);
    createPipeline(context);
}

VkRenderPass VulkanLightingPipeline::getRenderPass() const
{
    return renderPass;
}

VkPipeline VulkanLightingPipeline::getPipeline() const
{
    return pipeline;
}

VkPipelineLayout VulkanLightingPipeline::getPipelineLayout() const
{
    return pipelineLayout;
}

void VulkanLightingPipeline::cleanup(VkDevice device)
{
    vkDestroyPipeline(device, pipeline, nullptr);

    vkDestroyPipelineLayout(device, pipelineLayout, nullptr);

    vkDestroyRenderPass(device, renderPass, nullptr);
}

void VulkanLightingPipeline::recordDrawCommands(int width, int height, const VulkanSwapChainManager& swapChainManager, const VulkanFullScreenQuad& fullScreenQuad, VkCommandBuffer commandBuffer, uint32_t currentFrame, uint32_t imageIndex)
{
    VkRenderPassBeginInfo lightingRenderPassInfo{};
    lightingRenderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    lightingRenderPassInfo.renderPass = renderPass;
    lightingRenderPassInfo.framebuffer = swapChainManager.swapChainFramebuffers[imageIndex];
    lightingRenderPassInfo.renderArea.offset = { 0, 0 };
    lightingRenderPassInfo.renderArea.extent = swapChainManager.swapChainExtent;

    std::array<VkClearValue, 1> lightingClearValues{};
    lightingClearValues[0].color = { {0.0f, 0.0f, 0.0f, 1.0f} };

    lightingRenderPassInfo.clearValueCount = static_cast<uint32_t>(lightingClearValues.size());
    lightingRenderPassInfo.pClearValues = lightingClearValues.data();

    vkCmdBeginRenderPass(commandBuffer, &lightingRenderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

    VkExtent2D extent;
    extent.width = width;
    extent.height = height;

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = width;
    viewport.height = height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

    VkRect2D scissor{};
    scissor.offset = { 0, 0 };
    scissor.extent = extent;
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &fullScreenQuad.descriptorSets[currentFrame], 0, nullptr);

    VkBuffer vertexBuffers[] = { fullScreenQuad.vertexBuffer };
    VkDeviceSize offsets[] = { 0 };
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);

    vkCmdDraw(commandBuffer, 6, 1, 0, 0);

    vkCmdEndRenderPass(commandBuffer);
}