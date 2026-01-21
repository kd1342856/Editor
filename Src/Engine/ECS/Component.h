#pragma once

class Entity;

// Base class for all components
class Component
{
public:
	virtual ~Component() {}

	// Lifecycle methods
	virtual void Init() {}
	virtual void Update() {}
	virtual void PostUpdate() {}
	virtual void PreDraw() {}
	virtual void Draw() {}
	virtual void DrawLit() {}
	virtual void DrawUnLit() {}
	virtual void DrawBright() {}
	virtual void GenerateDepthMapFromLight() {}
	virtual void DrawSprite() {}
	virtual void DrawInspector() {} 
	virtual void DrawDebug() {} // Debug drawing

	// Owner management
	void SetOwner(const std::shared_ptr<Entity>& owner) { m_owner = owner; }
	std::shared_ptr<Entity> GetOwner() const { return m_owner.lock(); }

protected:
	std::weak_ptr<Entity> m_owner;
};
