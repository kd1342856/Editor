#include "Scene.h"
#include "../ECS/Entity/EntityManager.h"

void Scene::AddEntity(const std::shared_ptr<Entity>& entity)
{
	EntityManager::Instance().AddEntity(entity);
}
