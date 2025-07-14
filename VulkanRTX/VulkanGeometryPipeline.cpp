#include "VulkanGeometryPipeline.hpp"
#include <stdexcept>
#include "Utils.hpp"

void VulkanGeometryPipeline::init(const VulkanContext& context, VulkanCommandBufferManager& commandBufferManager, const VulkanSwapChainManager& swapChainManager, const VulkanGBufferManager& gBufferManager)
{
    createRenderPasses(context, swapChainManager.swapChainImageFormat);

    createDescriptorSetLayouts(context);
    createPipelineLayouts(context);
    createPipeline(context, swapChainManager);

    createFramebuffers(context, gBufferManager, swapChainManager.swapChainExtent.width, swapChainManager.swapChainExtent.height);
}

void VulkanGeometryPipeline::createDescriptorSetLayouts(const VulkanContext& context)
{
    // Geometry Pass: Uniform buffer and texture sampler
    VkDescriptorSetLayoutBinding uboLayoutBinding{};
    uboLayoutBinding.binding = 0;
    uboLayoutBinding.descriptorCount = 1;
    uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboLayoutBinding.pImmutableSamplers = nullptr;
    uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

    VkDescriptorSetLayoutBinding samplerLayoutBinding{};
    samplerLayoutBinding.binding = 1;
    samplerLayoutBinding.descriptorCount = 1;
    samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    samplerLayoutBinding.pImmutableSamplers = nullptr;
    samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    std::array<VkDescriptorSetLayoutBinding, 2> bindings =
    {
        uboLayoutBinding,
        samplerLayoutBinding
    };

    // Create descriptor layout for the geometry pass
    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
    layoutInfo.pBindings = bindings.data();

    if (vkCreateDescriptorSetLayout(context.device, &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create geometry pass descriptor set layout!");
    }
}

void VulkanGeometryPipeline::createRenderPasses(const VulkanContext& context, VkFormat swapChainImageFormat)
{
    // Create the render pass for the geometry pass (writes to G-buffer)
    VkAttachmentDescription depthAttachment = {};
    depthAttachment.format = VulkanUtils::DepthStencil::findDepthFormat(context.physicalDevice);
    depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL; // TODO: Figure out why implicit transition to VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL does not seem to work

    VkAttachmentDescription normalAttachment = {};
    normalAttachment.format = VK_FORMAT_R16G16B16A16_SFLOAT;
    normalAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    normalAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    normalAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    normalAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    normalAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    normalAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    normalAttachment.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    VkAttachmentDescription albedoAttachment = {};
    albedoAttachment.format = VK_FORMAT_R32G32B32A32_SFLOAT;
    albedoAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    albedoAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    albedoAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    albedoAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    albedoAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    albedoAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    albedoAttachment.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    VkAttachmentReference normalAttachmentRef = {};
    normalAttachmentRef.attachment = 0;
    normalAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depthAttachmentRef = {};
    depthAttachmentRef.attachment = 1;
    depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference albedoAttachmentRef = {};
    albedoAttachmentRef.attachment = 2;
    albedoAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference colorAttachmentRefs[]{ normalAttachmentRef, albedoAttachmentRef };

    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 2;
    subpass.pColorAttachments = colorAttachmentRefs;
    subpass.pDepthStencilAttachment = &depthAttachmentRef;

    std::array<VkAttachmentDescription, 3> attachments = { normalAttachment, depthAttachment, albedoAttachment };
    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    renderPassInfo.pAttachments = attachments.data();
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;

    // Create the render pass for the geometry pass
    if (vkCreateRenderPass(context.device, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create geometry pass render pass!");
    }
}

void VulkanGeometryPipeline::createFramebuffers(const VulkanContext& context, const VulkanGBufferManager& gBufferManager, uint32_t width, uint32_t height)
{
    // Geometry pass framebuffers
    std::array<VkImageView, 3> geometryAttachments = 
    {
        gBufferManager.normalImageView,
        gBufferManager.depthImageView,
        gBufferManager.albedoImageView
    };

    VkFramebufferCreateInfo geometryFramebufferInfo{};
    geometryFramebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    geometryFramebufferInfo.renderPass = renderPass;
    geometryFramebufferInfo.attachmentCount = static_cast<uint32_t>(geometryAttachments.size());
    geometryFramebufferInfo.pAttachments = geometryAttachments.data();
    geometryFramebufferInfo.width = width;
    geometryFramebufferInfo.height = height;
    geometryFramebufferInfo.layers = 1;

    // Create geometry pass framebuffer (G-buffer)
    if (vkCreateFramebuffer(context.device, &geometryFramebufferInfo, nullptr, &framebuffer) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create geometry pass framebuffer!");
    }
}

void VulkanGeometryPipeline::createPipelineLayouts(const VulkanContext& context)
{
    // Geometry Pipeline Layout
    std::array<VkDescriptorSetLayout, 1> geometryDescriptorSetLayouts = { descriptorSetLayout };

    VkPipelineLayoutCreateInfo geometryPipelineLayoutInfo{};
    geometryPipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    geometryPipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(geometryDescriptorSetLayouts.size());
    geometryPipelineLayoutInfo.pSetLayouts = geometryDescriptorSetLayouts.data();
    geometryPipelineLayoutInfo.pushConstantRangeCount = 0;

    if (vkCreatePipelineLayout(context.device, &geometryPipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create geometry pass pipeline layout!");
    }
}

void VulkanGeometryPipeline::createPipeline(const VulkanContext& context, const VulkanSwapChainManager& swapChainManager)
{
    // Vertex and fragment shaders for geometry pass
    std::vector<char> vertShaderCode = readFile("shaders/geometry_vert.spv");
    std::vector<char> fragShaderCode = readFile("shaders/geometry_frag.spv");

    VkShaderModule vertShaderModule = VulkanUtils::Shaders::createShaderModule(context, vertShaderCode);
    VkShaderModule fragShaderModule = VulkanUtils::Shaders::createShaderModule(context, fragShaderCode);

    VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
    vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertShaderStageInfo.module = vertShaderModule;
    vertShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
    fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageInfo.module = fragShaderModule;
    fragShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

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

    // Viewport and Scissor (can be dynamic)
    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(swapChainManager.swapChainExtent.width);
    viewport.height = static_cast<float>(swapChainManager.swapChainExtent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor{};
    scissor.offset = { 0, 0 };
    scissor.extent = swapChainManager.swapChainExtent;

    VkPipelineViewportStateCreateInfo viewportState{};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.scissorCount = 1;

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

    // Multisampling state (no anti-aliasing for now)
    VkPipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    // Depth and Stencil state for geometry pass (writes depth and normal)
    VkPipelineDepthStencilStateCreateInfo depthStencil{};
    depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencil.depthTestEnable = VK_TRUE;
    depthStencil.depthWriteEnable = VK_TRUE;
    depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
    depthStencil.depthBoundsTestEnable = VK_FALSE;
    depthStencil.stencilTestEnable = VK_FALSE;

    // Color blend state for geometry pass
    VkPipelineColorBlendAttachmentState normalBlendAttachment{};
    normalBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    normalBlendAttachment.blendEnable = VK_FALSE;

    VkPipelineColorBlendAttachmentState albedoBlendAttachment{};
    albedoBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    albedoBlendAttachment.blendEnable = VK_FALSE;

    VkPipelineColorBlendAttachmentState geometryBlendAttachments[]{ normalBlendAttachment, albedoBlendAttachment };

    VkPipelineColorBlendStateCreateInfo geometryPassColorBlending{};
    geometryPassColorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    geometryPassColorBlending.logicOpEnable = VK_FALSE;
    geometryPassColorBlending.attachmentCount = 2;
    geometryPassColorBlending.pAttachments = geometryBlendAttachments;

    // Dynamic state (viewport and scissor)
    std::vector<VkDynamicState> dynamicStates = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR
    };

    VkPipelineDynamicStateCreateInfo dynamicState{};
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
    dynamicState.pDynamicStates = dynamicStates.data();

    // Create geometry pass pipeline
    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pColorBlendState = &geometryPassColorBlending;
    pipelineInfo.pDynamicState = &dynamicState;
    pipelineInfo.layout = pipelineLayout;
    pipelineInfo.renderPass = renderPass;
    pipelineInfo.subpass = 0;
    pipelineInfo.pDepthStencilState = &depthStencil;

    if (vkCreateGraphicsPipelines(context.device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create geometry pass graphics pipeline!");
    }

    // Clean up shader modules as they are no longer needed after pipeline creation
    vkDestroyShaderModule(context.device, vertShaderModule, nullptr);
    vkDestroyShaderModule(context.device, fragShaderModule, nullptr);
}

void VulkanGeometryPipeline::handleResize(const VulkanContext& context, VulkanCommandBufferManager& commandBufferManager, const VulkanSwapChainManager& swapChainManager, const VulkanGBufferManager& gBufferManager, uint32_t width, uint32_t height)
{
    // Pipelines
    vkDestroyPipeline(context.device, pipeline, nullptr);
    createPipeline(context, swapChainManager);

    // Framebuffers
    vkDestroyFramebuffer(context.device, framebuffer, nullptr);
    createFramebuffers(context, gBufferManager, width, height);
}

VkDescriptorSetLayout VulkanGeometryPipeline::getDescriptorSetLayout() const
{
    return descriptorSetLayout;
}

VkRenderPass VulkanGeometryPipeline::getRenderPass() const
{
    return renderPass;
}

VkFramebuffer VulkanGeometryPipeline::getFrameBuffer() const
{
    return framebuffer;
}

VkPipeline VulkanGeometryPipeline::getPipeline() const
{
    return pipeline;
}

VkPipelineLayout VulkanGeometryPipeline::getPipelineLayout() const
{
    return pipelineLayout;
}

void VulkanGeometryPipeline::cleanup(VkDevice device)
{
    // Cleanup pipelines
    vkDestroyPipeline(device, pipeline, nullptr);

    // Cleanup pipeline layouts
    vkDestroyPipelineLayout(device, pipelineLayout, nullptr);

    // Cleanup render passes
    vkDestroyRenderPass(device, renderPass, nullptr);

    // Cleanup descriptor set layouts
    vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);

    // Cleanup framebuffer
    vkDestroyFramebuffer(device, framebuffer, nullptr);
}
