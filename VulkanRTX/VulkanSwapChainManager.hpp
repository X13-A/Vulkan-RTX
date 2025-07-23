#pragma once

#include "Constants.hpp"
#include <vector>
#include "VulkanUtils.hpp"
#include "VulkanContext.hpp"
#include "VulkanCommandBufferManager.hpp"
#include "Vulkan_GLFW.hpp"

struct SwapChainSupportDetails
{
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

class VulkanSwapChainManager
{
public:
    VkSwapchainKHR swapChain;
    std::vector<VkImage> swapChainImages;
    VkFormat swapChainImageFormat;
    VkExtent2D swapChainExtent;
    std::vector<VkImageView> swapChainImageViews;
    std::vector<VkFramebuffer> swapChainFramebuffers;

    bool framebufferResized = false;

public:
    static SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface);
    static VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
    static VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
    static VkExtent2D chooseSwapExtent(int width, int height, const VkSurfaceCapabilitiesKHR& capabilities);

    void init(int width, int height, const VulkanContext& context);
    void createSwapChain(int width, int height, const VulkanContext& context);
    void createImageViews(const VulkanContext& context);
    void cleanup(VkDevice device);
    void createFramebuffers(const VulkanContext& context, VkRenderPass renderPass);
    void handleResize(int width, int height, const VulkanContext& context, VulkanCommandBufferManager& commandBufferManager, VkRenderPass renderPass);
};
