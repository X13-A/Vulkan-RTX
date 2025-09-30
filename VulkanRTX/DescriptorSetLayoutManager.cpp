#include "DescriptorSetLayoutManager.hpp"
#include <stdexcept>
#include <array>
#include "Constants.hpp"

VkDescriptorSetLayout DescriptorSetLayoutManager::modelLayout = VK_NULL_HANDLE;
VkDescriptorSetLayout DescriptorSetLayoutManager::materialLayout = VK_NULL_HANDLE;
VkDescriptorSetLayout DescriptorSetLayoutManager::fullScreenQuadLayout = VK_NULL_HANDLE;
VkDescriptorSetLayout DescriptorSetLayoutManager::rayTracingDescriptorSetLayout = VK_NULL_HANDLE;

void DescriptorSetLayoutManager::createLayouts(const VulkanContext& context)
{
    DescriptorSetLayoutManager::createModelLayout(context);
    DescriptorSetLayoutManager::createMaterialLayout(context);
    DescriptorSetLayoutManager::createFullScreenQuadLayout(context);
    DescriptorSetLayoutManager::createRayTracingDescriptorSetLayout(context);
}

void DescriptorSetLayoutManager::createModelLayout(const VulkanContext& context)
{
    VkDescriptorSetLayoutBinding uboLayoutBinding{};
    uboLayoutBinding.binding = 0;
    uboLayoutBinding.descriptorCount = 1;
    uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboLayoutBinding.pImmutableSamplers = nullptr;
    uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

    std::array<VkDescriptorSetLayoutBinding, 1> bindings =
    {
        uboLayoutBinding,
    };

    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
    layoutInfo.pBindings = bindings.data();

    if (vkCreateDescriptorSetLayout(context.device, &layoutInfo, nullptr, &modelLayout) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create geometry pass descriptor set layout!");
    }
}

void DescriptorSetLayoutManager::createMaterialLayout(const VulkanContext& context)
{
    std::vector<VkDescriptorSetLayoutBinding> bindings;

    VkDescriptorSetLayoutBinding albedoBinding{};
    albedoBinding.binding = 0;
    albedoBinding.descriptorCount = 1;
    albedoBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    albedoBinding.pImmutableSamplers = nullptr;
    albedoBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    bindings.push_back(albedoBinding);

    VkDescriptorSetLayoutBinding normalBinding{};
    normalBinding.binding = 1;
    normalBinding.descriptorCount = 1;
    normalBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    normalBinding.pImmutableSamplers = nullptr;
    normalBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    bindings.push_back(normalBinding);

    // TODO:
    // Roughness
    // Metallic
    // etc

    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
    layoutInfo.pBindings = bindings.data();

    if (vkCreateDescriptorSetLayout(context.device, &layoutInfo, nullptr, &materialLayout) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create material descriptor set layout!");
    }
}

void DescriptorSetLayoutManager::createFullScreenQuadLayout(const VulkanContext& context)
{
    VkDescriptorSetLayoutBinding lightingUboLayoutBinding{};
    lightingUboLayoutBinding.binding = 0;
    lightingUboLayoutBinding.descriptorCount = 1;
    lightingUboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    lightingUboLayoutBinding.pImmutableSamplers = nullptr;
    lightingUboLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

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

    VkDescriptorSetLayoutBinding albedoSamplerLayoutBinding{};
    albedoSamplerLayoutBinding.binding = 3;
    albedoSamplerLayoutBinding.descriptorCount = 1;
    albedoSamplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    albedoSamplerLayoutBinding.pImmutableSamplers = nullptr;
    albedoSamplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    std::array<VkDescriptorSetLayoutBinding, 4> lightingBindings =
    {
        lightingUboLayoutBinding,
        depthSamplerLayoutBinding,
        normalSamplerLayoutBinding,
        albedoSamplerLayoutBinding
    };

    VkDescriptorSetLayoutCreateInfo lightingLayoutInfo{};
    lightingLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    lightingLayoutInfo.bindingCount = static_cast<uint32_t>(lightingBindings.size());
    lightingLayoutInfo.pBindings = lightingBindings.data();

    if (vkCreateDescriptorSetLayout(context.device, &lightingLayoutInfo, nullptr, &fullScreenQuadLayout) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create lighting pass descriptor set layout!");
    }
}

