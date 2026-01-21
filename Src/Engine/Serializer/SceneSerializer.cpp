#include "SceneSerializer.h"
#include "../Data/Loader/ObjData/ObjData.h"
#include "../ImGui/Editor/EditorManager.h" 
#include "../../Application/GameObject/Camera/CameraBase.h"
#include "../Components/RenderComponent.h"
#include "../Components/TransformComponent.h"
#include "../ImGui/Log/Logger.h"
#include "../Data/Loader/Manifest/ManifestLoader.h"

// Helpers
static std::string Local_sjis_to_utf8(const std::string& str)
{
	if (str.empty()) return "";
	int size_needed = MultiByteToWideChar(932, 0, &str[0], (int)str.size(), NULL, 0);
	std::wstring wstr(size_needed, 0);
	MultiByteToWideChar(932, 0, &str[0], (int)str.size(), &wstr[0], size_needed);
	int size_needed_utf8 = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), NULL, 0, NULL, NULL);
	std::string str_utf8(size_needed_utf8, 0);
	WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), &str_utf8[0], size_needed_utf8, NULL, NULL);
	return str_utf8;
}

void SceneSerializer::Save(const std::string& manifestPath, 
	const std::vector<std::shared_ptr<Entity>>& list, 
	const std::shared_ptr<CameraBase>& editorCamera)
{
	// Paths setup (Derived from manifestPath)
	std::filesystem::path manPath(manifestPath);
	std::string baseDir = manPath.parent_path().string();
	std::string objRel = "ObjData/ObjData.json";
	std::string playerRel = "State/Player/PlayerState.json";
	std::string objAbs = baseDir + "/" + objRel;
	std::string playerAbs = baseDir + "/" + playerRel;

	// 1) ObjData Saving
	ObjectData io;
	auto objects = io.ConvertToDataList(list);

	if (editorCamera)
	{
		ObjData cam;
		cam.name = "__OverheadCamera";
		cam.modelPath = "";
		cam.isDynamic = false;
		cam.pos = editorCamera->GetPosition();
		cam.rot = editorCamera->GetEulerDeg();
		cam.scale = { 1,1,1 };
		cam.isLit = false;
		cam.isUnLit = false;
		cam.isBright = false;
		cam.isShadow = false;
		objects.push_back(cam);
	}

	auto isFinite = [](float v) { return std::isfinite(v); };
	auto fixV3 = [&](Math::Vector3& v, float def)
	{
		if (!isFinite(v.x)) v.x = def;
		if (!isFinite(v.y)) v.y = def;
		if (!isFinite(v.z)) v.z = def;
	};

	for (auto& o : objects)
	{
		fixV3(o.pos, 0.0f);
		fixV3(o.rot, 0.0f);
		fixV3(o.scale, 1.0f);
	}
	std::filesystem::create_directories(std::filesystem::path(objAbs).parent_path());
	io.SaveObj(objects, objAbs);

	// 2) PlayerState Saving
	{
		nlohmann::json jp;
		std::shared_ptr<Entity> playerEnt = nullptr;
		for (auto& e : list)
		{
			if (e && e->GetName() == "Player") { playerEnt = e; break; }
		}

		if (!playerEnt)
		{
			jp = {
				{"name", "Player"},
				{"modelPath", ""},
				{"dynamic", true},
				{"pos",   {0,0,0}},
				{"rot",   {0,0,0}},
				{"scale", {1,1,1}},
				{"draw",  {{"lit", true}, {"unlit", true}, {"bright", false}, {"shadow", true}}}
			};
		}
		else
		{
			Math::Vector3 pos{ 0,0,0 }, rot{ 0,0,0 }, scl{ 1,1,1 };
			bool lit = true, unlit = true, bright = false, shadow = true;
			std::string modelPath; bool dynamic = false;

			if (playerEnt->HasComponent<TransformComponent>())
			{
				const auto& tf = playerEnt->GetComponent<TransformComponent>();
				pos = tf->GetPosition(); rot = tf->GetRotation(); scl = tf->GetScale();
			}
			if (playerEnt->HasComponent<RenderComponent>())
			{
				const auto& rc = playerEnt->GetComponent<RenderComponent>();
				modelPath = rc->GetModelPath();
				dynamic = rc->IsDynamic();
			}

			// Visibility flags check (Assuming Entity has visibility layout)
			// ...

			auto fixV3 = [&](Math::Vector3& v, float def) {
				if (!isFinite(v.x)) v.x = def;
				if (!isFinite(v.y)) v.y = def;
				if (!isFinite(v.z)) v.z = def;
			};
			fixV3(pos, 0.0f); fixV3(rot, 0.0f); fixV3(scl, 1.0f);

			jp = {
				{"name", Local_sjis_to_utf8(playerEnt->GetName())},
				{"modelPath", Local_sjis_to_utf8(modelPath)},
				{"dynamic", dynamic},
				{"pos",   {pos.x, pos.y, pos.z}},
				{"rot",   {rot.x, rot.y, rot.z}},
				{"scale", {scl.x, scl.y, scl.z}},
				{"draw",  {{"lit",lit},{"unlit",unlit},{"bright",bright},{"shadow",shadow}}}
			};
		}

		std::filesystem::create_directories(std::filesystem::path(playerAbs).parent_path());
		std::ofstream ofs(playerAbs);
		if (!ofs) Logger::Error("[Save] open failed: " + playerAbs);
		else      ofs << jp.dump(4);
	}

	// 3) Manifest Saving
	{
		nlohmann::json jm = {
			{"version", 1},
			{"name",    "StageA"},
			{"paths", {
				{"ObjData",     objRel},
				{"PlayerState", playerRel }
			}}
		};
		std::filesystem::create_directories(std::filesystem::path(manifestPath).parent_path());
		std::ofstream ofs(manifestPath);
		if (!ofs) Logger::Error("[Save] open failed: " + manifestPath);
		else      ofs << jm.dump(4);
	}

	Logger::Log("Serializer", "Saved to " + manifestPath);
}

bool SceneSerializer::Load(const std::string& manifestPath, 
	std::vector<std::shared_ptr<Entity>>& outEntities, 
	std::shared_ptr<CameraBase>& editorCamera)
{
	if (!std::filesystem::exists(manifestPath)) return false;

	StageManifest man = Loader::ManifestLoader::Load(manifestPath);

	ObjectData io;
	auto all = io.LoadJson(man.paths.objData);

	outEntities.clear();
	std::vector<ObjData> entitiesOnly;

	for (const auto& d : all)
	{
		if (d.name == "__OverheadCamera")
		{
			if (editorCamera)
			{
				editorCamera->SetPosition(d.pos);
				editorCamera->SetEulerDeg(d.rot);
			}
		}
		else
		{
			entitiesOnly.push_back(d);
		}
	}

	for (auto& data : entitiesOnly)
	{
		auto ent = std::make_shared<Entity>();

		auto tf = std::make_shared<TransformComponent>();
		tf->SetPosition(data.pos); // Adjusted to SetPosition
		tf->SetRotation(data.rot);
		tf->SetScale(data.scale);
		ent->AddComponent<TransformComponent>(tf);

		auto rc = std::make_shared<RenderComponent>();
		if (!data.modelPath.empty())
		{
			rc->SetModel(data.modelPath);
		}
		ent->AddComponent<RenderComponent>(rc);

		ent->SetName(data.name);
		ent->Init();
		outEntities.push_back(ent);
	}

	Logger::Log("Serializer", "Loaded from " + manifestPath);
	return true;
}
