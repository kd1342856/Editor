#include "SceneSerializer.h"
#include "../../Application/GameObject/Camera/CameraBase.h"

using json = nlohmann::json;
namespace DirectX 
{
	namespace SimpleMath 
	{
		void to_json(json& j, const Vector3& v) 
		{
			j = json{ v.x, v.y, v.z };
		}
		void from_json(const json& j, Vector3& v) 
		{
			if (j.is_array() && j.size() >= 3) 
			{
				v.x = j[0]; v.y = j[1]; v.z = j[2];
			}
			else {
				v = Vector3::Zero;
			}
		}
	}
}

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

		// -- Transform (トランスフォーム) --
		if (entity->HasComponent<TransformComponent>())
		{
			auto tc = entity->GetComponent<TransformComponent>();
			eJson["Transform"] = {
				{ "Position", tc->GetPosition() },
				{ "Rotation", tc->GetRotation() },
				{ "Scale",    tc->GetScale() }
			};
		}

		// -- Render (描画) --
		if (entity->HasComponent<RenderComponent>())
		{
			auto rc = entity->GetComponent<RenderComponent>();
			eJson["Render"] = 
			{
				{ "ModelPath", rc->GetModelPath() },
				{ "IsDynamic", rc->IsDynamic() }
			};
		}

		// -- Collider (衝突判定) --
		if (entity->HasComponent<ColliderComponent>())
		{
			auto collider = entity->GetComponent<ColliderComponent>();
			eJson["Collider"] = 
			{
				{ "Enable",         collider->IsEnable() },
				{ "DebugDraw",      collider->IsDebugDrawEnabled() },
				{ "CollisionType",  collider->GetCollisionType() },
				// 各形状
				{ "EnableSphere",   collider->GetEnableSphere() },
				{ "SphereRadius",   collider->GetSphereRadius() },
				{ "EnableBox",      collider->GetEnableBox() },
				{ "BoxExtents",     collider->GetBoxExtents() },
				{ "EnableModel",    collider->GetEnableModel() }, // メッシュコライダー
				{ "Offset",         collider->GetOffset() }
			};
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
			newEntity->Init(); // 基本初期化

			// 名前
			if (eJson.contains("Name")) newEntity->SetName(eJson["Name"]);

			// -- Transform (トランスフォーム) --
			if (eJson.contains("Transform"))
			{
				auto trans	= std::make_shared<TransformComponent>();
				auto& tJson = eJson["Transform"];
				trans->SetPosition	(tJson.value("Position"	, Math::Vector3::Zero));
				trans->SetRotation	(tJson.value("Rotation"	, Math::Vector3::Zero));
				trans->SetScale		(tJson.value("Scale"	, Math::Vector3::One));
				newEntity->AddComponent(trans);
			}

			// -- Render (描画) --
			if (eJson.contains("Render"))
			{
				auto render = std::make_shared<RenderComponent>();
				auto& rJson = eJson["Render"];
				
				std::string path	= rJson.value("ModelPath", "");
				bool isDynamic		= rJson.value("IsDynamic", false);

				if (isDynamic)
				{
					render->SetModelWork(path);
				}
				else
				{
					render->SetModelData(path);
				}
				newEntity->AddComponent(render);
			}

			// -- Collider (衝突判定) --
			if (eJson.contains("Collider"))
			{
				auto collider = std::make_shared<ColliderComponent>();
				auto& cJson = eJson["Collider"];

				collider->SetEnable(cJson.value("Enable", true));
				collider->SetDebugDrawEnabled(cJson.value("DebugDraw", true));
				collider->SetCollisionType(cJson.value("CollisionType", (UINT)KdCollider::TypeBump));
				
				// 各形状
				collider->SetEnableSphere(cJson.value("EnableSphere", false));
				collider->SetSphereRadius(cJson.value("SphereRadius", 1.0f));
				
				collider->SetEnableBox(cJson.value("EnableBox", false));
				collider->SetBoxExtents(cJson.value("BoxExtents", Math::Vector3(0.5f)));

				collider->SetEnableModel(cJson.value("EnableModel", false));
				
				collider->SetOffset(cJson.value("Offset", Math::Vector3::Zero));

				newEntity->AddComponent(collider);
			}

			outEntities.push_back(newEntity);
		}
	}

	Logger::Log("Serializer", "ロード完了: " + filepath);
	return true;
}
