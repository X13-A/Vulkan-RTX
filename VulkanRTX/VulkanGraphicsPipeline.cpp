#include "VulkanGraphicsPipeline.hpp"
#include "VulkanUtils.hpp"
#include "VulkanGeometry.hpp"
#include "Utils.hpp"
#include <array>
#include <iostream>

void VulkanGraphicsPipeline::init(const VulkanContext& context, VulkanCommandBufferManager& commandBufferManager, const VulkanSwapChainManager& swapChainManager)
{
    gBufferManager.init(context, commandBufferManager, swapChainManager.swapChainExtent.width, swapChainManager.swapChainExtent.height);

    createRenderPasses(context, swapChainManager.swapChainImageFormat);

    createDescriptorSetLayouts(context);

    createPipelineLayouts(context);

    createPipelines(context, swapChainManager);

    createFramebuffers(context, swapChainManager.swapChainExtent.width, swapChainManager.swapChainExtent.height);
}


void VulkanGraphicsPipeline::handleResize(GLFWwindow* window, const VulkanContext& context, VulkanCommandBufferManager& commandBufferManager, const VulkanSwapChainManager& swapChainManager)
{
    int width = 0, height = 0;
    glfwGetFramebufferSize(window, &width, &height);

    while (width == 0 || height == 0)
    {
        glfwGetFramebufferSize(window, &width, &height);
        glfwWaitEvents();
    }

    vkDeviceWaitIdle(context.device);

    // GBuffer
    gBufferManager.cleanup(context.device);
    gBufferManager.init(context, commandBufferManager, width, height);

    // Pipelines
    vkDestroyPipeline(context.device, geometryPipeline, nullptr);
    vkDestroyPipeline(context.device, lightingPipeline, nullptr);
    createPipelines(context, swapChainManager);

    // Framebuffers
    vkDestroyFramebuffer(context.device, geometryFramebuffer, nullptr);
    createFramebuffers(context, width, height);
}

void VulkanGraphicsPipeline::cleanup(VkDevice device)
{
    // Cleanup pipelines
    vkDestroyPipeline(device, geometryPipeline, nullptr);
    vkDestroyPipeline(device, lightingPipeline, nullptr);

    // Cleanup pipeline layouts
    vkDestroyPipelineLayout(device, geometryPipelineLayout, nullptr);
    vkDestroyPipelineLayout(device, lightingPipelineLayout, nullptr);

    // Cleanup render passes
    vkDestroyRenderPass(device, geometryRenderPass, nullptr);
    vkDestroyRenderPass(device, lightingRenderPass, nullptr);

    // Cleanup descriptor set layouts
    vkDestroyDescriptorSetLayout(device, geometryDescriptorSetLayout, nullptr);
    vkDestroyDescriptorSetLayout(device, lightingDescriptorSetLayout, nullptr);

    // Cleanup descriptor pool
    vkDestroyDescriptorPool(device, descriptorPool, nullptr);
}


void VulkanGraphicsPipeline::createDescriptorSetLayouts(const VulkanContext& context)
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

    if (vkCreateDescriptorSetLayout(context.device, &layoutInfo, nullptr, &geometryDescriptorSetLayout) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create geometry pass descriptor set layout!");
    }

    // Lighting Pass: Add additional bindings for lighting calculations
    // Uniform buffer for lighting fragment stage (e.g., lighting parameters, such as light position, intensity, etc.)
    VkDescriptorSetLayoutBinding lightingUboLayoutBinding{};
    lightingUboLayoutBinding.binding = 0;
    lightingUboLayoutBinding.descriptorCount = 1;
    lightingUboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    lightingUboLayoutBinding.pImmutableSamplers = nullptr;
    lightingUboLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    // Samplers for depth and normal textures
    VkDescriptorSetLayoutBinding depthSamplerLayoutBinding{};
    depthSamplerLayoutBinding.binding = 1;
    depthSamplerLayoutBinding.descriptorCount = 1;
    depthSamplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    depthSamplerLayoutBinding.pImmutableSamplers = nullptr;
    depthSamplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    VkDescriptorSetLayoutBinding normalSamplerLayoutBinding{};
    normalSamplerLayoutBinding.binding = 2;
    normalSamplerLayoutBinding.descriptorCount = 1;
    normalSamplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    normalSamplerLayoutBinding.pImmutableSamplers = nullptr;
    normalSamplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    // Lighting pass descriptor set layout will need the uniform buffer for lighting and two samplers (depth and normal)
    std::array<VkDescriptorSetLayoutBinding, 3> lightingBindings = 
    {
        lightingUboLayoutBinding,
        depthSamplerLayoutBinding,
        normalSamplerLayoutBinding,
    };

    // Create descriptor layout for the lighting pass
    VkDescriptorSetLayoutCreateInfo lightingLayoutInfo{};
    lightingLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    lightingLayoutInfo.bindingCount = static_cast<uint32_t>(lightingBindings.size());
    lightingLayoutInfo.pBindings = lightingBindings.data();

    if (vkCreateDescriptorSetLayout(context.device, &lightingLayoutInfo, nullptr, &lightingDescriptorSetLayout) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create lighting pass descriptor set layout!");
    }
}

