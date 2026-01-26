#pragma once

class Entity;
class CameraBase;

class SceneSerializer
{
public:
	static void Save(const std::string& manifestPath, 
		const std::vector<std::shared_ptr<Entity>>& entities, 
		const std::shared_ptr<CameraBase>& editorCamera);

	static bool Load(const std::string& manifestPath, 
		std::vector<std::shared_ptr<Entity>>& outEntities, 
		std::shared_ptr<CameraBase>& editorCamera);
};
