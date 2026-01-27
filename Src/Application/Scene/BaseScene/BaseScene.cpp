#include "BaseScene.h"
#include "../../../Engine/Scene/SceneManager.h"
#include "../../../Engine/Serializer/SceneSerializer.h"
#include "../../../Engine/ImGui/Editor/EditorManager.h" 
#include "../../../Engine/ECS/EntityManager.h"

void BaseScene::Init()
{
	if (m_name.empty()) return;

	std::string path = "Asset/Data/Scene/" + m_name + ".json";
	std::vector<std::shared_ptr<Entity>> loadedEntities;

	std::shared_ptr<CameraBase> cam = nullptr;

	if (SceneSerializer::Load(path, loadedEntities, cam))
	{
		for (auto& entity : loadedEntities)
		{
			EntityManager::Instance().AddEntity(entity);
		}
	}
}

void BaseScene::Release()
{
	
}
