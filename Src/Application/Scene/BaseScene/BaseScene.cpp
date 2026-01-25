#include "BaseScene.h"
#include "../../../Engine/Scene/SceneManager.h"
#include "../../../Engine/Serializer/SceneSerializer.h"
#include "../../../Engine/ImGui/Editor/EditorManager.h" 

void BaseScene::Init()
{
	if (m_name.empty()) return;

	std::string path = "Asset/Data/Scene/" + m_name + ".json";
	std::vector<std::shared_ptr<Entity>> loadedEntities;
	
	// Dummy camera ptr (SceneSerializer might need one, but for just entities we can pass null or handle it)
	std::shared_ptr<CameraBase> cam = nullptr; 

	if (SceneSerializer::Load(path, loadedEntities, cam))
	{
		for (auto& e : loadedEntities)
		{
			SceneManager::Instance().AddEntity(e);
		}
	}
}

void BaseScene::Release()
{
	
}
