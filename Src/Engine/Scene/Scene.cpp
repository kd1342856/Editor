#include "Scene.h"
#include "SceneManager.h"

void Scene::AddEntity(const std::shared_ptr<Entity>& entity)
{
	SceneManager::Instance().AddEntity(entity);
}
