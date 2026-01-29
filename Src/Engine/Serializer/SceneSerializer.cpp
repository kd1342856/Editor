#include "SceneSerializer.h"
#include "../../Application/GameObject/Camera/CameraBase.h"
#include "JsonUtils.h"
#include "../ECS/Component/Factory/ComponentFactory.h"

// We don't need individual component headers anymore for loading!
// But we might need them if we want to explicitly use them for something else, 
// though the goal is to be generic.

using json = nlohmann::json;

// --- Save Implementation ---
void SceneSerializer::Save(const std::string& filepath, 
	const std::vector<std::shared_ptr<Entity>>& entities, 
	const std::shared_ptr<CameraBase>& editorCamera)
{
	json sceneJson;
	sceneJson["Scene"] = "Untitled";
	
	// 1. エディタカメラの保存 (オプション)
	if (editorCamera)
	{
		json camJson;
		camJson["Position"] = editorCamera->GetPosition();
		camJson["Rotation"] = editorCamera->GetEulerDeg();
		sceneJson["EditorCamera"] = camJson;
	}

	// 2. エンティティの保存
	json entitiesJson = json::array();

	for (const auto& entity : entities)
	{
		if (!entity) continue;

		json eJson;
		eJson["Name"] = entity->GetName();

		// Update: Fully Dynamic Serialization
		// Iterate over all components in the entity
		for (const auto& [typeIdx, component] : entity->GetAllComponents())
		{
			// Get the type string (key) from the component itself
			std::string typeName = component->GetType();
			
			json compJson;
			component->Serialize(compJson);
			
			eJson[typeName] = compJson;
		}

		entitiesJson.push_back(eJson);
	}

	sceneJson["Entities"] = entitiesJson;

	// ファイルへの書き込み
	std::filesystem::create_directories(std::filesystem::path(filepath).parent_path());
	std::ofstream os(filepath);
	if (os)
	{
		os << sceneJson.dump(4);
		Logger::Log("Serializer", "Saved Scene to: " + filepath);
	}
	else
	{
		Logger::Error("Failed to save scene: " + filepath);
	}
}

// --- Load Implementation ---
bool SceneSerializer::Load(const std::string& filepath, 
	std::vector	<std::shared_ptr<Entity>>	& outEntities, 
	std::shared_ptr<CameraBase>				& editorCamera)
{
	std::ifstream is(filepath);
	if (!is)
	{
		Logger::Error("Failed to open scene file: " + filepath);
		return false;
	}

	json sceneJson;
	try
	{
		is >> sceneJson;
	}
	catch (json::parse_error& error)
	{
		Logger::Error(std::string("JSON Parse Error: ") + error.what());
		return false;
	}

	// 1. エディタカメラの読み込み
	if (sceneJson.contains("EditorCamera") && editorCamera)
	{
		auto& camJson = sceneJson["EditorCamera"];
		if (camJson.contains("Position"))
		{
			editorCamera->SetPosition(camJson["Position"]);
		}
		if (camJson.contains("Rotation")) 
		{
			editorCamera->SetEulerDeg(camJson["Rotation"]);
		}
	}

	// 2. エンティティの読み込み
	outEntities.clear(); // 現在のシーンをクリア

	if (sceneJson.contains("Entities"))
	{
		for (auto& eJson : sceneJson["Entities"])
		{
			std::shared_ptr<Entity> newEntity = std::make_shared<Entity>();

			// 名前
			if (eJson.contains("Name")) newEntity->SetName(eJson["Name"]);

			// Update: Fully Dynamic Deserialization
			// Iterate over all JSON keys in the entity object
			for (auto& [key, value] : eJson.items())
			{
				if (key == "Name") continue; // Skip name

				// Try to create a component with this key
				auto component = ComponentFactory::Instance().Create(key);
				if (component)
				{
					try 
					{
						component->Deserialize(value);
						newEntity->AddComponent(component);
					}
					catch(const std::exception& e)
					{
						Logger::Error(std::string("Failed to deserialize component: ") + key + " Error: " + e.what());
					}
				}
				else
				{
					// Unknown component or just extra data, ignore or log warning
					// Logger::Warning("Unknown component type: " + key);
				}
			}

			outEntities.push_back(newEntity);
		}
	}

	Logger::Log("Serializer", "ロード完了: " + filepath);
	return true;
}
