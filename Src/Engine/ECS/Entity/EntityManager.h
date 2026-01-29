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
	void ProcessPendingUpdates();

	const std::vector<std::shared_ptr<Entity>>& GetEntityList() const { return m_entityList; }

private:
	EntityManager() {}
	~EntityManager() { Release(); }

	std::vector<std::shared_ptr<Entity>> m_entityList;
	std::vector<std::shared_ptr<Entity>> m_pendingAddList;
	std::vector<std::shared_ptr<Entity>> m_pendingRemoveList;

public:
	static EntityManager& Instance()
	{
		static EntityManager instance;
		return instance;
	}
};