#include "EnginePch.h"
#include "InspectorPanel.h"
#include "../../EditorManager.h"
#include "imgui/imgui.h"

namespace EditorPanels
{
	void InspectorPanel::Draw(EditorManager& editor)
	{
		ImGui::Begin("Inspector");
		// Stub: Show details of selected entity
		ImGui::Text("Inspector Placeholder");
		ImGui::End();
	}
}
