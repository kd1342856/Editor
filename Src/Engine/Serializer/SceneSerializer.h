#pragma once

class Entity;
class CameraBase;

class SceneSerializer
{
public:
	// Save entities and camera state to a manifest/json structure
	static void Save(const std::string& manifestPath, 
		const std::vector<std::shared_ptr<Entity>>& entities, 
		const std::shared_ptr<CameraBase>& editorCamera);

	// Load entities from manifest, populate outEntities, and update camera
	// Returns true on success
	static bool Load(const std::string& manifestPath, 
		std::vector<std::shared_ptr<Entity>>& outEntities, 
		std::shared_ptr<CameraBase>& editorCamera);
};
