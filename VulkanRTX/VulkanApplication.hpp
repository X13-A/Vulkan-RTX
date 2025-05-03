#include "VulkanContext.hpp"
#include "VulkanSwapChainManager.hpp"
#include "VulkanGraphicsPipeline.hpp"
#include "VulkanModel.hpp"
#include "VulkanRenderer.hpp"
#include "WindowManager.hpp"

#include <chrono>

class VulkanApplication
{
private:
    WindowManager windowManager;

    VulkanContext context;
    VulkanSwapChainManager swapChainManager;
    VulkanCommandBufferManager commandBufferManager;
    VulkanGraphicsPipeline graphicsPipeline;
    std::vector<VulkanModel> models;
    VulkanFullScreenQuad fullScreenQuad;

    VulkanRenderer renderer;

    std::chrono::time_point<std::chrono::high_resolution_clock> lastTime;
    int frameCount = 0;

public:
    void run();

    void handleResize();

private:
    void initVulkan();

    void updateScene();

    void mainLoop();

    void updateFPS();

    void cleanup();
};
