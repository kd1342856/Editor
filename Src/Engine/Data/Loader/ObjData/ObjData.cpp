#include "ObjData.h"
#include "../../../ECS/Entity.h"
#include "../../../Components/RenderComponent.h"
#include "../../../Components/TransformComponent.h"

std::vector<ObjData> ObjectData::ConvertToDataList(const std::vector<std::shared_ptr<Entity>>& src)
{
	std::vector<ObjData> ret;
	for (const auto& e : src)
	{
		if (!e) continue;
		// Skip Player/Camera as they are handled separately or by tag
		if (e->GetName() == "Player") continue; 

		ObjData d;
		d.name = e->GetName();

		if (e->HasComponent<RenderComponent>())
		{
			const auto& rc = e->GetComponent<RenderComponent>();
			d.modelPath = rc->GetModelPath(); 
			d.isDynamic = rc->IsDynamic();
		}
		
		if (e->HasComponent<TransformComponent>())
		{
			const auto& tf = e->GetComponent<TransformComponent>();
			d.pos = tf->GetPosition();
			d.rot = tf->GetRotation();
			d.scale = tf->GetScale();
		}

		// Visibility
		// d.isLit = e->IsVisible(Entity::VisibilityFlags::Lit);
		// ...

		ret.push_back(d);
	}
	return ret;
}

void ObjectData::SaveObj(const std::vector<ObjData>& list, const std::string& path)
{
	nlohmann::json j_list = nlohmann::json::array();

	for (const auto& d : list)
	{
		nlohmann::json j;
		j["name"] = d.name;
		j["modelPath"] = d.modelPath;
		j["dynamic"] = d.isDynamic;
		j["pos"] = { d.pos.x, d.pos.y, d.pos.z };
		j["rot"] = { d.rot.x, d.rot.y, d.rot.z };
		j["scale"] = { d.scale.x, d.scale.y, d.scale.z };
		j["draw"] = {
			{"lit", d.isLit},
			{"unlit", d.isUnLit},
			{"bright", d.isBright},
			{"shadow", d.isShadow}
		};
		j_list.push_back(j);
	}

	std::ofstream ofs(path);
	if (ofs) ofs << j_list.dump(4);
}

std::vector<ObjData> ObjectData::LoadJson(const std::string& path)
{
	std::vector<ObjData> ret;
	std::ifstream ifs(path);
	if (!ifs) return ret;

	nlohmann::json j_list;
	ifs >> j_list;

	for (const auto& j : j_list)
	{
		ObjData d;
		d.name = j.value("name", "Unknown");
		d.modelPath = j.value("modelPath", "");
		d.isDynamic = j.value("dynamic", false);

		if (j.contains("pos")) {
			auto& p = j["pos"];
			d.pos = { p[0], p[1], p[2] };
		}
		if (j.contains("rot")) {
			auto& p = j["rot"];
			d.rot = { p[0], p[1], p[2] };
		}
		if (j.contains("scale")) {
			auto& p = j["scale"];
			d.scale = { p[0], p[1], p[2] };
		}

		if (j.contains("draw")) {
			auto& draw = j["draw"];
			d.isLit = draw.value("lit", true);
			d.isUnLit = draw.value("unlit", true);
			d.isBright = draw.value("bright", false);
			d.isShadow = draw.value("shadow", true);
		}

		ret.push_back(d);
	}
	return ret;
}