void VulkanGraphicsPipeline::createDescriptorPool(const VulkanContext& context, size_t modelCount, size_t fullScreenQuadCount)
{
    std::array<VkDescriptorPoolSize, 2> poolSizes{};
    poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizes[0].descriptorCount = static_cast<uint32_t>(modelCount * MAX_FRAMES_IN_FLIGHT + fullScreenQuadCount * MAX_FRAMES_IN_FLIGHT);

    poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSizes[1].descriptorCount = static_cast<uint32_t>(modelCount * MAX_FRAMES_IN_FLIGHT + fullScreenQuadCount * 2 * MAX_FRAMES_IN_FLIGHT);

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    poolInfo.pPoolSizes = poolSizes.data();
    poolInfo.maxSets = static_cast<uint32_t>(modelCount * MAX_FRAMES_IN_FLIGHT + fullScreenQuadCount * MAX_FRAMES_IN_FLIGHT);

    if (vkCreateDescriptorPool(context.device, &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create descriptor pool!");
    }
}

void VulkanGraphicsPipeline::createRenderPasses(const VulkanContext& context, VkFormat swapChainImageFormat)
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
    depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL; // TODO: Figure out why implicit transition to VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL does not seem work

    VkAttachmentDescription normalAttachment = {};
    normalAttachment.format = VK_FORMAT_R16G16B16A16_SFLOAT;
    normalAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    normalAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    normalAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    normalAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    normalAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    normalAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    normalAttachment.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    VkAttachmentReference normalAttachmentRef = {};
    normalAttachmentRef.attachment = 0;
    normalAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depthAttachmentRef = {};
    depthAttachmentRef.attachment = 1;
    depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &normalAttachmentRef;
    subpass.pDepthStencilAttachment = &depthAttachmentRef;
    
    std::array<VkAttachmentDescription, 2> attachments = { normalAttachment, depthAttachment };
    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    renderPassInfo.pAttachments = attachments.data();
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;

    // Create the render pass for the geometry pass
    if (vkCreateRenderPass(context.device, &renderPassInfo, nullptr, &geometryRenderPass) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create geometry pass render pass!");
    }

    // Lighting pass render pass creation
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

    if (vkCreateRenderPass(context.device, &lightingRenderPassInfo, nullptr, &lightingRenderPass) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create lighting pass render pass!");
    }
}

void VulkanGraphicsPipeline::createFramebuffers(const VulkanContext& context, uint32_t width, uint32_t height)
{
    // 1. Geometry pass framebuffers (G-buffer)
    // The geometry pass framebuffer will store depth and normal data in the G-buffer.
    std::array<VkImageView, 2> geometryAttachments = {
        gBufferManager.normalImageView,  // Normal buffer from G-buffer
        gBufferManager.depthImageView    // Depth buffer from G-buffer
    };

    VkFramebufferCreateInfo geometryFramebufferInfo{};
    geometryFramebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    geometryFramebufferInfo.renderPass = geometryRenderPass;  // Render pass for the geometry pass
    geometryFramebufferInfo.attachmentCount = static_cast<uint32_t>(geometryAttachments.size());
    geometryFramebufferInfo.pAttachments = geometryAttachments.data();
    geometryFramebufferInfo.width = width;
    geometryFramebufferInfo.height = height;
    geometryFramebufferInfo.layers = 1;

    // Create geometry pass framebuffer (G-buffer)
    if (vkCreateFramebuffer(context.device, &geometryFramebufferInfo, nullptr, &geometryFramebuffer) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create geometry pass framebuffer!");
    }
}

void VulkanGraphicsPipeline::createPipelineLayouts(const VulkanContext& context)
{
    // Geometry Pipeline Layout
    std::array<VkDescriptorSetLayout, 1> geometryDescriptorSetLayouts = { geometryDescriptorSetLayout };

    VkPipelineLayoutCreateInfo geometryPipelineLayoutInfo{};
    geometryPipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    geometryPipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(geometryDescriptorSetLayouts.size());
    geometryPipelineLayoutInfo.pSetLayouts = geometryDescriptorSetLayouts.data();
    geometryPipelineLayoutInfo.pushConstantRangeCount = 0;

    if (vkCreatePipelineLayout(context.device, &geometryPipelineLayoutInfo, nullptr, &geometryPipelineLayout) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create geometry pass pipeline layout!");
    }

    // Lighting Pipeline Layout
    std::array<VkDescriptorSetLayout, 1> lightingDescriptorSetLayouts = { lightingDescriptorSetLayout };

    VkPipelineLayoutCreateInfo lightingPipelineLayoutInfo{};
    lightingPipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    lightingPipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(lightingDescriptorSetLayouts.size());
    lightingPipelineLayoutInfo.pSetLayouts = lightingDescriptorSetLayouts.data();
    lightingPipelineLayoutInfo.pushConstantRangeCount = 0;

    if (vkCreatePipelineLayout(context.device, &lightingPipelineLayoutInfo, nullptr, &lightingPipelineLayout) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create lighting pass pipeline layout!");
    }
}

