#include "ImGuiManager.h"
#include "../Scene/SceneManager.h"
#include "Editor/EditorManager.h"

void ImGuiManager::GuiInit()
{
	m_editor = std::make_shared<EditorManager>();
	m_editor->Init();
	// if (m_editor) m_editor->SetMode(EditorMode::Game);

	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui::StyleColorsClassic();

	ImGui_ImplWin32_Init(Engine::Instance().GetWindowHandle());
	ImGui_ImplDX11_Init(KdDirect3D::Instance().WorkDev(), KdDirect3D::Instance().WorkDevContext());

#include "imgui/ja_glyph_ranges.h"
	ImGuiIO& io = ImGui::GetIO();
	ImFontConfig config;
	config.MergeMode = true;
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	// io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable; // Viewports can be unstable, optional

	ImGuiStyle& style = ImGui::GetStyle();
	if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		style.WindowRounding = 0.0f;
		style.Colors[ImGuiCol_WindowBg].w = 1.0f;
	}

	io.Fonts->AddFontDefault();
	// Windows Font for Japanese
	io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\msgothic.ttc", 13.0f, &config, glyphRangesJapanese);

	m_isReleased = false;
}

void ImGuiManager::GuiProcess()
{
	if (m_isReleased) return;

	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
	
	// Create a full-screen DockSpace
	ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());

	// Main Editor Draw
	if (m_editor)
	{
		m_editor->Draw();

		m_editor->DrawUI();
	}

	// Debug Callbacks
	for (auto& [id, cb] : m_debugCallbacks)
	{
		if (cb) cb();
	}

	ImGui::Render();
	
	// Ensure BackBuffer is bound for ImGui Binding
	if (auto backBuffer = KdDirect3D::Instance().WorkBackBuffer())
	{
		ID3D11RenderTargetView* rtv = backBuffer->WorkRTView();
		KdDirect3D::Instance().WorkDevContext()->OMSetRenderTargets(1, &rtv, nullptr);
	}

	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

	if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		ImGui::UpdatePlatformWindows();
		ImGui::RenderPlatformWindowsDefault();
	}
}

void ImGuiManager::GuiRelease()
{
	if (m_isReleased) return;
	
	// Release EditorManager resources BEFORE destroying ImGui context
	m_editor.reset();

	if (ImGui::GetCurrentContext())
	{
		ImGui_ImplDX11_Shutdown();
		ImGui_ImplWin32_Shutdown();
		ImGui::DestroyContext();
	}

	m_isReleased = true;
}

void ImGuiManager::SetMode(EditorMode m)
{
	// if (m_editor) m_editor->SetMode(m);
}

void ImGuiManager::ToggleMode()
{
	// if (!m_editor) return;
	// m_editor->SetMode(m_editor->IsEditorMode() ? EditorMode::Game : EditorMode::Editor);
}

void ImGuiManager::RegisterDebugWindow(void* id, std::function<void()> cb)
{
	m_debugCallbacks[id] = cb;
}

void ImGuiManager::UnregisterDebugWindow(void* id)
{
	m_debugCallbacks.erase(id);
}

void ImGuiManager::SetCameras(const std::shared_ptr<CameraBase>& tps,
	const std::shared_ptr<CameraBase>& build,
	const std::shared_ptr<CameraBase>& editor)
{
	if (m_editor) m_editor->SetCameras(tps, build, editor);
}

// Mouse Helpers
bool ImGuiManager::GetGameViewUVFromMouse(float& u, float& v) const
{
	return false; 
}

bool ImGuiManager::IsMouseOverGameView() const
{
	return false;
}
