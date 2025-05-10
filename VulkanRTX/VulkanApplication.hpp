#include "VulkanContext.hpp"
#include "VulkanSwapChainManager.hpp"
#include "VulkanGraphicsPipeline.hpp"
#include "VulkanModel.hpp"
#include "VulkanRenderer.hpp"
#include "WindowManager.hpp"
#include "InputManager.hpp"
#include <chrono>
#include "entt.hpp"
#include "Camera.hpp"
#include "AllEvents.hpp"
#include "CreativeControls.hpp"

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
    VulkanGraphicsPipeline graphicsPipeline;
    std::vector<VulkanModel> models;
    VulkanFullScreenQuad fullScreenQuad;

    VulkanRenderer renderer;

    std::chrono::time_point<std::chrono::high_resolution_clock> lastTime;
    int frameCount = 0;

public:
    void run();

    void handleResize();

    bool shouldTerminate() const;
private:
    void initVulkan();

    void handleWindowResize(const WindowResizeEvent& e);

    void updateScene();

    void mainLoop();

    void updateFPS();

    void cleanup();
};