void VulkanGraphicsPipeline::createPipelines(const VulkanContext& context, const VulkanSwapChainManager& swapChainManager)
{
    // Vertex and fragment shaders for geometry pass
    std::vector<char> vertShaderCode = readFile("shaders/geometry_vert.spv");
    std::vector<char> fragShaderCode = readFile("shaders/geometry_frag.spv");

    VkShaderModule vertShaderModule = createShaderModule(context, vertShaderCode);
    VkShaderModule fragShaderModule = createShaderModule(context, fragShaderCode);

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
    std::array<VkVertexInputAttributeDescription, 4> attributeDescriptions = VulkanVertex::getAttributeDescriptions();

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

    // Color blend state for geometry pass (no blending for G-buffer)
    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_FALSE;

    VkPipelineColorBlendStateCreateInfo colorBlending{};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;

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
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDynamicState = &dynamicState;
    pipelineInfo.layout = geometryPipelineLayout;
    pipelineInfo.renderPass = geometryRenderPass;
    pipelineInfo.subpass = 0;
    pipelineInfo.pDepthStencilState = &depthStencil;

    if (vkCreateGraphicsPipelines(context.device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &geometryPipeline) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create geometry pass graphics pipeline!");
    }

    // Lighting pass pipeline creation
    // Assuming lighting pass is a full-screen quad, so no vertex attributes other than position
    std::vector<char> lightingVertShaderCode = readFile("shaders/lighting_vert.spv");
    std::vector<char> lightingFragShaderCode = readFile("shaders/lighting_frag.spv");

    VkShaderModule lightingVertShaderModule = createShaderModule(context, lightingVertShaderCode);
    VkShaderModule lightingFragShaderModule = createShaderModule(context, lightingFragShaderCode);

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

    // Depth and Stencil state for geometry pass (writes depth and normal)
    VkPipelineDepthStencilStateCreateInfo depthStencilLighting{};
    depthStencilLighting.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencilLighting.depthTestEnable = VK_FALSE;
    depthStencilLighting.depthWriteEnable = VK_FALSE;
    depthStencilLighting.depthCompareOp = VK_COMPARE_OP_LESS;
    depthStencilLighting.depthBoundsTestEnable = VK_FALSE;
    depthStencilLighting.stencilTestEnable = VK_FALSE;

    // Set up the lighting pass pipeline (assuming it's a fullscreen quad for post-processing)
    VkGraphicsPipelineCreateInfo lightingPipelineInfo{};
    lightingPipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    lightingPipelineInfo.stageCount = 2;
    lightingPipelineInfo.pStages = lightingShaderStages;
    lightingPipelineInfo.pVertexInputState = &vertexInputInfo;
    lightingPipelineInfo.pInputAssemblyState = &inputAssembly;
    lightingPipelineInfo.pViewportState = &viewportState;
    lightingPipelineInfo.pRasterizationState = &rasterizer;
    lightingPipelineInfo.pMultisampleState = &multisampling;
    lightingPipelineInfo.pColorBlendState = &colorBlending;
    lightingPipelineInfo.pDynamicState = &dynamicState;
    lightingPipelineInfo.layout = lightingPipelineLayout;
    lightingPipelineInfo.renderPass = lightingRenderPass;
    lightingPipelineInfo.subpass = 0;
    lightingPipelineInfo.pDepthStencilState = &depthStencilLighting;

    if (vkCreateGraphicsPipelines(context.device, VK_NULL_HANDLE, 1, &lightingPipelineInfo, nullptr, &lightingPipeline) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create lighting pass graphics pipeline!");
    }

    // Clean up shader modules as they are no longer needed after pipeline creation
    vkDestroyShaderModule(context.device, vertShaderModule, nullptr);
    vkDestroyShaderModule(context.device, fragShaderModule, nullptr);
    vkDestroyShaderModule(context.device, lightingVertShaderModule, nullptr);
    vkDestroyShaderModule(context.device, lightingFragShaderModule, nullptr);
}

VkShaderModule VulkanGraphicsPipeline::createShaderModule(const VulkanContext& context, const std::vector<char>& code)
{
    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size();
    createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

    VkShaderModule shaderModule;
    if (vkCreateShaderModule(context.device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create shader module!");
    }
    return shaderModule;
}

