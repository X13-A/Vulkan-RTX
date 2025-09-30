#include "Scene.hpp"
#include <chrono>
#include "Time.hpp"

const ModelLoadInfo Scene::modelLoadInfos[] =
{
	// FOR DEBUG:
	// arrowGizmo must be the first model
	// the first model is currently used as a gizmo for debug purposes
	{
		"arrowGizmo",
		"models/gizmos/arrow/arrow.obj",
		glm::vec3(0, 0, 0),
		glm::vec3(0.25, 0.25, 0.25),
		glm::vec3(0, 0, 0)
	},
	{
		"atrium",
		"models/Atrium/atrium.obj",
		glm::vec3(0, 0, 0),
		glm::vec3(1, 1, 1),
		glm::vec3(0, 0, 0)
	},
	{
		"brickwall",
		"models/wall/quad.obj",
		glm::vec3(3, 3, -2),
		glm::vec3(5, 5, 5),
		glm::vec3(0, 180, 0)
	},
	{
		"brickwall2",
		"models/wall/quad.obj",
		glm::vec3(3, 3, -1.75),
		glm::vec3(5, 5, 5),
		glm::vec3(0, 0, 0)
	},
	{
		"portal_gun",
		"models/portal_gun_pbr/portal_gun.obj",
		glm::vec3(0, 2.2, 0),
		glm::vec3(5, 5, 5),
		glm::vec3(0, 0, 0)
	},
	//{
	//	"sphere",
	//	"models/sphere/sphere.obj",
	//	glm::vec3(0, 0, 0),
	//	glm::vec3(0.2, 0.2, 0.2),
	//	glm::vec3(0, 0, 0)
	//},
	//{
	//	"sphere",
	//	"models/sphere/sphere.obj",
	//	glm::vec3(2, 0, 0),
	//	glm::vec3(0.2, 0.2, 0.2),
	//	glm::vec3(0, 0, 0)
	//},
	//{
	//	"sphere",
	//	"models/sphere/sphere.obj",
	//	glm::vec3(0, 2, 0),
	//	glm::vec3(0.2, 0.2, 0.2),
	//	glm::vec3(0, 0, 0)
	//},
	//{
	//	"sphere",
	//	"models/sphere/sphere.obj",
	//	glm::vec3(0, 0, 2),
	//	glm::vec3(0.2, 0.2, 0.2),
	//	glm::vec3(0, 0, 0)
	//},
	//{
	//	"ancient-temple-stylized",
	//	"models/ancient-temple-stylized/ancient-temple-stylized.obj",
	//	glm::vec3(5, 0, 0),
	//	glm::vec3(0.25f, 0.25f, 0.25f),
	//	glm::vec3(-36, 90, 0)
	//},
	//{
	//	"sponza",
	//	"models/Sponza/sponza.obj",
	//	glm::vec3(0, 0, 0),
	//	glm::vec3(0.01, 0.01, 0.01),
	//	glm::vec3(0, 0, 0)
	//},
	//{
	//	"San miguel",
	//	"models/San_Miguel/san-miguel-low-poly.obj",
	//	glm::vec3(0, 0, 0),
	//	glm::vec3(1, 1, 1),
	//	glm::vec3(0, 0, 0)
	//},
	//{
	//	"moai",
	//	"models/Moai/moai.obj",
	//	glm::vec3(0, 0.1, 0),
	//	glm::vec3(0.2, 0.2, 0.2),
	//	glm::vec3(0, 0, 0)
	//},
	//{
	//	"portal_gun_glass",
	//	"models/portal_gun/portal_gun_glass.obj",
	//	glm::vec3(0, 0, 0),
	//	glm::vec3(5, 5, 5),
	//	glm::vec3(0, 0, 0)
	//},
	//{
	//	"viling_room",
	//	"models/viking_room/viking_room.obj",
	//	glm::vec3(0, -2, 0),
	//	glm::vec3(5, 5, 5),
	//	glm::vec3(-90, -90, 0)
	//},
	//{
	//	"triangle",
	//	"models/triangle/triangle.obj",
	//	"models/triangle/triangle.png",
	//	glm::vec3(1, 1, 1),
	//	glm::vec3(2, 2, 2),
	//	glm::vec3(3, 3, 3)
	//},
	// TODO: prevent double cleanup of error texture
	//{
	//	"sphere",
	//	"models/sphere/sphere.obj",
	//	glm::vec3(0, 0, 2.5),
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
std::vector<VulkanModel> Scene::models = {};
std::vector<ModelInfo> Scene::modelInfos = {};

uint32_t Scene::getModelCount()
{
	return sizeof(Scene::modelLoadInfos) / sizeof(ModelLoadInfo);
}

uint32_t Scene::getMaterialCount()
{
	uint32_t count = 0;
	for (const ModelInfo& model : modelInfos)
	{
		count += model.materials.size();
	}
	return count;
}

uint32_t Scene::getMeshCount()
{
	uint32_t count = 0;
	for (const ModelInfo& model : modelInfos)
	{
		count += model.meshes.size();
	}
	return count;
}


const std::vector<VulkanModel>& Scene::getModels()
{
	return models;
}

void Scene::fetchModels()
{
	for (const ModelLoadInfo& loadInfo : modelLoadInfos)
	{
		ModelInfo info = ObjLoader::loadObj(loadInfo.objPath);
		modelInfos.push_back(info);
	}
}

void Scene::loadModels(const VulkanContext& context, VulkanCommandBufferManager& commandBufferManager, VkDescriptorPool descriptorPool)
{
	for (int i = 0; i < modelInfos.size(); i++)
	{
		ModelInfo info = modelInfos[i];
		ModelLoadInfo loadInfo = modelLoadInfos[i];

		VulkanModel model;
		model.name = loadInfo.name;
		model.transform.setPosition(loadInfo.position);
		model.transform.setScale(loadInfo.scale);
		model.transform.setRotation(loadInfo.rotation);

		model.load(info, context, commandBufferManager, descriptorPool);
		models.push_back(model);
	}
}

void Scene::update()
{
	// Pass
}

void Scene::cleanup(VkDevice device)
{
	for (VulkanModel& model : models)
	{
		model.cleanup(device);
	}
	models.clear();
}
