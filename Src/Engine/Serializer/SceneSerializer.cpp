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
	sceneJson["Scene"] = "Untitled"; // TODO: シーン名を入れる
	
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
			eJson["Render"] = {
				{ "ModelPath", rc->GetModelPath() },
				{ "IsDynamic", rc->IsDynamic() }
			};
		}

		// -- Collider (衝突判定) --
		if (entity->HasComponent<ColliderComponent>())
		{
			auto cc = entity->GetComponent<ColliderComponent>();
			eJson["Collider"] = {
				{ "Enable",         cc->IsEnable() },
				{ "DebugDraw",      cc->IsDebugDrawEnabled() },
				{ "CollisionType",  cc->GetCollisionType() },
				// 各形状
				{ "EnableSphere",   cc->GetEnableSphere() },
				{ "SphereRadius",   cc->GetSphereRadius() },
				{ "EnableBox",      cc->GetEnableBox() },
				{ "BoxExtents",     cc->GetBoxExtents() },
				{ "EnableModel",    cc->GetEnableModel() }, // メッシュコライダー
				{ "Offset",         cc->GetOffset() }
			};
		}

		entitiesJson.push_back(eJson);
	}

	sceneJson["Entities"] = entitiesJson;

	// ファイルへの書き込み
	std::filesystem::create_directories(std::filesystem::path(filepath).parent_path());
	std::ofstream o(filepath);
	if (o)
	{
		o << sceneJson.dump(4);
		Logger::Log("Serializer", "Saved Scene to: " + filepath);
	}
	else
	{
		Logger::Error("Failed to save scene: " + filepath);
	}
}

// --- Load Implementation ---
bool SceneSerializer::Load(const std::string& filepath, 
	std::vector<std::shared_ptr<Entity>>& outEntities, 
	std::shared_ptr<CameraBase>& editorCamera)
{
	std::ifstream i(filepath);
	if (!i)
	{
		Logger::Error("Failed to open scene file: " + filepath);
		return false;
	}

	json sceneJson;
	try
	{
		i >> sceneJson;
	}
	catch (json::parse_error& e)
	{
		Logger::Error(std::string("JSON Parse Error: ") + e.what());
		return false;
	}

	// 1. エディタカメラの読み込み
	if (sceneJson.contains("EditorCamera") && editorCamera)
	{
		auto& camJson = sceneJson["EditorCamera"];
		if (camJson.contains("Position")) editorCamera->SetPosition(camJson["Position"]);
		if (camJson.contains("Rotation")) editorCamera->SetEulerDeg(camJson["Rotation"]);
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
				auto tc = std::make_shared<TransformComponent>();
				auto& tJson = eJson["Transform"];
				tc->SetPosition(tJson.value("Position", Math::Vector3::Zero));
				tc->SetRotation(tJson.value("Rotation", Math::Vector3::Zero));
				tc->SetScale(tJson.value("Scale", Math::Vector3::One));
				newEntity->AddComponent(tc);
			}

			// -- Render (描画) --
			if (eJson.contains("Render"))
			{
				auto rc = std::make_shared<RenderComponent>();
				auto& rJson = eJson["Render"];
				
				std::string path = rJson.value("ModelPath", "");
				bool isDynamic = rJson.value("IsDynamic", false);

				if (isDynamic) rc->SetModelWork(path);
				else           rc->SetModelData(path);

				newEntity->AddComponent(rc);
			}

			// -- Collider (衝突判定) --
			if (eJson.contains("Collider"))
			{
				auto cc = std::make_shared<ColliderComponent>();
				auto& cJson = eJson["Collider"];

				cc->SetEnable(cJson.value("Enable", true));
				cc->SetDebugDrawEnabled(cJson.value("DebugDraw", true));
				cc->SetCollisionType(cJson.value("CollisionType", (UINT)KdCollider::TypeBump));
				
				// 各形状
				cc->SetEnableSphere(cJson.value("EnableSphere", false));
				cc->SetSphereRadius(cJson.value("SphereRadius", 1.0f));
				
				cc->SetEnableBox(cJson.value("EnableBox", false));
				cc->SetBoxExtents(cJson.value("BoxExtents", Math::Vector3(0.5f)));

				cc->SetEnableModel(cJson.value("EnableModel", false));
				
				cc->SetOffset(cJson.value("Offset", Math::Vector3::Zero));

				newEntity->AddComponent(cc);
			}

			outEntities.push_back(newEntity);
		}
	}

	Logger::Log("Serializer", "ロード完了: " + filepath);
	return true;
}
