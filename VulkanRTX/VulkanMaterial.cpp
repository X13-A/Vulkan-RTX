#include "VulkanMaterial.hpp"
#include "DescriptorSetLayoutManager.hpp"
#include <stdexcept>
#include "TextureManager.hpp"
#include <iostream>

void VulkanMaterial::init(const PBRMaterialInfo& info, const VulkanContext& context, VulkanCommandBufferManager& commandBufferManager, VkDescriptorPool descriptorPool, bool hasError)
{
    this->hasError = hasError;
    if (!hasError)
    {
        // Use albedo map only if available
        if (!info.albedoTexture.empty())
        {
            albedoTexture.init(info.albedoTexture, context, commandBufferManager);
        }
        // Otherwise create 1x1 texture with appropriate color
        else
        {
            albedoTexture = VulkanTexture::create1x1Texture(info.albedoFactor[0], info.albedoFactor[1], info.albedoFactor[2], context, commandBufferManager);
        }
    }
	createDescriptorSets(context, DescriptorSetLayoutManager::getMaterialLayout(), descriptorPool);
}

VkDescriptorSetLayout VulkanMaterial::createDescriptorSetLayout(const VulkanContext& context)
{
    std::vector<VkDescriptorSetLayoutBinding> bindings;

    VkDescriptorSetLayoutBinding albedoBinding{};
    albedoBinding.binding = 0;
    albedoBinding.descriptorCount = 1;
    albedoBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    albedoBinding.pImmutableSamplers = nullptr;
    albedoBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT |
        VK_SHADER_STAGE_RAYGEN_BIT_KHR |
        VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR |
        VK_SHADER_STAGE_ANY_HIT_BIT_KHR;
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

    VkDescriptorSetLayout descriptorSetLayout;
    if (vkCreateDescriptorSetLayout(context.device, &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create material descriptor set layout!");
    }

    return descriptorSetLayout;
}

void VulkanMaterial::createDescriptorSets(const VulkanContext& context, VkDescriptorSetLayout materialDescriptorSetLayout, VkDescriptorPool descriptorPool)
{
    std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, materialDescriptorSetLayout);

    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = descriptorPool;
    allocInfo.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
    allocInfo.pSetLayouts = layouts.data();

    descriptorSets.resize(MAX_FRAMES_IN_FLIGHT);
    std::cout << "Allocating " << allocInfo.descriptorSetCount << " sets (material)\n";
    if (vkAllocateDescriptorSets(context.device, &allocInfo, descriptorSets.data()) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to allocate material descriptor sets! (material)");
    }

    // Update descriptor sets with texture data
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        VkDescriptorImageInfo imageInfo{};
        imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        if (!hasError)
        {
            imageInfo.imageView = albedoTexture.imageView;
            imageInfo.sampler = albedoTexture.sampler;
        }
        else
        {
            imageInfo.imageView = TextureManager::errorTexture.imageView;
            imageInfo.sampler = TextureManager::errorTexture.sampler;
        }

        VkWriteDescriptorSet albedoWrite{};
        albedoWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        albedoWrite.dstSet = descriptorSets[i];
        albedoWrite.dstBinding = 0;
        albedoWrite.dstArrayElement = 0;
        albedoWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        albedoWrite.descriptorCount = 1;
        albedoWrite.pImageInfo = &imageInfo;

        vkUpdateDescriptorSets(context.device, 1, &albedoWrite, 0, nullptr);
    }
}

void VulkanMaterial::cleanup(VkDevice device)
{
    // Cleanup albedo texture
    albedoTexture.cleanup(device);
    descriptorSets.clear();
}