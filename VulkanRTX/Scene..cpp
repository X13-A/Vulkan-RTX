#include "Scene.hpp"
#include <chrono>
#include "Time.hpp"

const ModelLoadInfo Scene::modelLoadInfos[] =
{
	{
		"portal_gun",
		"models/portal_gun/portal_gun.obj",
		"models/portal_gun/PortalGun_Albedo.png"
	},
	{
		"portal_gun_glass",
		"models/portal_gun/portal_gun_glass.obj",
		"textures/white.jpg"
	},
	{
		"viling_room",
		"models/viking_room/viking_room.obj",
		"models/viking_room/viking_room.png"
	}
};
std::vector<VulkanModel> Scene::loadedModels = {};


uint32_t Scene::getModelCount()
{
	return sizeof(Scene::modelLoadInfos) / sizeof(ModelLoadInfo);
}

const std::vector<VulkanModel>& Scene::getModels()
{
	return loadedModels;
}

void Scene::loadModels(const VulkanContext& context, VulkanCommandBufferManager& commandBufferManager, VkDescriptorSetLayout geometryDescriptorSetLayout, VkDescriptorPool descriptorPool)
{
	for (ModelLoadInfo loadInfo : modelLoadInfos)
	{
		VulkanModel model;
		model.name = loadInfo.name;
		model.init(loadInfo.objPath, loadInfo.texturePath, context, commandBufferManager, geometryDescriptorSetLayout, descriptorPool);
		loadedModels.push_back(model);
	}
}

void Scene::update()
{
	static auto startTime = std::chrono::high_resolution_clock::now();
	auto currentTime = std::chrono::high_resolution_clock::now();
	float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

	// Portal gun
	float scale = 5.0f;
	for (int i = 0; i < 2; i++)
	{
		loadedModels[i].transform.setTransformMatrix(glm::mat4(1.0f));
		loadedModels[i].transform.scale(glm::vec3(scale, scale, scale));
		loadedModels[i].transform.setRotation(glm::vec3(0, Time::time() * 45.0, 0));
	}

	// Viking room
	scale = 5.0f;
	loadedModels[2].transform.setTransformMatrix(glm::mat4(1.0f));
	loadedModels[2].transform.scale(glm::vec3(scale, scale, scale));
	loadedModels[2].transform.translate(glm::vec3(0, -2, 0));
	loadedModels[2].transform.rotate(glm::vec3(-90, -90, 0));
}

void Scene::cleanup(VkDevice device)
{
	for (VulkanModel& model : loadedModels)
	{
		model.cleanup(device);
	}
	loadedModels.clear();
}
