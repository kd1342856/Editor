#pragma once

class Entity;

class RenderSystem
{
public:

	void BeginFrame();
	void Submit(const std::shared_ptr<Entity>& entity);

	void Execute3D();
	void ExecuteSprite();
	void ExecuteDebug();

private:
	std::vector<std::shared_ptr<Entity>> m_entities;

	RenderSystem() {}
	~RenderSystem() {}

public:
	static RenderSystem& Instance()
	{
		static RenderSystem instance;
		return instance;
	}
};
