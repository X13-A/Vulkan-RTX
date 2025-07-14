#include "VulkanContext.hpp"
#include "VulkanModel.hpp"

// Structure pour les données caméra (doit correspondre au shader)
struct CameraData 
{
    glm::mat4 view;
    glm::mat4 proj;
    glm::mat4 viewInverse;
    glm::mat4 projInverse;
};

class VulkanRayTracingPipeline
{
private:
    // Pipeline
    VkPipeline pipeline;
    VkPipelineLayout pipelineLayout;
    VkDescriptorSetLayout descriptorSetLayout;

    // Shader binding table
    VkBuffer sbtBuffer;
    VkDeviceMemory sbtBufferMemory;
    VkStridedDeviceAddressRegionKHR raygenSbtEntry{};
    VkStridedDeviceAddressRegionKHR missSbtEntry{};
    VkStridedDeviceAddressRegionKHR hitSbtEntry{};
    VkStridedDeviceAddressRegionKHR callableSbtEntry{};

    // Storage Image
    VkImage storageImage;
    VkDeviceMemory storageImageMemory;
    VkImageView storageImageView;
    uint32_t storageImageWidth;
    uint32_t storageImageHeight;

    // Uniform Buffer
    VkBuffer uniformBuffer;
    VkDeviceMemory uniformBufferMemory;
    void* uniformBufferMapped;

    // Descriptor Set
    VkDescriptorPool descriptorPool;
    VkDescriptorSet descriptorSet;

    // buffers
    VkBuffer globalVertexBuffer;
    VkDeviceMemory globalVertexBufferMemory;

    VkBuffer globalIndexBuffer;
    VkDeviceMemory globalIndexBufferMemory;

    // Sampler
    VkSampler globalTextureSampler;

public:
    void init(const VulkanContext& context, uint32_t width, uint32_t height);
    void setupScene(const VulkanContext& context, VulkanCommandBufferManager& commandBufferManager, VkAccelerationStructureKHR tlas, const std::vector<VulkanModel>& models);

    void createRayTracingDescriptorSetLayout(const VulkanContext& context);
    void createRayTracingPipelineLayout(const VulkanContext& context);
    void createRayTracingPipeline(const VulkanContext& context);
    void createShaderBindingTable(const VulkanContext& context);
    
    void createRayTracingResources(const VulkanContext& context, VulkanCommandBufferManager& commandBufferManager, VkAccelerationStructureKHR tlas, const std::vector<VulkanModel>& models, std::vector<VkImageView>& outTextureViews);

    void createDescriptorPool(const VulkanContext& context);
    void createDescriptorSet(const VulkanContext& context);
    void writeDescriptorSet(const VulkanContext& context, VkAccelerationStructureKHR tlas, const std::vector<VkImageView>& textureViews);
    
    void createStorageImage(const VulkanContext& context, uint32_t width, uint32_t height);
    void createUniformBuffer(const VulkanContext& context);
    void updateUniformBuffer(const CameraData& cameraData);

    void traceRays(VkCommandBuffer commandBuffer, uint32_t frameCount);
    void handleResize(const VulkanContext& context, uint32_t width, uint32_t height);
    void cleanup(VkDevice device);

    // Getters
    VkImage getStorageImage() const;
    VkDescriptorSet getDescriptorSet() const;
    VkPipelineLayout getPipelineLayout() const;
    uint32_t getStorageImageWidth() const;
    uint32_t getStorageImageHeight() const;
};