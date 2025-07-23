#include "VulkanContext.hpp"
#include "VulkanModel.hpp"

struct SceneData 
{
    glm::mat4 view;
    glm::mat4 proj;
    glm::mat4 viewInverse;
    glm::mat4 projInverse;
    glm::vec3 cameraPos;
    uint32_t recursionDepth;
    glm::vec2 nearFar;
    int spp;

};

struct InstanceData
{
    glm::mat4 normalMatrix;
    uint32_t meshOffset;
    glm::vec3 padding;
};

struct MeshData
{
    uint32_t indexOffset;
    uint32_t vertexOffset;
};

struct PushConstants
{
    uint32_t frameCount;
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

    VkImage last_storageImage;
    VkDeviceMemory last_storageImageMemory;
    VkImageView last_storageImageView;

    // Uniform Buffer
    VkBuffer uniformBuffer;
    VkDeviceMemory uniformBufferMemory;
    void* uniformBufferMapped;

    // Descriptor Set
    VkDescriptorPool descriptorPool;
    VkDescriptorSet descriptorSet;

    // buffers
    VkBuffer instanceDataBuffer;
    VkDeviceMemory instanceDataBufferMemory;

    VkBuffer globalIndexBuffer;
    VkDeviceMemory globalIndexBufferMemory;

    VkBuffer meshDataBuffer;
    VkDeviceMemory meshDataBufferMemory;

    VkBuffer globalVertexBuffer;
    VkDeviceMemory globalVertexBufferMemory;


    // Sampler
    VkSampler globalTextureSampler;

public:
    void init(const VulkanContext& context, uint32_t width, uint32_t height);
    void writeDescriptors(const VulkanContext& context, VulkanCommandBufferManager& commandBufferManager, const std::vector<VulkanModel>& models, VkAccelerationStructureKHR tlas, VkImageView depthImageView, VkImageView normalsImageView, VkImageView albedoImageView);

    void createRayTracingDescriptorSetLayout(const VulkanContext& context);
    void createRayTracingPipelineLayout(const VulkanContext& context);
    void createRayTracingPipeline(const VulkanContext& context);
    void createShaderBindingTable(const VulkanContext& context);
    
    void createRayTracingResources(const VulkanContext& context, VulkanCommandBufferManager& commandBufferManager, VkAccelerationStructureKHR tlas, const std::vector<VulkanModel>& models, std::vector<VkImageView>& outTextureViews);

    void createDescriptorPool(const VulkanContext& context);
    void createDescriptorSet(const VulkanContext& context);
    void writeDescriptorSet(const VulkanContext& context, VkImageView depthImageView, VkImageView normalsImageView, VkImageView albedoImageView, VkAccelerationStructureKHR tlas, const std::vector<VkImageView>& textureViews);
    
    void createStorageImage(const VulkanContext& context, uint32_t width, uint32_t height);
    void createUniformBuffer(const VulkanContext& context);
    void updateUniformBuffer(const SceneData& sceneData);

    void traceRays(VkCommandBuffer commandBuffer, uint32_t frameCount);
    void handleResize(const VulkanContext& context, uint32_t width, uint32_t height, VkImageView depthImageView, VkImageView normalsImageView, VkImageView albedoImageView);
    void cleanup(VkDevice device);

    // Getters
    VkImage getStorageImage() const;
    VkImage getLastStorageImage() const;
    VkDescriptorSet getDescriptorSet() const;
    VkPipelineLayout getPipelineLayout() const;
    uint32_t getStorageImageWidth() const;
    uint32_t getStorageImageHeight() const;
};