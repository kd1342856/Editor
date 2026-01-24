#include "SceneManager.h"
#include "Scene.h"

void SceneManager::Init()
{
	// Initial setup
}

void SceneManager::PreUpdate()
{
}

void SceneManager::Update()
{
	// Scene Transition
	if (!m_nextSceneName.empty())
	{
		if (m_currentScene) m_currentScene->Release();
		ClearEntities();
		
		auto it = m_sceneRegistry.find(m_nextSceneName);
		if (it != m_sceneRegistry.end())
		{
			m_currentScene = it->second();
			if (m_currentScene)
			{
				m_currentScene->Init();
			}
		}
		m_nextSceneName.clear();
	}

	// Logic Update
	if (m_currentScene) m_currentScene->Update();

	for (auto& entity : m_entityList)
	{
		entity->Update();
	}
}

void SceneManager::PostUpdate()
{
	for (auto& entity : m_entityList)
	{
		entity->PostUpdate();
	}
}

void SceneManager::PreDraw()
{
	for (auto& entity : m_entityList)
	{
		entity->PreDraw();
	}
}

void SceneManager::Draw()
{
	// 3D Drawing
	if (m_currentScene) m_currentScene->Draw();

	// Render States are usually managed by ShaderManager
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

void SceneManager::DrawDebug()
{
	for (auto& entity : m_entityList)
	{
		entity->DrawDebug();
	}
}

void SceneManager::DrawSprite()
{
	for (auto& entity : m_entityList)
	{
		entity->DrawSprite();
	}
}

void SceneManager::Release()
{
	if (m_currentScene) m_currentScene->Release();
	ClearEntities();
}

void SceneManager::AddEntity(const std::shared_ptr<Entity>& entity)
{
	if (entity)
	{
		entity->Init();
		m_entityList.push_back(entity);
	}
}

void SceneManager::ClearEntities()
{
	m_entityList.clear();
}

void SceneManager::RegisterScene(const std::string& name, SceneFactory factory)
{
	m_sceneRegistry[name] = factory;
}

void SceneManager::ChangeScene(const std::string& name)
{
	m_nextSceneName = name;
}
