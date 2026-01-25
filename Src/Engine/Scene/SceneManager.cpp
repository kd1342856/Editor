#include "SceneManager.h"
#include "Scene.h"
#include "../../Application/Scene/BaseScene/BaseScene.h"

void SceneManager::Init()
{
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
		}
		else
		{
			// Not registered? Try dynamic BaseScene
			auto s = std::make_shared<BaseScene>();
			s->SetName(m_nextSceneName);
			m_currentScene = s;
		}

        m_currentSceneName = m_nextSceneName;

		if (m_currentScene)
		{
			m_currentScene->Init();
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

void SceneManager::RemoveEntity(const std::shared_ptr<Entity>& entity)
{
	auto it = std::remove(m_entityList.begin(), m_entityList.end(), entity);
	if (it != m_entityList.end())
	{
		m_entityList.erase(it, m_entityList.end());
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

std::vector<std::string> SceneManager::GetSceneNames()
{
	std::vector<std::string> names;
	
	// 1. Registered Scenes
	for (const auto& pair : m_sceneRegistry)
	{
		names.push_back(pair.first);
	}

	// 2. Scan JSON files in Asset/Data/Scene/
	std::filesystem::path sceneDir = "Asset/Data/Scene";
	if (std::filesystem::exists(sceneDir))
	{
		for (const auto& entry : std::filesystem::directory_iterator(sceneDir))
		{
			if (entry.path().extension() == ".json")
			{
				std::string name = entry.path().stem().string();
				// Avoid duplicates if already registered
				if (m_sceneRegistry.find(name) == m_sceneRegistry.end())
				{
					names.push_back(name);
				}
			}
		}
	}

	return names;
}

void SceneManager::CreateScene(const std::string& name)
{
	// Simply creating a JSON file is enough for BaseScene to load it later
	std::string path = "Asset/Data/Scene/" + name + ".json";
	
	// Create directory if not exists
	std::filesystem::path dir = "Asset/Data/Scene";
	if (!std::filesystem::exists(dir)) std::filesystem::create_directories(dir);

	if (!std::filesystem::exists(path))
	{
		std::ofstream ofs(path);
		if (ofs.is_open())
		{
			// Basic empty scene JSON
			ofs << "{ \"Entities\": [], \"Camera\": {} }";
			ofs.close();
		}
	}

	// If it's not a registered class scene, we treat it as a BaseScene
	if (m_sceneRegistry.find(name) == m_sceneRegistry.end())
	{
		// Auto-register a factory for BaseScene
		RegisterScene(name, [name]() { 
			auto s = std::make_shared<BaseScene>(); 
			s->SetName(name); 
			return s; 
		});
	}

	ChangeScene(name);
}
