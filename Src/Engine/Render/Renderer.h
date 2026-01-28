#pragma once

class Renderer
{
public:
	void Draw();
	void DrawDebug();
	void DrawSprite();

private:
	Renderer() {}
	~Renderer() {}

public:
	static Renderer& Instance()
	{
		static Renderer instance;
		return instance;
	}
};