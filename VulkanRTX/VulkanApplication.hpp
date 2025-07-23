#include "VulkanContext.hpp"
#include "VulkanSwapChainManager.hpp"
#include "VulkanGraphicsPipelineManager.hpp"
#include "VulkanModel.hpp"
#include "VulkanRenderer.hpp"
#include "WindowManager.hpp"
#include "InputManager.hpp"
#include <chrono>
#include "entt.hpp"
#include "Camera.hpp"
#include "AllEvents.hpp"
#include "CreativeControls.hpp"
#include "VulkanTLAS.hpp"


class VulkanApplication
{
private:
    entt::dispatcher dispatcher;
    WindowManager windowManager;
    InputManager inputManager;
    Camera camera;
    CreativeControls* controls;

    VulkanContext context;
    VulkanSwapChainManager swapChainManager;
    VulkanCommandBufferManager commandBufferManager;
    VulkanGraphicsPipelineManager graphicsPipelineManager;
    
    VulkanFullScreenQuad fullScreenQuad;
    VulkanTLAS sceneTLAS;
    std::vector<BLASInstance> BLASintances;

    VulkanRenderer renderer;

    std::chrono::time_point<std::chrono::high_resolution_clock> lastTime;

    int nativeWidth, nativeHeight;
    int scaledWidth, scaledHeight;

public:
    void run();

    void handleResize();

    bool shouldTerminate() const;

    void handleInputs();

private:
    void initVulkan();

    void handleWindowResize(const WindowResizeEvent& e);

    void mainLoop();

    void updateFPS();

    void cleanup();
};
