#include "Renderer.h"
#include "../ECS/EntityManager.h"

void Renderer::Draw()
{
	const auto& entities = EntityManager::Instance().GetEntityList();

	KdShaderManager::Instance().m_StandardShader.BeginLit();
	for (auto& entity : entities)
	{
		entity->DrawLit();
	}
	KdShaderManager::Instance().m_StandardShader.EndLit();

	KdShaderManager::Instance().m_StandardShader.BeginUnLit();
	for (auto& entity : entities)
	{
		entity->DrawUnLit();
	}
	KdShaderManager::Instance().m_StandardShader.EndUnLit();

	DrawDebug();
}

void Renderer::DrawDebug()
{
	const auto& entities = EntityManager::Instance().GetEntityList();
	for (auto& entity : entities)
	{
		entity->DrawDebug();
	}
}

void Renderer::DrawSprite()
{
	const auto& entities = EntityManager::Instance().GetEntityList();
	for (auto& entity : entities)
	{
		entity->DrawSprite();
	}
}