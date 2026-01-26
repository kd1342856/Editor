#pragma once

class Entity;
class Scene;

class SceneManager
{
public:


	void Init();
	void PreUpdate();
	void Update();
	void PostUpdate();
	void PreDraw();
	void Draw();
	void DrawDebug();
	void DrawSprite();
	void Release();

	// ECS
	void AddEntity(const std::shared_ptr<Entity>& entity);
	void RemoveEntity(const std::shared_ptr<Entity>& entity);
	const std::vector<std::shared_ptr<Entity>>& GetEntityList() const { return m_entityList; }
	void ClearEntities();

	// シーン
	using SceneFactory = std::function<std::shared_ptr<Scene>()>;
	void RegisterScene(const std::string& name, SceneFactory factory);
	void ChangeScene(const std::string& name);

	std::vector<std::string> GetSceneNames();
    const std::string& GetCurrentSceneName() const { return m_currentSceneName; }
	void CreateScene(const std::string& name);

private:
	SceneManager() {}
	~SceneManager() { Release(); }

	std::vector<std::shared_ptr<Entity>> m_entityList;

	std::unordered_map<std::string, SceneFactory> m_sceneRegistry;
	std::shared_ptr<Scene> m_currentScene = nullptr;
    std::string m_currentSceneName = "";
	std::string m_nextSceneName = "";

public:
	static SceneManager& Instance()
	{
		static SceneManager instance;
		return instance;
	}
};
