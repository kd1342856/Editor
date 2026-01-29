#include "RenderSystem.h"
#include "../ECS/Entity/Entity/Entity.h"

void RenderSystem::BeginFrame()
{
	m_entities.clear();
}

void RenderSystem::Submit(const std::shared_ptr<Entity>& entity)
{
	if (entity)
	{
		m_entities.push_back(entity);
	}
}

void RenderSystem::Execute3D()
{
	// 1. Lit Pass (Light affected)
	KdShaderManager::Instance().m_StandardShader.BeginLit();
	for (const auto& entity : m_entities)
	{
		entity->DrawLit();
	}
	KdShaderManager::Instance().m_StandardShader.EndLit();
	

	// 2. UnLit Pass
	KdShaderManager::Instance().m_StandardShader.BeginUnLit();	
	for (const auto& entity : m_entities)
	{
		entity->DrawUnLit();
	}
	KdShaderManager::Instance().m_StandardShader.EndUnLit();
	
	for (const auto& entity : m_entities)
	{
		entity->DrawBright();
	}
}

void RenderSystem::ExecuteSprite()
{
	for (const auto& entity : m_entities)
	{
		entity->DrawSprite();
	}
}

void RenderSystem::ExecuteDebug()
{
	// 5. Debug Pass
	// Debug drawing (lines, spheres) handles its own shader state (often UnLit or internal debug shader).
	for (const auto& entity : m_entities)
	{
		entity->DrawDebug();
	}
}
