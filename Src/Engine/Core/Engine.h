#pragma once

class Engine
{
public:

	bool Init(int width, int height);
	void Execute();
	void Release();

	void Quit() { m_endFlag = true; }
	void SetMouseGrabbed(bool enable); // Added

	// Accessors
	int GetWindowWidth() const { return m_windowWidth; }
	int GetWindowHeight() const { return m_windowHeight; }
	HWND GetWindowHandle() const { return m_window.GetWndHandle(); }

private:
	Engine() {}
	~Engine() { Release(); }

	void KdBeginUpdate();
	void KdPostUpdate();
	void KdBeginDraw(bool usePostProcess = true);
	void KdPostDraw();



	void PreUpdate();
	void Update();
	void PostUpdate();

	void PreDraw();
	void Draw();
	void DrawSprite();
	void PostDraw();

	KdWindow m_window;

	bool m_endFlag = false;
	bool m_isReleased = false;
	int m_windowWidth = 1280;
	int m_windowHeight = 720;

public:
	static Engine& Instance()
	{
		static Engine instance;
		return instance;
	}
};
