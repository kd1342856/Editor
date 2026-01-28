#pragma once
class Entity;

class EntityManager
{
public:
	void Update();
	void PostUpdate();
	void PreDraw();
	void Release();

	void InitEntities();
	void ActivateEntities();
	void AddEntity(const std::shared_ptr<Entity>& entity);
	void RemoveEntity(const std::shared_ptr<Entity>& entity);
	void ClearEntities();
	const std::vector<std::shared_ptr<Entity>>& GetEntityList() const { return m_entityList; }

private:
	EntityManager() {}
	~EntityManager() { Release(); }

	std::vector<std::shared_ptr<Entity>> m_entityList;

public:
	static EntityManager& Instance()
	{
		static EntityManager instance;
		return instance;
	}
};