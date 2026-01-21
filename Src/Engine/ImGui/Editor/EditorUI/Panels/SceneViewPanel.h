#pragma once

class EditorManager;

namespace EditorPanels
{
	class SceneViewPanel
	{
	public:
		void DrawWindow(EditorManager& editor);  // For Editor Mode
		void DrawOverlay(EditorManager& editor); // For Game Mode (Fullscreen)

	private:
		// State for view manipulation
		bool m_gameImgValid = false;
		Math::Vector2 m_gameImgMin = Math::Vector2::Zero;
		Math::Vector2 m_gameImgMax = Math::Vector2::Zero;
	};
}
