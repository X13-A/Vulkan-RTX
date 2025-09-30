#include "VulkanRayTracingPipeline.hpp"
#include "Utils.hpp"
#include <stdexcept>
#include "VulkanUtils.hpp"
#include "VulkanExtensionFunctions.hpp"
#include "Constants.hpp"
#include <iostream>
#include "TextureManager.hpp"
#include "RunTimeSettings.hpp"
#include <random>
#include "DescriptorSetLayoutManager.hpp"

void VulkanRayTracingPipeline::init(const VulkanContext& context, uint32_t width, uint32_t height)
{
    sampleCount = 0;
    createRayTracingPipelineLayout(context);
    createRayTracingPipeline(context);
    createShaderBindingTable(context);
    createDescriptorPool(context);
    createDescriptorSet(context);
    createUniformBuffer(context);
    createStorageImage(context, width, height);
}

void VulkanRayTracingPipeline::writeDescriptors(const VulkanContext& context, VulkanCommandBufferManager& commandBufferManager, const std::vector<VulkanModel>& models, VkAccelerationStructureKHR tlas, VkImageView depthImageView, VkImageView normalsImageView, VkImageView albedoImageView)
{
    std::vector<VkImageView> allAlbedoTextureViews;
    std::vector<VkImageView> allNormalTextureViews;
    createRayTracingResources(context, commandBufferManager, tlas, models, allAlbedoTextureViews, allNormalTextureViews);
    writeDescriptorSet(context, depthImageView, normalsImageView, albedoImageView, tlas, allAlbedoTextureViews, allNormalTextureViews);
}

void VulkanRayTracingPipeline::handleResize(const VulkanContext& context, uint32_t width, uint32_t height, VkImageView depthImageView, VkImageView normalsImageView, VkImageView albedoImageView)
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
    if (last_storageImageView != VK_NULL_HANDLE)
    {
        vkDestroyImageView(context.device, last_storageImageView, nullptr);
    }
    if (storageImage != VK_NULL_HANDLE)
    {
        vkDestroyImage(context.device, last_storageImage, nullptr);
    }
    if (storageImageMemory != VK_NULL_HANDLE)
    {
        vkFreeMemory(context.device, last_storageImageMemory, nullptr);
    }

    createStorageImage(context, width, height);

    std::vector<VkWriteDescriptorSet> descriptorWrites;

    // Binding 1: Storage image
    VkDescriptorImageInfo storageInfo{};
    storageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
    storageInfo.imageView = storageImageView;
    storageInfo.sampler = VK_NULL_HANDLE;

    VkWriteDescriptorSet storageWrite{};
    storageWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    storageWrite.dstSet = descriptorSet;
    storageWrite.dstBinding = 1;
    storageWrite.dstArrayElement = 0;
    storageWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    storageWrite.descriptorCount = 1;
    storageWrite.pImageInfo = &storageInfo;
    descriptorWrites.push_back(storageWrite);

    // Binding 9: Depth
    VkDescriptorImageInfo depthInfos;
    depthInfos.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
    depthInfos.imageView = depthImageView;
    depthInfos.sampler = globalTextureSampler;

    VkWriteDescriptorSet depthWrite{};
    depthWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    depthWrite.dstSet = descriptorSet;
    depthWrite.dstBinding = 9;
    depthWrite.dstArrayElement = 0;
    depthWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    depthWrite.descriptorCount = 1;
    depthWrite.pImageInfo = &depthInfos;
    descriptorWrites.push_back(depthWrite);

    // Binding 10: Normals
    VkDescriptorImageInfo normalsInfos;
    normalsInfos.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    normalsInfos.imageView = normalsImageView;
    normalsInfos.sampler = globalTextureSampler;

    VkWriteDescriptorSet normalsWrite{};
    normalsWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    normalsWrite.dstSet = descriptorSet;
    normalsWrite.dstBinding = 10;
    normalsWrite.dstArrayElement = 0;
    normalsWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    normalsWrite.descriptorCount = 1;
    normalsWrite.pImageInfo = &normalsInfos;
    descriptorWrites.push_back(normalsWrite);

    // Binding 11: Albedo
    VkDescriptorImageInfo albedoInfos;
    albedoInfos.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    albedoInfos.imageView = albedoImageView;
    albedoInfos.sampler = globalTextureSampler;

    VkWriteDescriptorSet albedoWrite{};
    albedoWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    albedoWrite.dstSet = descriptorSet;
    albedoWrite.dstBinding = 11;
    albedoWrite.dstArrayElement = 0;
    albedoWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    albedoWrite.descriptorCount = 1;
    albedoWrite.pImageInfo = &albedoInfos;
    descriptorWrites.push_back(albedoWrite);

    // Binding 12: Frame accumulation
    VkDescriptorImageInfo lastImageInfos;
    lastImageInfos.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    lastImageInfos.imageView = last_storageImageView;
    lastImageInfos.sampler = globalTextureSampler;

    VkWriteDescriptorSet lastImageWrite{};
    lastImageWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    lastImageWrite.dstSet = descriptorSet;
    lastImageWrite.dstBinding = 12;
    lastImageWrite.dstArrayElement = 0;
    lastImageWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    lastImageWrite.descriptorCount = 1;
    lastImageWrite.pImageInfo = &lastImageInfos;
    descriptorWrites.push_back(lastImageWrite);

    vkUpdateDescriptorSets(context.device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
}

