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

void EntityManager::Draw()
{
	KdShaderManager::Instance().m_StandardShader.BeginLit();
	for (auto& entity : m_entityList)
	{
		entity->DrawLit();
	}
	KdShaderManager::Instance().m_StandardShader.EndLit();

	KdShaderManager::Instance().m_StandardShader.BeginUnLit();
	for (auto& entity : m_entityList)
	{
		entity->DrawUnLit();
	}
	KdShaderManager::Instance().m_StandardShader.EndUnLit();

	DrawDebug();
}

void EntityManager::DrawDebug()
{
	for (auto& entity : m_entityList)
	{
		entity->DrawDebug();
	}
}

void EntityManager::DrawSprite()
{
	for (auto& entity : m_entityList)
	{
		entity->DrawSprite();
	}
}

void EntityManager::Release()
{
	ClearEntities();
}

void EntityManager::AddEntity(const std::shared_ptr<Entity>& entity)
{
	if (entity)
	{
		entity->Init();
		m_entityList.push_back(entity);
	}
}

void EntityManager::RemoveEntity(const std::shared_ptr<Entity>& entity)
{
	auto it = std::remove(m_entityList.begin(), m_entityList.end(), entity);
	if (it != m_entityList.end())
	{
		m_entityList.erase(it, m_entityList.end());
	}
}

void EntityManager::ClearEntities()
{
	m_entityList.clear();
}