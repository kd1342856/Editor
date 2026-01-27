#include "HierarchyPanel.h"
#include "../../EditorManager.h"
#include "../../../../ECS/EntityManager.h"

namespace EditorPanels
{
	void HierarchyPanel::Draw(EditorManager& editor)
	{
		ImGui::Begin("Hierarchy");
		// Temporary List
		if (ImGui::TreeNode("Entities"))
		{
			auto& list = EntityManager::Instance().GetEntityList();
			for (size_t i = 0; i < list.size(); ++i)
			{
				auto& entity = list[i];
				if (!entity) continue;
				
				ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_Leaf;
				// if (editor.GetSelectedIndex() == i) flags |= ImGuiTreeNodeFlags_Selected;

				if (ImGui::TreeNodeEx(entity->GetName().c_str(), flags))
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
