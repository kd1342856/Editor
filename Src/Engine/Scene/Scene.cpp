#include "Scene.h"
#include "../ECS/EntityManager.h"

void Scene::AddEntity(const std::shared_ptr<Entity>& entity)
{
	EntityManager::Instance().AddEntity(entity);
}
