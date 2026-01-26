#pragma once

class Entity;
class EditorManager;
class CameraBase;
class GameScene;
enum class EditorMode; 

class ImGuiManager
{
public:

	void GuiInit();
	void GuiProcess();
	void GuiRelease();

	void SetMode(EditorMode m);
	void ToggleMode();

	void SetCameras(const std::shared_ptr<CameraBase>& tps,
		const std::shared_ptr<CameraBase>& build,
		const std::shared_ptr<CameraBase>& editor);
	
	std::shared_ptr<EditorManager> m_editor;

	// アクセサ
	float GetWheelDelta() const { return ImGui::GetIO().MouseWheel; }
	bool GetGameViewUVFromMouse(float& u, float& v) const;
	bool IsMouseOverGameView() const;
	bool IsUIMouseCaptured()const { return ImGui::GetIO().WantCaptureMouse; }

	// デバッグ
	void RegisterDebugWindow(void* id, std::function<void()> cb);
	void UnregisterDebugWindow(void* id);

private:
	ImGuiManager() {}
	~ImGuiManager() { GuiRelease(); }

	// コールバック
	std::unordered_map<void*, std::function<void()>> m_debugCallbacks;
	
	bool m_isReleased = false;
public:
	static ImGuiManager& Instance()
	{
		static ImGuiManager Instance;
		return Instance;
	}
};
