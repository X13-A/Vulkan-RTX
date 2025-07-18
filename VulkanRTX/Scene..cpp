#include "Scene.hpp"
#include <chrono>
#include "Time.hpp"

const ModelLoadInfo Scene::modelLoadInfos[] =
{
	{
		"portal_gun",
		"models/portal_gun/portal_gun.obj",
		"models/portal_gun/PortalGun_Albedo.png",
		glm::vec3(0, 0, 0),
		glm::vec3(5, 5, 5),
		glm::vec3(0, 0, 0)
	},
	{
		"portal_gun_glass",
		"models/portal_gun/portal_gun_glass.obj",
		"textures/white.jpg",
		glm::vec3(0, 0, 0),
		glm::vec3(5, 5, 5),
		glm::vec3(0, 0, 0)
	},
	{
		"viling_room",
		"models/viking_room/viking_room.obj",
		"models/viking_room/viking_room.png",
		glm::vec3(0, -2, 0),
		glm::vec3(5, 5, 5),
		glm::vec3(-90, -90, 0)
	},
	//{
	//	"triangle",
	//	"models/triangle/triangle.obj",
	//	"models/triangle/triangle.png",
	//	glm::vec3(1, 1, 1),
	//	glm::vec3(2, 2, 2),
	//	glm::vec3(3, 3, 3)
	//},
	//{
	//	"sphere",
	//	"models/sphere/sphere.obj",
	//	"textures/white.jpg",
	//	glm::vec3(2.5, 0, 2.5),
	//	glm::vec3(1, 1, 1),
	//	glm::vec3(0, 0, 0)
	//},
	//{
	//	"triangle",
	//	"models/triangle/triangle.obj",
	//	"models/portal_gun/portal_gun_glass.png",
	//	glm::vec3(0, 0, 15),
	//	glm::vec3(5, 5, 5),
	//	glm::vec3(6, 6, 6)
	//}
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
		
		model.transform.setPosition(loadInfo.position);
		model.transform.setScale(loadInfo.scale);
		model.transform.setRotation(loadInfo.rotation);

		model.init(loadInfo.objPath, loadInfo.texturePath, context, commandBufferManager, geometryDescriptorSetLayout, descriptorPool);
		loadedModels.push_back(model);
	}
}

void Scene::update()
{
	// Pass
}

void Scene::cleanup(VkDevice device)
{
	for (VulkanModel& model : loadedModels)
	{
		model.cleanup(device);
	}
	loadedModels.clear();
}
