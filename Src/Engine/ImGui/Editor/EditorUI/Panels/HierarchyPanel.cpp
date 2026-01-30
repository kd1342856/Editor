#include "HierarchyPanel.h"
#include "../../EditorManager.h"
#include "../../../../ECS/Entity/EntityManager.h"

namespace EditorPanels
{
	void HierarchyPanel::Draw(EditorManager& editor)
	{
		if (ImGui::Begin("Hierarchy"))
		{
			// 作成用の右クリックコンテキストメニュー
			if (ImGui::BeginPopupContextWindow("HierarchyContextMenu"))
			{
				if (ImGui::MenuItem("Create Empty Object"))
				{
					auto entity = std::make_shared<Entity>();
					std::string name = editor.GetUniqueName("Empty Object");
					entity->SetName(name);
					entity->AddComponent(std::make_shared<TransformComponent>());
					entity->Init();
					EntityManager::Instance().AddEntity(entity);
					entity->Activate();
				}

				ImGui::Separator();

				ImGui::EndPopup();
			}

			const auto& entities = EntityManager::Instance().GetEntityList();

			// 削除処理 (Deleteキー)
			if (ImGui::IsWindowFocused() && ImGui::IsKeyPressed(ImGuiKey_Delete))
			{
				if (auto selected = editor.GetSelectedEntity())
				{
					EntityManager::Instance().RemoveEntity(selected);
					editor.SetSelectedEntity(nullptr); // 選択解除
				}
			}

			// エンティティ一覧
			auto displayEntities = entities; 

			for (int idx=0; idx<displayEntities.size(); ++idx)
			{
				auto& entity = displayEntities[idx];

				// ID重複回避のために ##Address を付与
				std::string label = entity->GetName() + "##" + std::to_string((uintptr_t)entity.get());

				bool isSelected = (editor.GetSelectedEntity() == entity);
				if(ImGui::Selectable(label.c_str(), isSelected))
				{
					editor.SetSelectedEntity(entity);
				}
			}
		}
		ImGui::End();
	}
}
