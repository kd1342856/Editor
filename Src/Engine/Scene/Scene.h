#pragma once

class Entity;

class Scene
{
public:
	virtual ~Scene() {}

	virtual void Init() {}
	virtual void Update() {}
	virtual void Draw() {}
	virtual void Release() {}

	void AddEntity(const std::shared_ptr<Entity>& entity);

protected:

};
