#include "DescriptorSetLayoutManager.hpp"
#include <stdexcept>
#include <array>

VkDescriptorSetLayout DescriptorSetLayoutManager::modelLayout = VK_NULL_HANDLE;
VkDescriptorSetLayout DescriptorSetLayoutManager::materialLayout = VK_NULL_HANDLE;
VkDescriptorSetLayout DescriptorSetLayoutManager::fullScreenQuadLayout = VK_NULL_HANDLE;

void DescriptorSetLayoutManager::createModelLayout(const VulkanContext& context)
{
    VkDescriptorSetLayoutBinding uboLayoutBinding{};
    uboLayoutBinding.binding = 0;
    uboLayoutBinding.descriptorCount = 1;
    uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboLayoutBinding.pImmutableSamplers = nullptr;
    uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

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

    // TODO:
    // Normal
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
}