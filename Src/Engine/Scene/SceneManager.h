#pragma once

class Entity;
class Scene;

class SceneManager
{
public:


	void Init(){}
	void PreUpdate(){}
	void Update();
	void PreDraw();
	void Draw();
	void DrawSprite();
	void Release();

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
