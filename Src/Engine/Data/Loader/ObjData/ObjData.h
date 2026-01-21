#pragma once

// JSON Support
#include "Framework/KdFramework.h" // Ensures json support if in framework

struct ObjData
{
	std::string name;
	std::string modelPath;
	bool isDynamic = false;

	Math::Vector3 pos = Math::Vector3::Zero;
	Math::Vector3 rot = Math::Vector3::Zero;
	Math::Vector3 scale = Math::Vector3::One;

	bool isLit = true;
	bool isUnLit = true;
	bool isBright = false;
	bool isShadow = true;
};

class Entity;

class ObjectData
{
public:
	// Convert Entity list to Data list for saving
	std::vector<ObjData> ConvertToDataList(const std::vector<std::shared_ptr<Entity>>& src);

	// Save to JSON file
	void SaveObj(const std::vector<ObjData>& list, const std::string& path);

	// Load from JSON file
	std::vector<ObjData> LoadJson(const std::string& path);
};