void VulkanRayTracingPipeline::createRayTracingPipelineLayout(const VulkanContext& context)
{
    // Push constants
    VkPushConstantRange pushConstantRange{};
    pushConstantRange.stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR | VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR | VK_SHADER_STAGE_MISS_BIT_KHR;
    pushConstantRange.offset = 0;
    pushConstantRange.size = sizeof(PushConstants);

    VkDescriptorSetLayout layout = DescriptorSetLayoutManager::getRayTracingLayout();
    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &layout;
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
    raygenGroup.generalShader = RT_RAYGEN_SHADER_INDEX;
    raygenGroup.closestHitShader = VK_SHADER_UNUSED_KHR;
    raygenGroup.anyHitShader = VK_SHADER_UNUSED_KHR;
    raygenGroup.intersectionShader = VK_SHADER_UNUSED_KHR;
    shaderGroups.push_back(raygenGroup);

    // Miss group
    VkRayTracingShaderGroupCreateInfoKHR missGroup{};
    missGroup.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
    missGroup.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
    missGroup.generalShader = RT_MISS_SHADER_INDEX;
    missGroup.closestHitShader = VK_SHADER_UNUSED_KHR;
    missGroup.anyHitShader = VK_SHADER_UNUSED_KHR;
    missGroup.intersectionShader = VK_SHADER_UNUSED_KHR;
    shaderGroups.push_back(missGroup);

    // Hit group
    VkRayTracingShaderGroupCreateInfoKHR hitGroup{};
    hitGroup.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
    hitGroup.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_KHR;
    hitGroup.generalShader = VK_SHADER_UNUSED_KHR;
    hitGroup.closestHitShader = RT_CLOSEST_HIT_GENERAL_SHADER_INDEX;
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
    pipelineInfo.maxPipelineRayRecursionDepth = RT_MAX_RECURSION_DEPTH;
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
    VkPhysicalDeviceRayTracingPipelinePropertiesKHR rtProps{};
    rtProps.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_PROPERTIES_KHR;

    VkPhysicalDeviceProperties2 props{};
    props.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
    props.pNext = &rtProps;
    vkGetPhysicalDeviceProperties2(context.physicalDevice, &props);

    uint32_t handleSize = rtProps.shaderGroupHandleSize;
    uint32_t handleSizeAligned = ((handleSize + rtProps.shaderGroupBaseAlignment - 1) / rtProps.shaderGroupBaseAlignment) * rtProps.shaderGroupBaseAlignment;

    uint32_t groupCount = 3; // raygen + miss + hit
    uint32_t sbtSize = groupCount * handleSizeAligned;

    std::vector<uint8_t> shaderHandleStorage(sbtSize);
    if (rt_vkGetRayTracingShaderGroupHandlesKHR(context.device, pipeline, 0, groupCount, sbtSize, shaderHandleStorage.data()) != VK_SUCCESS)
    {
        throw std::runtime_error("Could not get ray tracing shader group handles!");
    }

    VkBufferUsageFlags sbtUsage = VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
    VkMemoryPropertyFlags sbtMemProps = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

    VulkanUtils::Buffers::createBuffer(context, sbtSize, sbtUsage, sbtMemProps, sbtBuffer, sbtBufferMemory, true);

    void* data;
    vkMapMemory(context.device, sbtBufferMemory, 0, sbtSize, 0, &data);

    uint8_t* pData = reinterpret_cast<uint8_t*>(data);
    for (uint32_t g = 0; g < groupCount; g++)
    {
        memcpy(pData, shaderHandleStorage.data() + g * handleSize, handleSize);
        pData += handleSizeAligned;
    }

    vkUnmapMemory(context.device, sbtBufferMemory);

    VkDeviceAddress sbtAddress = VulkanUtils::Buffers::getBufferDeviceAdress(context, sbtBuffer);

    raygenSbtEntry.deviceAddress = sbtAddress;
    raygenSbtEntry.stride = handleSizeAligned;
    raygenSbtEntry.size = handleSizeAligned;

    missSbtEntry.deviceAddress = sbtAddress + handleSizeAligned;
    missSbtEntry.stride = handleSizeAligned;
    missSbtEntry.size = handleSizeAligned;

    hitSbtEntry.deviceAddress = sbtAddress + handleSizeAligned * 2;
    hitSbtEntry.stride = 0; // All geometries use same entry
    hitSbtEntry.size = handleSizeAligned;

    callableSbtEntry = {};
}

