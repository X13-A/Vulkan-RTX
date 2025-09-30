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
            albedoMap.init(info.albedoTexture, context, commandBufferManager);
        }
        // Otherwise create 1x1 texture with appropriate color
        else
        {
            albedoMap = VulkanTexture::create1x1TextureRGBA(static_cast<uint8_t>(info.albedoFactor[0] * 255), static_cast<uint8_t>(info.albedoFactor[1] * 255), static_cast<uint8_t>(info.albedoFactor[2] * 255), context, commandBufferManager);
        }
        if (!info.bumpTexture.empty())
        {
            // TODO: Use last channel for specular or something ?
            bumpMap.init(info.bumpTexture, context, commandBufferManager, VK_FORMAT_R8G8B8A8_UNORM); // Use UNORM for vectors
        }
        else
        {
            bumpMap = VulkanTexture::create1x1TextureRGBA(128, 128, 255, context, commandBufferManager, VK_FORMAT_R8G8B8A8_UNORM);
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
        VkDescriptorImageInfo albedoInfo{};
        albedoInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        VkDescriptorImageInfo bumpInfo{};
        bumpInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        if (!hasError)
        {
            albedoInfo.imageView = albedoMap.imageView;
            albedoInfo.sampler = albedoMap.sampler;

            bumpInfo.imageView = bumpMap.imageView;
            bumpInfo.sampler = bumpMap.sampler;
        }
        else
        {
            albedoInfo.imageView = TextureManager::errorAlbedoTexture.imageView;
            albedoInfo.sampler = TextureManager::errorAlbedoTexture.sampler;

            bumpInfo.imageView = TextureManager::errorBumpTexture.imageView;
            bumpInfo.sampler = TextureManager::errorBumpTexture.sampler;
        }

        std::vector< VkWriteDescriptorSet> descriptorWrites;

        VkWriteDescriptorSet albedoWrite{};
        albedoWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        albedoWrite.dstSet = descriptorSets[i];
        albedoWrite.dstBinding = 0;
        albedoWrite.dstArrayElement = 0;
        albedoWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        albedoWrite.descriptorCount = 1;
        albedoWrite.pImageInfo = &albedoInfo;
        descriptorWrites.push_back(albedoWrite);

        VkWriteDescriptorSet bumpWrite{};
        bumpWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        bumpWrite.dstSet = descriptorSets[i];
        bumpWrite.dstBinding = 1;
        bumpWrite.dstArrayElement = 0;
        bumpWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        bumpWrite.descriptorCount = 1;
        bumpWrite.pImageInfo = &bumpInfo;
        descriptorWrites.push_back(bumpWrite);

        vkUpdateDescriptorSets(context.device, descriptorWrites.size(), descriptorWrites.data(), 0, nullptr);
    }
}

void VulkanMaterial::cleanup(VkDevice device)
{
    // Cleanup albedo texture
    albedoMap.cleanup(device);
    bumpMap.cleanup(device);
    descriptorSets.clear();
}