#pragma once

class Entity;

class Component
{
public:
	virtual ~Component() {}

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
	virtual void DrawDebug() {}

	void SetOwner(const std::shared_ptr<Entity>& owner) { m_owner = owner; }
	std::shared_ptr<Entity> GetOwner() const { return m_owner.lock(); }

	void SetEnable(bool enable) { m_enable = enable; }
	bool IsEnable() const { return m_enable; }

protected:
	std::weak_ptr<Entity> m_owner;
	bool m_enable = true;
};
