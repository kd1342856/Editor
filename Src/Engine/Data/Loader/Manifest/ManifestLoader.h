#pragma once

struct StageManifest
{
	std::string name;
	struct Paths {
		std::string objData;
		std::string playerState;
	} paths;
};

namespace Loader {
	class ManifestLoader
	{
	public:
		static StageManifest Load(const std::string& path);
	};
}
