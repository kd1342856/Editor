#include "EntityManager.h"

void EntityManager::Update()
{
	for (auto& entity : m_entityList)
	{
		entity->Update();
	}
}

void EntityManager::PostUpdate()
{
	for (auto& entity : m_entityList)
	{
		entity->PostUpdate();
	}
}

void EntityManager::PreDraw()
{
	for (auto& entity : m_entityList)
	{
		entity->PreDraw();
	}
}

void EntityManager::Release()
{
	ClearEntities();
}

void EntityManager::InitEntities()
{
	for (auto& entity : m_entityList)
	{
		if (entity)
		{
			entity->Init();
		}
	}
}

void EntityManager::ActivateEntities()
{
	for (auto& entity : m_entityList)
	{
		if (entity)
		{
			entity->Activate();
		}
	}
}

void EntityManager::AddEntity(const std::shared_ptr<Entity>& entity)
{
	if (entity)
	{
		// Queue for addition
		m_pendingAddList.push_back(entity);
	}
}

void EntityManager::RemoveEntity(const std::shared_ptr<Entity>& entity)
{
	if (entity)
	{
		// Queue for removal
		m_pendingRemoveList.push_back(entity);
	}
}

void EntityManager::ProcessPendingUpdates()
{
	// Add pending entities
	for (const auto& entity : m_pendingAddList)
	{
		entity->Init();
		m_entityList.push_back(entity);
	}
	m_pendingAddList.clear();

	// Remove pending entities
	for (const auto& entity : m_pendingRemoveList)
	{
		auto it = std::remove(m_entityList.begin(), m_entityList.end(), entity);
		if (it != m_entityList.end())
		{
			m_entityList.erase(it, m_entityList.end());
		}
	}
	m_pendingRemoveList.clear();
}

void EntityManager::ClearEntities()
{
	m_entityList.clear();
	m_pendingAddList.clear();
	m_pendingRemoveList.clear();
}