void VulkanRayTracingPipeline::createStorageImage(const VulkanContext& context, uint32_t width, uint32_t height)
{
    VkFormat imageFormat = VK_FORMAT_R16G16B16A16_UNORM;
    VkImageUsageFlags imageUsage = VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    VkMemoryPropertyFlags memProps = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

    VulkanUtils::Image::createImage(context, width, height, imageFormat, VK_IMAGE_TILING_OPTIMAL, imageUsage, memProps, storageImage, storageImageMemory);
    storageImageWidth = width;
    storageImageHeight = height;

    storageImageView = VulkanUtils::Image::createImageView(context, storageImage, imageFormat, VK_IMAGE_ASPECT_COLOR_BIT);
    std::cout << "Storage image size: " << width << ", " << height << std::endl;

    // Create last storage image (for frame blending)
    imageFormat = VK_FORMAT_R16G16B16A16_UNORM;
    imageUsage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    memProps = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

    VulkanUtils::Image::createImage(context, width, height, imageFormat, VK_IMAGE_TILING_OPTIMAL, imageUsage, memProps, last_storageImage, last_storageImageMemory);
    last_storageImageView = VulkanUtils::Image::createImageView(context, last_storageImage, imageFormat, VK_IMAGE_ASPECT_COLOR_BIT);
}

void VulkanRayTracingPipeline::createUniformBuffer(const VulkanContext& context)
{
    VkDeviceSize bufferSize = sizeof(SceneData);
    VkBufferUsageFlags usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    VkMemoryPropertyFlags properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

    VulkanUtils::Buffers::createBuffer(context, bufferSize, usage, properties, uniformBuffer, uniformBufferMemory, false);

    vkMapMemory(context.device, uniformBufferMemory, 0, bufferSize, 0, &uniformBufferMapped);
}

