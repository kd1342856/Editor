#include "ManifestLoader.h"
#include <fstream>
#include "Framework/KdFramework.h" // For json

using namespace Loader;

StageManifest ManifestLoader::Load(const std::string& path)
{
	StageManifest man;
	std::ifstream ifs(path);
	if (!ifs) return man;

	nlohmann::json j;
	ifs >> j;

	man.name = j.value("name", "Unknown");

	if (j.contains("paths"))
	{
		auto& p = j["paths"];
		std::filesystem::path base = std::filesystem::path(path).parent_path();

		// Convert relative paths in manifest to absolute paths (or run-relative)
		std::string objRel = p.value("ObjData", "");
		std::string playerRel = p.value("PlayerState", "");

		man.paths.objData = (base / objRel).string();
		man.paths.playerState = (base / playerRel).string();
	}

	return man;
}