void DescriptorSetLayoutManager::createRayTracingDescriptorSetLayout(const VulkanContext& context)
{
    int binding = 0;

    // TLAS
    VkDescriptorSetLayoutBinding tlasBinding{};
    tlasBinding.binding = binding++;
    tlasBinding.descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
    tlasBinding.descriptorCount = 1;
    tlasBinding.stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR | VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
    tlasBinding.pImmutableSamplers = nullptr;

    // Storage Image
    VkDescriptorSetLayoutBinding storageImageBinding{};
    storageImageBinding.binding = binding++;
    storageImageBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    storageImageBinding.descriptorCount = 1;
    storageImageBinding.stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR;
    storageImageBinding.pImmutableSamplers = nullptr;

    // Uniform Buffer
    VkDescriptorSetLayoutBinding uniformBinding{};
    uniformBinding.binding = binding++;
    uniformBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uniformBinding.descriptorCount = 1;
    uniformBinding.stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR | VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR | VK_SHADER_STAGE_MISS_BIT_KHR;
    uniformBinding.pImmutableSamplers = nullptr;

    // Vertex Buffer
    VkDescriptorSetLayoutBinding vertexBufferBinding{};
    vertexBufferBinding.binding = binding++;
    vertexBufferBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    vertexBufferBinding.descriptorCount = 1;
    vertexBufferBinding.stageFlags = VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
    vertexBufferBinding.pImmutableSamplers = nullptr;

    // Index Buffer
    VkDescriptorSetLayoutBinding indexBufferBinding{};
    indexBufferBinding.binding = binding++;
    indexBufferBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    indexBufferBinding.descriptorCount = 1;
    indexBufferBinding.stageFlags = VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
    indexBufferBinding.pImmutableSamplers = nullptr;

    // Mesh Data Buffer
    VkDescriptorSetLayoutBinding meshDataBufferBinding{};
    meshDataBufferBinding.binding = binding++;
    meshDataBufferBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    meshDataBufferBinding.descriptorCount = 1;
    meshDataBufferBinding.stageFlags = VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
    meshDataBufferBinding.pImmutableSamplers = nullptr;

    // Instance Data Buffer
    VkDescriptorSetLayoutBinding instanceDataBufferBinding{};
    instanceDataBufferBinding.binding = binding++;
    instanceDataBufferBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    instanceDataBufferBinding.descriptorCount = 1;
    instanceDataBufferBinding.stageFlags = VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
    instanceDataBufferBinding.pImmutableSamplers = nullptr;

    // Textures array
    VkDescriptorSetLayoutBinding instancesAlbedoBinding{};
    instancesAlbedoBinding.binding = binding++;
    instancesAlbedoBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    instancesAlbedoBinding.descriptorCount = MAX_MESHES;
    instancesAlbedoBinding.stageFlags = VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
    instancesAlbedoBinding.pImmutableSamplers = nullptr;

    // Textures array
    VkDescriptorSetLayoutBinding instancesNormalBinding{};
    instancesNormalBinding.binding = binding++;
    instancesNormalBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    instancesNormalBinding.descriptorCount = MAX_MESHES;
    instancesNormalBinding.stageFlags = VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
    instancesNormalBinding.pImmutableSamplers = nullptr;

    // GBuffer
    VkDescriptorSetLayoutBinding depthBinding{};
    depthBinding.binding = binding++;
    depthBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    depthBinding.descriptorCount = 1;
    depthBinding.stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR;
    depthBinding.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutBinding normalsBinding{};
    normalsBinding.binding = binding++;
    normalsBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    normalsBinding.descriptorCount = 1;
    normalsBinding.stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR;
    normalsBinding.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutBinding albedoBinding{};
    albedoBinding.binding = binding++;
    albedoBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    albedoBinding.descriptorCount = 1;
    albedoBinding.stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR;
    albedoBinding.pImmutableSamplers = nullptr;

    // Frame accumulation
    VkDescriptorSetLayoutBinding lastImageBinding{};
    lastImageBinding.binding = binding++;
    lastImageBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    lastImageBinding.descriptorCount = 1;
    lastImageBinding.stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR;
    lastImageBinding.pImmutableSamplers = nullptr;

    std::array<VkDescriptorSetLayoutBinding, 13> bindings =
    {
        tlasBinding,
        storageImageBinding,
        uniformBinding,
        vertexBufferBinding,
        indexBufferBinding,
        meshDataBufferBinding,
        instanceDataBufferBinding,
        instancesAlbedoBinding,
        instancesNormalBinding,
        depthBinding,
        normalsBinding,
        albedoBinding,
        lastImageBinding
    };

    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
    layoutInfo.pBindings = bindings.data();

    if (vkCreateDescriptorSetLayout(context.device, &layoutInfo, nullptr, &rayTracingDescriptorSetLayout) != VK_SUCCESS)
    {
        throw std::runtime_error("Could not create descriptor set layout (ray tracing)!");
    }
}

VkDescriptorSetLayout DescriptorSetLayoutManager::getModelLayout()
{
    if (modelLayout == VK_NULL_HANDLE)
    {
        throw std::runtime_error("Descriptor set layout not initialized !");
    }
    return modelLayout;
}

VkDescriptorSetLayout DescriptorSetLayoutManager::getMaterialLayout()
{
    if (materialLayout == VK_NULL_HANDLE)
    {
        throw std::runtime_error("Descriptor set layout not initialized !");
    }
    return materialLayout;
}

VkDescriptorSetLayout DescriptorSetLayoutManager::getFullScreenQuadLayout()
{
    if (fullScreenQuadLayout == VK_NULL_HANDLE)
    {
        throw std::runtime_error("Descriptor set layout not initialized !");
    }
    return fullScreenQuadLayout;
}

VkDescriptorSetLayout DescriptorSetLayoutManager::getRayTracingLayout()
{
    if (rayTracingDescriptorSetLayout == VK_NULL_HANDLE)
    {
        throw std::runtime_error("Descriptor set layout not initialized !");
    }
    return rayTracingDescriptorSetLayout;
}

void DescriptorSetLayoutManager::cleanup(VkDevice device)
{
    if (modelLayout != VK_NULL_HANDLE)
    {
        vkDestroyDescriptorSetLayout(device, modelLayout, nullptr);
    }
    if (materialLayout != VK_NULL_HANDLE)
    {
        vkDestroyDescriptorSetLayout(device, materialLayout, nullptr);
    }
    if (fullScreenQuadLayout != VK_NULL_HANDLE)
    {
        vkDestroyDescriptorSetLayout(device, fullScreenQuadLayout, nullptr);
    }
    if (rayTracingDescriptorSetLayout != VK_NULL_HANDLE)
    {
        vkDestroyDescriptorSetLayout(device, rayTracingDescriptorSetLayout, nullptr);
    }
}