void VulkanRayTracingPipeline::updateUniformBuffer(const SceneData& sceneData)
{
    memcpy(uniformBufferMapped, &sceneData, sizeof(sceneData));
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
    poolSizes[3].descriptorCount = 4; // vertex + index + mesh + instance
    poolSizes[4].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSizes[4].descriptorCount = MAX_MESHES * 2 + 3 + 1; // MAX_MESHES * 2 for albedo + normals, +3 for GBuffer, + 1 for last image

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

void VulkanRayTracingPipeline::createRayTracingResources(const VulkanContext& context, VulkanCommandBufferManager& commandBufferManager, VkAccelerationStructureKHR tlas, const std::vector<VulkanModel>& models, std::vector<VkImageView>& outAlbedoTextureViews, std::vector<VkImageView>& outBumpTextureViews)
{
    // Compute sizes across all submeshes
    size_t totalVertices = 0;
    size_t totalIndices = 0;
    size_t totalMeshes = 0;
    size_t totalModels = models.size();

    for (const auto& model : models)
    {
        for (const auto& shadedMesh : model.shadedMeshes)
        {
            totalVertices += shadedMesh.mesh.vertices.size();
            totalIndices += shadedMesh.mesh.indices.size();
            totalMeshes++;
        }
    }

    // Create combined buffers
    std::vector<VulkanVertex> allVertices;
    std::vector<uint32_t> allIndices;

    allVertices.reserve(totalVertices);
    allIndices.reserve(totalIndices);

    std::vector<MeshData> allMeshData;
    std::vector<InstanceData> allInstanceData;

    allMeshData.reserve(totalMeshes);
    allInstanceData.reserve(totalModels);

    uint32_t vertexOffset = 0;
    uint32_t indexOffset = 0;
    uint32_t meshOffset = 0;

    // Process all models and their submeshes
    for (const auto& model : models)
    {
        // Safer to assign attributes explicitly since the struct might change
        InstanceData instanceData;
        instanceData.normalMatrix = glm::transpose(glm::inverse(model.transform.getTransformMatrix()));
        instanceData.meshOffset = meshOffset;
        allInstanceData.push_back(instanceData);

        for (const auto& shadedMesh : model.shadedMeshes)
        {
            const VulkanMesh& mesh = shadedMesh.mesh;

            // Store mesh data
            MeshData meshData;
            meshData.indexOffset = indexOffset;
            meshData.vertexOffset = vertexOffset;
            allMeshData.push_back(meshData);

            // Collect vertex and index data
            allVertices.insert(allVertices.end(), mesh.vertices.begin(), mesh.vertices.end());
            allIndices.insert(allIndices.end(), mesh.indices.begin(), mesh.indices.end());

            // Collect texture from material
            if (!shadedMesh.material.hasError)
            {
                outAlbedoTextureViews.push_back(shadedMesh.material.albedoMap.imageView);
                outBumpTextureViews.push_back(shadedMesh.material.bumpMap.imageView);
            }
            else
            {
                outAlbedoTextureViews.push_back(TextureManager::errorAlbedoTexture.imageView);
                outBumpTextureViews.push_back(TextureManager::errorBumpTexture.imageView);
            }

            // Update offsets
            vertexOffset += static_cast<uint32_t>(mesh.vertices.size());
            indexOffset += static_cast<uint32_t>(mesh.indices.size());
            meshOffset++;
        }

    }

    // Create vertex buffer
    VkBufferUsageFlags vertexUsageFlags = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    VkMemoryPropertyFlags vertexMemoryFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    VulkanUtils::Buffers::createAndFillBuffer<VulkanVertex>(context, commandBufferManager, allVertices, globalVertexBuffer, globalVertexBufferMemory, vertexUsageFlags, vertexMemoryFlags, false);

    // Create index buffer
    VkBufferUsageFlags indexUsageFlags = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    VkMemoryPropertyFlags indexMemoryFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    VulkanUtils::Buffers::createAndFillBuffer<uint32_t>(context, commandBufferManager, allIndices, globalIndexBuffer, globalIndexBufferMemory, indexUsageFlags, indexMemoryFlags, false);

    // Create mesh data buffer
    VkBufferUsageFlags meshDataUsageFlags = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    VkMemoryPropertyFlags meshDataMemoryFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    VulkanUtils::Buffers::createAndFillBuffer<MeshData>(context, commandBufferManager, allMeshData, meshDataBuffer, meshDataBufferMemory, meshDataUsageFlags, meshDataMemoryFlags, false);

    // Create offset buffer
    VkBufferUsageFlags offsetUsage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    VkMemoryPropertyFlags offsetMemory = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    VulkanUtils::Buffers::createAndFillBuffer<InstanceData>(context, commandBufferManager, allInstanceData, instanceDataBuffer, instanceDataBufferMemory, offsetUsage, offsetMemory, false);

    // Create samplers
    VulkanUtils::Textures::createSampler(context, &globalTextureSampler, VK_FILTER_LINEAR, VK_FILTER_LINEAR, VK_SAMPLER_MIPMAP_MODE_LINEAR);
    VulkanUtils::Textures::createSampler(context, &pointTextureSampler, VK_FILTER_NEAREST, VK_FILTER_NEAREST, VK_SAMPLER_MIPMAP_MODE_NEAREST);
}

void VulkanRayTracingPipeline::createDescriptorSet(const VulkanContext& context)
{
    VkDescriptorSetLayout layout = DescriptorSetLayoutManager::getRayTracingLayout();
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = descriptorPool;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &layout;

    if (vkAllocateDescriptorSets(context.device, &allocInfo, &descriptorSet) != VK_SUCCESS)
    {
        throw std::runtime_error("Could not allocate descriptor set!");
    }
}

void VulkanRayTracingPipeline::writeDescriptorSet(const VulkanContext& context, VkImageView depthImageView, VkImageView normalsImageView, VkImageView albedoImageView, VkAccelerationStructureKHR tlas, const std::vector<VkImageView>& albedoTextureViews, const std::vector<VkImageView>& normalTextureViews)
{
    std::vector<VkWriteDescriptorSet> descriptorWrites;

    // TLAS
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

    // Storage Image
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

    // Uniform Buffer
    VkDescriptorBufferInfo uniformBufferInfo{};
    uniformBufferInfo.buffer = uniformBuffer;
    uniformBufferInfo.offset = 0;
    uniformBufferInfo.range = sizeof(SceneData);

    VkWriteDescriptorSet uniformWrite{};
    uniformWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    uniformWrite.dstSet = descriptorSet;
    uniformWrite.dstBinding = 2;
    uniformWrite.dstArrayElement = 0;
    uniformWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uniformWrite.descriptorCount = 1;
    uniformWrite.pBufferInfo = &uniformBufferInfo;
    descriptorWrites.push_back(uniformWrite);

    // Vertex Buffer
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

    // Index Buffer
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

    // Mesh data Buffer
    VkDescriptorBufferInfo meshDataBufferInfo{};
    meshDataBufferInfo.buffer = meshDataBuffer;
    meshDataBufferInfo.offset = 0;
    meshDataBufferInfo.range = VK_WHOLE_SIZE;

    VkWriteDescriptorSet meshDataWrite{};
    meshDataWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    meshDataWrite.dstSet = descriptorSet;
    meshDataWrite.dstBinding = 5;
    meshDataWrite.dstArrayElement = 0;
    meshDataWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    meshDataWrite.descriptorCount = 1;
    meshDataWrite.pBufferInfo = &meshDataBufferInfo;
    descriptorWrites.push_back(meshDataWrite);

    // Instance data Buffer
    VkDescriptorBufferInfo instanceDataBufferInfo{};
    instanceDataBufferInfo.buffer = instanceDataBuffer;
    instanceDataBufferInfo.offset = 0;
    instanceDataBufferInfo.range = VK_WHOLE_SIZE;

    VkWriteDescriptorSet instanceDataWrite{};
    instanceDataWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    instanceDataWrite.dstSet = descriptorSet;
    instanceDataWrite.dstBinding = 6;
    instanceDataWrite.dstArrayElement = 0;
    instanceDataWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    instanceDataWrite.descriptorCount = 1;
    instanceDataWrite.pBufferInfo = &instanceDataBufferInfo;
    descriptorWrites.push_back(instanceDataWrite);

    if (std::max(albedoTextureViews.size(), normalTextureViews.size()) > MAX_MESHES)
    {
        std::cerr << "WARNING: reached maximum mesh count !" << std::endl;
    }

    // Material textures
    // Albedo
    std::vector<VkDescriptorImageInfo> albedoTextureInfos(MAX_MESHES);
    for (size_t i = 0; i < MAX_MESHES; i++)
    {
        albedoTextureInfos[i].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        albedoTextureInfos[i].sampler = globalTextureSampler;
        if (i < albedoTextureViews.size())
        {
            albedoTextureInfos[i].imageView = albedoTextureViews[i];
        }
        else
        {
            // TODO: find better alternative
            albedoTextureInfos[i].imageView = TextureManager::errorAlbedoTexture.imageView;
        }
    }

    // Normals
    std::vector<VkDescriptorImageInfo> normalTextureInfos(MAX_MESHES);
    for (size_t i = 0; i < MAX_MESHES; i++)
    {
        normalTextureInfos[i].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        normalTextureInfos[i].sampler = globalTextureSampler;
        if (i < normalTextureViews.size())
        {
            normalTextureInfos[i].imageView = normalTextureViews[i];
        }
        else
        {
            normalTextureInfos[i].imageView = TextureManager::errorBumpTexture.imageView;
        }
    }

    VkWriteDescriptorSet albedoWrite{};
    albedoWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    albedoWrite.dstSet = descriptorSet;
    albedoWrite.dstBinding = 7;
    albedoWrite.dstArrayElement = 0;
    albedoWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    albedoWrite.descriptorCount = MAX_MESHES;
    albedoWrite.pImageInfo = albedoTextureInfos.data();
    descriptorWrites.push_back(albedoWrite);

    VkWriteDescriptorSet normalWrite{};
    normalWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    normalWrite.dstSet = descriptorSet;
    normalWrite.dstBinding = 8;
    normalWrite.dstArrayElement = 0;
    normalWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    normalWrite.descriptorCount = MAX_MESHES;
    normalWrite.pImageInfo = normalTextureInfos.data();
    descriptorWrites.push_back(normalWrite);

    // Depth
    VkDescriptorImageInfo depthInfos;
    depthInfos.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
    depthInfos.imageView = depthImageView;
    depthInfos.sampler = globalTextureSampler;

    VkWriteDescriptorSet depthWrite{};
    depthWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    depthWrite.dstSet = descriptorSet;
    depthWrite.dstBinding = 9;
    depthWrite.dstArrayElement = 0;
    depthWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    depthWrite.descriptorCount = 1;
    depthWrite.pImageInfo = &depthInfos;

    descriptorWrites.push_back(depthWrite);

    // G-Normals
    VkDescriptorImageInfo normalsInfos;
    normalsInfos.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    normalsInfos.imageView = normalsImageView;
    normalsInfos.sampler = globalTextureSampler;

    VkWriteDescriptorSet gBufferNormalsWrite{};
    gBufferNormalsWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    gBufferNormalsWrite.dstSet = descriptorSet;
    gBufferNormalsWrite.dstBinding = 10;
    gBufferNormalsWrite.dstArrayElement = 0;
    gBufferNormalsWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    gBufferNormalsWrite.descriptorCount = 1;
    gBufferNormalsWrite.pImageInfo = &normalsInfos;

    descriptorWrites.push_back(gBufferNormalsWrite);

    // G-Albedo
    VkDescriptorImageInfo gBufferAlbedoInfos;
    gBufferAlbedoInfos.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    gBufferAlbedoInfos.imageView = albedoImageView;
    gBufferAlbedoInfos.sampler = globalTextureSampler;

    VkWriteDescriptorSet gBufferAlbedoWrite{};
    gBufferAlbedoWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    gBufferAlbedoWrite.dstSet = descriptorSet;
    gBufferAlbedoWrite.dstBinding = 11;
    gBufferAlbedoWrite.dstArrayElement = 0;
    gBufferAlbedoWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    gBufferAlbedoWrite.descriptorCount = 1;
    gBufferAlbedoWrite.pImageInfo = &gBufferAlbedoInfos;

    descriptorWrites.push_back(gBufferAlbedoWrite);

    // Frame accumulation
    VkDescriptorImageInfo lastImageInfos;
    lastImageInfos.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    lastImageInfos.imageView = last_storageImageView;
    lastImageInfos.sampler = pointTextureSampler;

    VkWriteDescriptorSet lastImageWrite{};
    lastImageWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    lastImageWrite.dstSet = descriptorSet;
    lastImageWrite.dstBinding = 12;
    lastImageWrite.dstArrayElement = 0;
    lastImageWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    lastImageWrite.descriptorCount = 1;
    lastImageWrite.pImageInfo = &lastImageInfos;

    descriptorWrites.push_back(lastImageWrite);


    vkUpdateDescriptorSets(context.device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
}

void VulkanRayTracingPipeline::traceRays(VkCommandBuffer commandBuffer, uint32_t frameCount)
{
    sampleCount = RunTimeSettings::spp * frameCount;

    // TODO: use or create VulkanUtils function
    VkImageMemoryBarrier barrier1{};
    barrier1.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier1.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    barrier1.newLayout = VK_IMAGE_LAYOUT_GENERAL;
    barrier1.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier1.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier1.image = storageImage;
    barrier1.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
    barrier1.srcAccessMask = 0;
    barrier1.dstAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
    vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR, 0, 0, nullptr, 0, nullptr, 1, &barrier1);

    VkImageMemoryBarrier barrier2{};
    barrier2.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier2.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    barrier2.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    barrier2.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier2.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier2.image = last_storageImage;
    barrier2.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
    barrier2.srcAccessMask = 0;
    barrier2.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR, 0, 0, nullptr, 0, nullptr, 1, &barrier2);

    // Bind pipeline
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, pipeline);
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, pipelineLayout, 0, 1, &descriptorSet, 0, nullptr);

    PushConstants push;
    push.frameCount = frameCount;
    static std::random_device rd;
    push.rng = rd();

    // Push constants
    vkCmdPushConstants(commandBuffer,
        pipelineLayout,
        VK_SHADER_STAGE_RAYGEN_BIT_KHR | VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR | VK_SHADER_STAGE_MISS_BIT_KHR,
        0,
        sizeof(PushConstants),
        &push);


    if (debug_vkCmdBeginDebugUtilsLabelEXT)
    {
        VkDebugUtilsLabelEXT labelInfo{};
        labelInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT;
        labelInfo.pLabelName = "RTX ON";
        labelInfo.color[0] = 1.0f;
        labelInfo.color[1] = 0.0f;
        labelInfo.color[2] = 0.0f;
        labelInfo.color[3] = 1.0f;
        debug_vkCmdBeginDebugUtilsLabelEXT(commandBuffer, &labelInfo);
    }

    // RAY TRACE !
    rt_vkCmdTraceRaysKHR(commandBuffer, &raygenSbtEntry, &missSbtEntry, &hitSbtEntry, &callableSbtEntry, storageImageWidth, storageImageHeight, RT_MAX_RECURSION_DEPTH);

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
    if (last_storageImageView != VK_NULL_HANDLE)
    {
        vkDestroyImageView(device, last_storageImageView, nullptr);
    }
    if (last_storageImage != VK_NULL_HANDLE)
    {
        vkDestroyImage(device, last_storageImage, nullptr);
    }
    if (last_storageImageMemory != VK_NULL_HANDLE)
    {
        vkFreeMemory(device, last_storageImageMemory, nullptr);
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
    if (meshDataBuffer != VK_NULL_HANDLE)
    {
        vkDestroyBuffer(device, meshDataBuffer, nullptr);
    }
    if (meshDataBufferMemory != VK_NULL_HANDLE)
    {
        vkFreeMemory(device, meshDataBufferMemory, nullptr);
    }
    if (instanceDataBuffer != VK_NULL_HANDLE)
    {
        vkDestroyBuffer(device, instanceDataBuffer, nullptr);
    }
    if (instanceDataBufferMemory != VK_NULL_HANDLE)
    {
        vkFreeMemory(device, instanceDataBufferMemory, nullptr);
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
}

// Getters
VkImage VulkanRayTracingPipeline::getStorageImage() const
{
    return storageImage;
}

VkImage VulkanRayTracingPipeline::getLastStorageImage() const
{
    return last_storageImage;
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