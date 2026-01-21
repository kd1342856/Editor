#pragma once

class Entity;

// Abstract Base Scene
class Scene
{
public:
	virtual ~Scene() {}

	virtual void Init() {}
	virtual void Update() {}
	virtual void Draw() {}
	virtual void Release() {}

	// Helper to add ECS objects during Init
	void AddEntity(const std::shared_ptr<Entity>& entity);

protected:
	// Scene can manage its own local entities if needed, 
	// or delegate to SceneManager. For now, let's say Scene delegates to Manager.
};
