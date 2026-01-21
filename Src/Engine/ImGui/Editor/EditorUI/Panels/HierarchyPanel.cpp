#include "EnginePch.h"
#include "HierarchyPanel.h"
#include "../../EditorManager.h"
#include "imgui/imgui.h"

namespace EditorPanels
{
	void HierarchyPanel::Draw(EditorManager& editor)
	{
		ImGui::Begin("Hierarchy");
		// Temporary List
		if (ImGui::TreeNode("Entities"))
		{
			auto& list = editor.GetEntityList();
			for (size_t i = 0; i < list.size(); ++i)
			{
				auto& e = list[i];
				if (!e) continue;
				
				ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_Leaf;
				// if (editor.GetSelectedIndex() == i) flags |= ImGuiTreeNodeFlags_Selected;

				if (ImGui::TreeNodeEx(e->GetName().c_str(), flags))
				{
					if (ImGui::IsItemClicked())
					{
						// editor.SelectIndex(i);
					}
					ImGui::TreePop();
				}
			}
			ImGui::TreePop();
		}
		ImGui::End();
	}
}
