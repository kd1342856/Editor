#include "HierarchyPanel.h"
#include "../../EditorManager.h"
#include "../../../../ECS/Entity/EntityManager.h"

namespace EditorPanels
{
	void HierarchyPanel::Draw(EditorManager& editor)
	{
		ImGui::Begin("Hierarchy");
		if (ImGui::TreeNode("Entities"))
		{
			auto& list = EntityManager::Instance().GetEntityList();
			for (size_t idx = 0; idx < list.size(); ++idx)
			{
				auto& entity = list[idx];
				if (!entity) continue;
				
				ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_Leaf;

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
