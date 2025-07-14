#include "VulkanRayTracingPipeline.hpp"
#include "Utils.hpp"
#include <stdexcept>
#include "VulkanUtils.hpp"
#include "VulkanExtensionFunctions.hpp"
#include "Constants.hpp"
#include <iostream>

void VulkanRayTracingPipeline::init(const VulkanContext& context, uint32_t width, uint32_t height)
{
    createRayTracingDescriptorSetLayout(context);
    createRayTracingPipelineLayout(context);
    createRayTracingPipeline(context);
    createShaderBindingTable(context);
    createDescriptorPool(context);
    createDescriptorSet(context);
    createUniformBuffer(context);
    createStorageImage(context, width, height);
}

void VulkanRayTracingPipeline::setupScene(const VulkanContext& context, VulkanCommandBufferManager& commandBufferManager, VkAccelerationStructureKHR tlas, const std::vector<VulkanModel>& models)
{
    std::vector<VkImageView> allTextureViews;
    createRayTracingResources(context, commandBufferManager, tlas, models, allTextureViews);
    writeDescriptorSet(context, tlas, allTextureViews);
}

void VulkanRayTracingPipeline::handleResize(const VulkanContext& context, uint32_t width, uint32_t height)
{
    if (storageImageView != VK_NULL_HANDLE)
    {
        vkDestroyImageView(context.device, storageImageView, nullptr);
    }
    if (storageImage != VK_NULL_HANDLE)
    {
        vkDestroyImage(context.device, storageImage, nullptr);
    }
    if (storageImageMemory != VK_NULL_HANDLE)
    {
        vkFreeMemory(context.device, storageImageMemory, nullptr);
    }

    createStorageImage(context, width, height);

    std::vector<VkWriteDescriptorSet> descriptorWrites;

    VkDescriptorImageInfo imageInfo{};
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
    imageInfo.imageView = storageImageView;
    imageInfo.sampler = VK_NULL_HANDLE;

    VkWriteDescriptorSet imageWrite{};
    imageWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    imageWrite.dstSet = descriptorSet;
    imageWrite.dstBinding = 1;
    imageWrite.dstArrayElement = 0;
    imageWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    imageWrite.descriptorCount = 1;
    imageWrite.pImageInfo = &imageInfo;
    descriptorWrites.push_back(imageWrite);

    vkUpdateDescriptorSets(context.device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
}

void VulkanRayTracingPipeline::createRayTracingDescriptorSetLayout(const VulkanContext& context)
{
    // TLAS
    VkDescriptorSetLayoutBinding tlasBinding{};
    tlasBinding.binding = 0;
    tlasBinding.descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
    tlasBinding.descriptorCount = 1;
    tlasBinding.stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR | VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
    tlasBinding.pImmutableSamplers = nullptr;

    // Storage Image
    VkDescriptorSetLayoutBinding storageImageBinding{};
    storageImageBinding.binding = 1;
    storageImageBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    storageImageBinding.descriptorCount = 1;
    storageImageBinding.stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR;
    storageImageBinding.pImmutableSamplers = nullptr;

    // Uniform Buffer
    VkDescriptorSetLayoutBinding uniformBinding{};
    uniformBinding.binding = 2;
    uniformBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uniformBinding.descriptorCount = 1;
    uniformBinding.stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR | VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR | VK_SHADER_STAGE_MISS_BIT_KHR;
    uniformBinding.pImmutableSamplers = nullptr;

    // Vertex Buffer
    VkDescriptorSetLayoutBinding vertexBufferBinding{};
    vertexBufferBinding.binding = 3;
    vertexBufferBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    vertexBufferBinding.descriptorCount = 1;
    vertexBufferBinding.stageFlags = VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
    vertexBufferBinding.pImmutableSamplers = nullptr;

    // Index Buffer
    VkDescriptorSetLayoutBinding indexBufferBinding{};
    indexBufferBinding.binding = 4;
    indexBufferBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    indexBufferBinding.descriptorCount = 1;
    indexBufferBinding.stageFlags = VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
    indexBufferBinding.pImmutableSamplers = nullptr;

    // Textures array
    VkDescriptorSetLayoutBinding textureBinding{};
    textureBinding.binding = 5;
    textureBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    textureBinding.descriptorCount = MAX_ALBEDO_TEXTURES;
    textureBinding.stageFlags = VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
    textureBinding.pImmutableSamplers = nullptr;

    std::array<VkDescriptorSetLayoutBinding, 6> bindings =
    {
        tlasBinding,
        storageImageBinding,
        uniformBinding,
        vertexBufferBinding,
        indexBufferBinding,
        textureBinding
    };

    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
    layoutInfo.pBindings = bindings.data();

    if (vkCreateDescriptorSetLayout(context.device, &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS)
    {
        throw std::runtime_error("Could not create descriptor set layout (ray tracing)!");
    }
}

void VulkanRayTracingPipeline::createRayTracingPipelineLayout(const VulkanContext& context)
{
    // Push constants
    VkPushConstantRange pushConstantRange{};
    pushConstantRange.stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR | VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR | VK_SHADER_STAGE_MISS_BIT_KHR;
    pushConstantRange.offset = 0;
    pushConstantRange.size = sizeof(uint32_t) * 1; // Only frame count for now

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;
    pipelineLayoutInfo.pushConstantRangeCount = 1;
    pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

    if (vkCreatePipelineLayout(context.device, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) 
    {
        throw std::runtime_error("Could not create pipeline layout (ray tracing)!");
    }
}

void VulkanRayTracingPipeline::createRayTracingPipeline(const VulkanContext& context)
{
    // Load rt shaders
    std::vector<char> raygenShaderCode = readFile("shaders/ray_gen.spv");
    std::vector<char> missShaderCode = readFile("shaders/ray_miss.spv");
    std::vector<char> closestHitShaderCode = readFile("shaders/ray_closesthit.spv");

    VkShaderModule raygenShaderModule = VulkanUtils::Shaders::createShaderModule(context, raygenShaderCode);
    VkShaderModule missShaderModule = VulkanUtils::Shaders::createShaderModule(context, missShaderCode);
    VkShaderModule closestHitShaderModule = VulkanUtils::Shaders::createShaderModule(context, closestHitShaderCode);
    
    std::vector<VkPipelineShaderStageCreateInfo> shaderStages;

    // Ray gen
    VkPipelineShaderStageCreateInfo raygenStage{};
    raygenStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    raygenStage.stage = VK_SHADER_STAGE_RAYGEN_BIT_KHR;
    raygenStage.module = raygenShaderModule;
    raygenStage.pName = "main";
    shaderStages.push_back(raygenStage);

    // Miss
    VkPipelineShaderStageCreateInfo missStage{};
    missStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    missStage.stage = VK_SHADER_STAGE_MISS_BIT_KHR;
    missStage.module = missShaderModule;
    missStage.pName = "main";
    shaderStages.push_back(missStage);

    // Closest hit
    VkPipelineShaderStageCreateInfo closestHitStage{};
    closestHitStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    closestHitStage.stage = VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
    closestHitStage.module = closestHitShaderModule;
    closestHitStage.pName = "main";
    shaderStages.push_back(closestHitStage);

    std::vector<VkRayTracingShaderGroupCreateInfoKHR> shaderGroups;

    // Raygen group
    VkRayTracingShaderGroupCreateInfoKHR raygenGroup{};
    raygenGroup.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
    raygenGroup.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
    raygenGroup.generalShader = RT_RAYGEN_SHADER_INDEX; // Raygen shader index
    raygenGroup.closestHitShader = VK_SHADER_UNUSED_KHR;
    raygenGroup.anyHitShader = VK_SHADER_UNUSED_KHR;
    raygenGroup.intersectionShader = VK_SHADER_UNUSED_KHR;
    shaderGroups.push_back(raygenGroup);

    // Miss group
    VkRayTracingShaderGroupCreateInfoKHR missGroup{};
    missGroup.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
    missGroup.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
    missGroup.generalShader = RT_MISS_SHADER_INDEX; // Miss shader index
    missGroup.closestHitShader = VK_SHADER_UNUSED_KHR;
    missGroup.anyHitShader = VK_SHADER_UNUSED_KHR;
    missGroup.intersectionShader = VK_SHADER_UNUSED_KHR;
    shaderGroups.push_back(missGroup);

    // Hit group
    VkRayTracingShaderGroupCreateInfoKHR hitGroup{};
    hitGroup.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
    hitGroup.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_KHR;
    hitGroup.generalShader = VK_SHADER_UNUSED_KHR;
    hitGroup.closestHitShader = RT_CLOSEST_HIT_GENERAL_SHADER_INDEX; // Closest hit shader index
    hitGroup.anyHitShader = VK_SHADER_UNUSED_KHR;
    hitGroup.intersectionShader = VK_SHADER_UNUSED_KHR;
    shaderGroups.push_back(hitGroup);

    // Create pipeline
    VkRayTracingPipelineCreateInfoKHR pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_KHR;
    pipelineInfo.stageCount = static_cast<uint32_t>(shaderStages.size());
    pipelineInfo.pStages = shaderStages.data();
    pipelineInfo.groupCount = static_cast<uint32_t>(shaderGroups.size());
    pipelineInfo.pGroups = shaderGroups.data();
    pipelineInfo.maxPipelineRayRecursionDepth = RT_RECURSION_DEPTH; // Recursion depth
    pipelineInfo.layout = pipelineLayout;

    if (rt_vkCreateRayTracingPipelinesKHR(context.device, VK_NULL_HANDLE, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline) != VK_SUCCESS)
    {
        throw std::runtime_error("Could not create ray tracing pipeline !");
    }

    // Cleanup
    vkDestroyShaderModule(context.device, raygenShaderModule, nullptr);
    vkDestroyShaderModule(context.device, missShaderModule, nullptr);
    vkDestroyShaderModule(context.device, closestHitShaderModule, nullptr);
}

void VulkanRayTracingPipeline::createShaderBindingTable(const VulkanContext& context)
{
    // 1. Récupérer les propriétés du ray tracing
    VkPhysicalDeviceRayTracingPipelinePropertiesKHR rtProps{};
    rtProps.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_PROPERTIES_KHR;

    VkPhysicalDeviceProperties2 props{};
    props.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
    props.pNext = &rtProps;
    vkGetPhysicalDeviceProperties2(context.physicalDevice, &props);

    // 2. Calculer les tailles et alignements
    uint32_t handleSize = rtProps.shaderGroupHandleSize;
    uint32_t handleSizeAligned = ((handleSize + rtProps.shaderGroupBaseAlignment - 1) / rtProps.shaderGroupBaseAlignment) * rtProps.shaderGroupBaseAlignment;

    uint32_t groupCount = 3; // raygen + miss + hit
    uint32_t sbtSize = groupCount * handleSizeAligned;

    // 3. Récupérer les handles des shader groups depuis la pipeline
    std::vector<uint8_t> shaderHandleStorage(sbtSize);
    if (rt_vkGetRayTracingShaderGroupHandlesKHR(context.device, pipeline, 0, groupCount, sbtSize, shaderHandleStorage.data()) != VK_SUCCESS)
    {
        throw std::runtime_error("Could not get ray tracing shader group handles!");
    }

    // 4. Créer le buffer SBT avec VulkanUtils
    VkBufferUsageFlags sbtUsage = VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
    VkMemoryPropertyFlags sbtMemProps = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

    VulkanUtils::Buffers::createBuffer(context, sbtSize, sbtUsage, sbtMemProps, sbtBuffer, sbtBufferMemory, true);

    // 5. Copier les handles dans le buffer SBT
    void* data;
    vkMapMemory(context.device, sbtBufferMemory, 0, sbtSize, 0, &data);

    uint8_t* pData = reinterpret_cast<uint8_t*>(data);
    for (uint32_t g = 0; g < groupCount; g++)
    {
        memcpy(pData, shaderHandleStorage.data() + g * handleSize, handleSize);
        pData += handleSizeAligned;
    }

    vkUnmapMemory(context.device, sbtBufferMemory);

    // 6. Obtenir l'adresse device du buffer SBT
    VkDeviceAddress sbtAddress = VulkanUtils::Buffers::getBufferDeviceAdress(context, sbtBuffer);

    // 7. Configurer les entrées SBT pour chaque type de shader
    // Raygen (index 0)
    raygenSbtEntry.deviceAddress = sbtAddress;
    raygenSbtEntry.stride = handleSizeAligned;
    raygenSbtEntry.size = handleSizeAligned;

    // Miss (index 1) 
    missSbtEntry.deviceAddress = sbtAddress + handleSizeAligned;
    missSbtEntry.stride = handleSizeAligned;
    missSbtEntry.size = handleSizeAligned;

    // Hit (index 2)
    hitSbtEntry.deviceAddress = sbtAddress + handleSizeAligned * 2;
    hitSbtEntry.stride = handleSizeAligned;
    hitSbtEntry.size = handleSizeAligned;

    callableSbtEntry = {};
}

void VulkanRayTracingPipeline::createStorageImage(const VulkanContext& context, uint32_t width, uint32_t height)
{
    VkFormat imageFormat = VK_FORMAT_R8G8B8A8_UNORM;
    VkImageUsageFlags imageUsage = VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    VkMemoryPropertyFlags memProps = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

    VulkanUtils::Image::createImage(context, width, height, imageFormat, VK_IMAGE_TILING_OPTIMAL, imageUsage, memProps, storageImage, storageImageMemory);
    storageImageWidth = width;
    storageImageHeight = height;

    storageImageView = VulkanUtils::Image::createImageView(context, storageImage, imageFormat, VK_IMAGE_ASPECT_COLOR_BIT);
    std::cout << "Storage image size: " << width << ", " << height << std::endl;
}

void VulkanRayTracingPipeline::createUniformBuffer(const VulkanContext& context)
{
    VkDeviceSize bufferSize = sizeof(CameraData);
    VkBufferUsageFlags usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    VkMemoryPropertyFlags properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

    VulkanUtils::Buffers::createBuffer(context, bufferSize, usage, properties, uniformBuffer, uniformBufferMemory, false);

    vkMapMemory(context.device, uniformBufferMemory, 0, bufferSize, 0, &uniformBufferMapped);
}

void VulkanRayTracingPipeline::updateUniformBuffer(const CameraData& cameraData)
{
    memcpy(uniformBufferMapped, &cameraData, sizeof(cameraData));
}

void VulkanRayTracingPipeline::createDescriptorPool(const VulkanContext& context)
{
    std::array<VkDescriptorPoolSize, 5> poolSizes{};
    poolSizes[0].type = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
    poolSizes[0].descriptorCount = 1;
    poolSizes[1].type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    poolSizes[1].descriptorCount = 1;
    poolSizes[2].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizes[2].descriptorCount = 1;
    poolSizes[3].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    poolSizes[3].descriptorCount = 2; // vertex + index
    poolSizes[4].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSizes[4].descriptorCount = MAX_ALBEDO_TEXTURES;

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    poolInfo.pPoolSizes = poolSizes.data();
    poolInfo.maxSets = 1;

    if (vkCreateDescriptorPool(context.device, &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS)
    {
        throw std::runtime_error("Could not create descriptor pool!");
    }
}

void VulkanRayTracingPipeline::createRayTracingResources(const VulkanContext& context, VulkanCommandBufferManager& commandBufferManager, VkAccelerationStructureKHR tlas, const std::vector<VulkanModel>& models, std::vector<VkImageView>& outTextureViews)
{
    // Compute sizes
    size_t totalVertices = 0;
    size_t totalIndices = 0;
    for (const auto& model : models) 
    {
        totalVertices += model.vertices.size();
        totalIndices += model.indices.size();
    }

    // Create combined buffers
    std::vector<VulkanVertex> allVertices;
    std::vector<uint32_t> allIndices;
    allVertices.reserve(totalVertices);
    allIndices.reserve(totalIndices);

    // Combine
    for (const auto& model : models) 
    {
        allVertices.insert(allVertices.end(), model.vertices.begin(), model.vertices.end());
        allIndices.insert(allIndices.end(), model.indices.begin(), model.indices.end());
    }

    // Create vertex buffer
    VkBufferUsageFlags vertexUsageFlags = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    VkMemoryPropertyFlags vertexMemoryFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    VulkanUtils::Buffers::createVertexBuffer(context, commandBufferManager, allVertices, globalVertexBuffer, globalVertexBufferMemory, vertexUsageFlags, vertexMemoryFlags, false);

    // Create index buffer
    VkBufferUsageFlags indexUsageFlags = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    VkMemoryPropertyFlags indexMemoryFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    VulkanUtils::Buffers::createIndexBuffer(context, commandBufferManager, allIndices, globalIndexBuffer, globalIndexBufferMemory, indexUsageFlags, indexMemoryFlags, false);

    // Collect all image views
    for (const auto& model : models) 
    {
        outTextureViews.push_back(model.albedoTexture.textureImageView);
    }

    // Create sampler
    VulkanUtils::Textures::createSampler(context, &globalTextureSampler);
}

void VulkanRayTracingPipeline::createDescriptorSet(const VulkanContext& context)
{
    // Allouer le descriptor set
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = descriptorPool;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &descriptorSetLayout;

    if (vkAllocateDescriptorSets(context.device, &allocInfo, &descriptorSet) != VK_SUCCESS)
    {
        throw std::runtime_error("Could not allocate descriptor set!");
    }
}

void VulkanRayTracingPipeline::writeDescriptorSet(const VulkanContext& context, VkAccelerationStructureKHR tlas, const std::vector<VkImageView>& textureViews)
{
    std::vector<VkWriteDescriptorSet> descriptorWrites;

    // Binding 0: TLAS
    VkWriteDescriptorSetAccelerationStructureKHR accelerationStructureDescriptor{};
    accelerationStructureDescriptor.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_KHR;
    accelerationStructureDescriptor.accelerationStructureCount = 1;
    accelerationStructureDescriptor.pAccelerationStructures = &tlas;

    VkWriteDescriptorSet tlasWrite{};
    tlasWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    tlasWrite.pNext = &accelerationStructureDescriptor;
    tlasWrite.dstSet = descriptorSet;
    tlasWrite.dstBinding = 0;
    tlasWrite.dstArrayElement = 0;
    tlasWrite.descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
    tlasWrite.descriptorCount = 1;
    descriptorWrites.push_back(tlasWrite);

    // Binding 1: Storage Image
    VkDescriptorImageInfo imageInfo{};
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
    imageInfo.imageView = storageImageView;
    imageInfo.sampler = VK_NULL_HANDLE;

    VkWriteDescriptorSet imageWrite{};
    imageWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    imageWrite.dstSet = descriptorSet;
    imageWrite.dstBinding = 1;
    imageWrite.dstArrayElement = 0;
    imageWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    imageWrite.descriptorCount = 1;
    imageWrite.pImageInfo = &imageInfo;
    descriptorWrites.push_back(imageWrite);

    // Binding 2: Uniform Buffer
    VkDescriptorBufferInfo uniformBufferInfo{};
    uniformBufferInfo.buffer = uniformBuffer;
    uniformBufferInfo.offset = 0;
    uniformBufferInfo.range = sizeof(CameraData);

    VkWriteDescriptorSet uniformWrite{};
    uniformWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    uniformWrite.dstSet = descriptorSet;
    uniformWrite.dstBinding = 2;
    uniformWrite.dstArrayElement = 0;
    uniformWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uniformWrite.descriptorCount = 1;
    uniformWrite.pBufferInfo = &uniformBufferInfo;
    descriptorWrites.push_back(uniformWrite);

    // Binding 3: Vertex Buffer
    VkDescriptorBufferInfo vertexBufferInfo{};
    vertexBufferInfo.buffer = globalVertexBuffer;
    vertexBufferInfo.offset = 0;
    vertexBufferInfo.range = VK_WHOLE_SIZE;

    VkWriteDescriptorSet vertexWrite{};
    vertexWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    vertexWrite.dstSet = descriptorSet;
    vertexWrite.dstBinding = 3;
    vertexWrite.dstArrayElement = 0;
    vertexWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    vertexWrite.descriptorCount = 1;
    vertexWrite.pBufferInfo = &vertexBufferInfo;
    descriptorWrites.push_back(vertexWrite);

    // Binding 4: Index Buffer
    VkDescriptorBufferInfo indexBufferInfo{};
    indexBufferInfo.buffer = globalIndexBuffer;
    indexBufferInfo.offset = 0;
    indexBufferInfo.range = VK_WHOLE_SIZE;

    VkWriteDescriptorSet indexWrite{};
    indexWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    indexWrite.dstSet = descriptorSet;
    indexWrite.dstBinding = 4;
    indexWrite.dstArrayElement = 0;
    indexWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    indexWrite.descriptorCount = 1;
    indexWrite.pBufferInfo = &indexBufferInfo;
    descriptorWrites.push_back(indexWrite);

    // Binding 5: Array de textures
    std::vector<VkDescriptorImageInfo> textureInfos(textureViews.size());
    for (size_t i = 0; i < textureViews.size(); i++) 
    {
        textureInfos[i].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        textureInfos[i].imageView = textureViews[i];
        textureInfos[i].sampler = globalTextureSampler;
    }

    VkWriteDescriptorSet textureWrite{};
    textureWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    textureWrite.dstSet = descriptorSet;
    textureWrite.dstBinding = 5;
    textureWrite.dstArrayElement = 0;
    textureWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    textureWrite.descriptorCount = static_cast<uint32_t>(textureViews.size());
    textureWrite.pImageInfo = textureInfos.data();
    descriptorWrites.push_back(textureWrite);

    vkUpdateDescriptorSets(context.device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
}

void VulkanRayTracingPipeline::traceRays(VkCommandBuffer commandBuffer, uint32_t frameCount)
{
    // TODO: use or create VulkanUtils function
    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    barrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = storageImage;
    barrier.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
    barrier.srcAccessMask = 0;
    barrier.dstAccessMask = VK_ACCESS_SHADER_WRITE_BIT;

    vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR, 0, 0, nullptr, 0, nullptr, 1, &barrier);

    // Bind pipeline
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, pipeline);
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, pipelineLayout, 0, 1, &descriptorSet, 0, nullptr);

    // Push constants
    vkCmdPushConstants(commandBuffer,
        pipelineLayout,
        VK_SHADER_STAGE_RAYGEN_BIT_KHR | VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR | VK_SHADER_STAGE_MISS_BIT_KHR,
        0,
        sizeof(uint32_t),
        &frameCount);


    if (debug_vkCmdBeginDebugUtilsLabelEXT)
    {
        VkDebugUtilsLabelEXT labelInfo{};
        labelInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT;
        labelInfo.pLabelName = "BEFORE_TRACE_RAYS";
        labelInfo.color[0] = 1.0f; // Rouge
        labelInfo.color[1] = 0.0f;
        labelInfo.color[2] = 0.0f;
        labelInfo.color[3] = 1.0f;
        debug_vkCmdBeginDebugUtilsLabelEXT(commandBuffer, &labelInfo);
    }

    // RAY TRACE !
    rt_vkCmdTraceRaysKHR(commandBuffer, &raygenSbtEntry, &missSbtEntry, &hitSbtEntry, &callableSbtEntry, storageImageWidth, storageImageHeight, 1);

    if (debug_vkCmdEndDebugUtilsLabelEXT) 
    {
        debug_vkCmdEndDebugUtilsLabelEXT(commandBuffer);
    }
}

void VulkanRayTracingPipeline::cleanup(VkDevice device)
{
    if (uniformBufferMapped) 
    {
        vkUnmapMemory(device, uniformBufferMemory);
    }
    if (descriptorPool != VK_NULL_HANDLE) 
    {
        vkDestroyDescriptorPool(device, descriptorPool, nullptr);
    }
    if (uniformBuffer != VK_NULL_HANDLE) 
    {
        vkDestroyBuffer(device, uniformBuffer, nullptr);
    }
    if (uniformBufferMemory != VK_NULL_HANDLE) 
    {
        vkFreeMemory(device, uniformBufferMemory, nullptr);
    }
    if (storageImageView != VK_NULL_HANDLE) 
    {
        vkDestroyImageView(device, storageImageView, nullptr);
    }
    if (storageImage != VK_NULL_HANDLE) 
    {
        vkDestroyImage(device, storageImage, nullptr);
    }
    if (storageImageMemory != VK_NULL_HANDLE) 
    {
        vkFreeMemory(device, storageImageMemory, nullptr);
    }

    if (globalVertexBuffer != VK_NULL_HANDLE)
    {
        vkDestroyBuffer(device, globalVertexBuffer, nullptr);
    }
    if (globalVertexBufferMemory != VK_NULL_HANDLE)
    {
        vkFreeMemory(device, globalVertexBufferMemory, nullptr);
    }
    if (globalIndexBuffer != VK_NULL_HANDLE)
    {
        vkDestroyBuffer(device, globalIndexBuffer, nullptr);
    }
    if (globalIndexBufferMemory != VK_NULL_HANDLE)
    {
        vkFreeMemory(device, globalIndexBufferMemory, nullptr);
    }
    if (globalTextureSampler != VK_NULL_HANDLE)
    {
        vkDestroySampler(device, globalTextureSampler, nullptr);
    }

    if (sbtBuffer != VK_NULL_HANDLE) 
    {
        vkDestroyBuffer(device, sbtBuffer, nullptr);
    }
    if (sbtBufferMemory != VK_NULL_HANDLE) 
    {
        vkFreeMemory(device, sbtBufferMemory, nullptr);
    }
    if (pipeline != VK_NULL_HANDLE) 
    {
        vkDestroyPipeline(device, pipeline, nullptr);
    }
    if (pipelineLayout != VK_NULL_HANDLE) 
    {
        vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
    }
    if (descriptorSetLayout != VK_NULL_HANDLE) 
    {
        vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);
    }
}

// Getters
VkImage VulkanRayTracingPipeline::getStorageImage() const
{
    return storageImage;
}

VkDescriptorSet VulkanRayTracingPipeline::getDescriptorSet() const
{
    return descriptorSet;
}

VkPipelineLayout VulkanRayTracingPipeline::getPipelineLayout() const
{
    return pipelineLayout;
}

uint32_t VulkanRayTracingPipeline::getStorageImageWidth() const
{
    return storageImageWidth;
}

uint32_t VulkanRayTracingPipeline::getStorageImageHeight() const
{
    return storageImageHeight;
}