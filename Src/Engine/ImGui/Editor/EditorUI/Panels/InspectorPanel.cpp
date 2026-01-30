#include "InspectorPanel.h"
#include "../../EditorManager.h"
#include "../../File/ImGuiFileBrowser.h"

#include "../../../../ECS/Entity/Entity/Entity.h"
#include "../../../../Components/Transform/TransformComponent.h"
#include "../../../../Components/Render/RenderComponent.h"
#include "../../../../Components/Collider/ColliderComponent.h"
#include "../../../../Components/Action/Player/ActionPlayerComponent.h"

// ImGui Helper for Inspector
namespace 
{
	void DrawComponentTransform(const std::shared_ptr<Entity>& sel)
	{
		if (!sel->HasComponent<TransformComponent>()) return;

		if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen))
		{
			auto trans = sel->GetComponent<TransformComponent>();
			if (trans)
			{
				bool enable = trans->IsEnable();
				if (ImGui::Checkbox("Enable##Transform", &enable)) trans->SetEnable(enable);

				Math::Vector3 pos = trans->GetPosition();
				Math::Vector3 rot = trans->GetRotation();
				Math::Vector3 scale = trans->GetScale();

				if (ImGui::DragFloat3("Position", &pos.x, 0.1f))	trans->SetPosition(pos);
				if (ImGui::DragFloat3("Rotation", &rot.x, 0.1f))	trans->SetRotation(rot);
				if (ImGui::DragFloat3("Scale", &scale.x, 0.1f))	trans->SetScale(scale);
			}
		}
	}

	void DrawComponentRender(const std::shared_ptr<Entity>& sel)
	{
		if (!sel->HasComponent<RenderComponent>()) return;

		if (ImGui::CollapsingHeader("Render", ImGuiTreeNodeFlags_DefaultOpen))
		{
			auto render = sel->GetComponent<RenderComponent>();
			if (render)
			{
				bool enable = render->IsEnable();
				if (ImGui::Checkbox("Enable##Render", &enable))
				{
					render->SetEnable(enable);
				}
				// モデルパス編集
				char pathBuffer[MAX_PATH] = "";
				if (!render->GetModelPath().empty())
				{
					strncpy_s(pathBuffer, render->GetModelPath().c_str(), _TRUNCATE);
				}

				// InputText
				if (ImGui::InputText("Model Path", pathBuffer, sizeof(pathBuffer), ImGuiInputTextFlags_EnterReturnsTrue))
				{
					if (render->IsDynamic())
					{
						render->SetModelWork(pathBuffer);
					}
					else
					{
						render->SetModelData(pathBuffer);
					}
				}
				ImGui::SameLine();
				if (ImGui::Button("...##ModelSelect"))
				{
					ImGuiFileBrowser::Instance().Open(
						"SelectModel",
						"Select Model File",
						{ ".gltf", ".glb", ".obj", ".fbx" }, // 一般的なモデルフォーマット
						[render](const std::string& path)
						{
							std::string setPath = path;
							try
							{
								setPath = std::filesystem::relative(path, std::filesystem::current_path()).string();
							}
							catch (...) {}

							if (render->IsDynamic())
							{
								render->SetModelWork(setPath);
							}
							else
							{
								render->SetModelData(setPath);
							}
						},
						"Asset");
				}

				// ドラッグ＆ドロップ (ターゲット)
				if (ImGui::BeginDragDropTarget())
				{
					if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
					{
						const char* path = (const char*)payload->Data;
						if (render->IsDynamic())
							render->SetModelWork(path);
						else
							render->SetModelData(path);
					}
					ImGui::EndDragDropTarget();
				}

				// Dynamic Toggle
				bool isDynamic = render->IsDynamic();
				if (ImGui::Checkbox("Dynamic (Animation)", &isDynamic))
				{
					// Re-load with new mode
					if (isDynamic)
					{
						render->SetModelWork(render->GetModelPath());
					}

					else
					{
						render->SetModelData(render->GetModelPath());
					}
				}

				// Reload & File Select
				if (ImGui::Button("Reload"))
				{
					if (render->IsDynamic())
					{
						render->SetModelWork(render->GetModelPath());
					}
					else
					{
						render->SetModelData(render->GetModelPath());
					}
				}
				ImGui::SameLine();

				if (ImGui::Button("..."))
				{
					ImGuiFileBrowser::Instance().Open(
						"ModelSelectPopup",
						"Select Model",
						{ ".gltf", ".glb", ".obj", ".fbx" },
						[render](const std::string& path)
						{
							// リソースパスの相対化などは適宜
							std::string setPath = path;
							try
							{
								setPath = std::filesystem::relative(path, std::filesystem::current_path()).string();
							}
							catch (...) {}

							if (render->IsDynamic())
							{
								render->SetModelWork(setPath);
							}
							else
							{
								render->SetModelData(setPath);
							}
						},
						"Asset");
				}
			}
		}
	}

	void DrawComponentCollider(const std::shared_ptr<Entity>& sel)
	{
		if (!sel->HasComponent<ColliderComponent>()) return;

		if (ImGui::CollapsingHeader("Collider", ImGuiTreeNodeFlags_DefaultOpen))
		{
			auto collider = sel->GetComponent<ColliderComponent>();

			bool enable = collider->IsEnable();
			if (ImGui::Checkbox("Enable##Collider", &enable))
			{
				collider->SetEnable(enable);
			}

			ImGui::SameLine();

			bool debug = collider->IsDebugDrawEnabled();
			if (ImGui::Checkbox("Show Wireframe", &debug))
			{
				collider->SetDebugDrawEnabled(debug);
			}

			ImGui::Separator();

			// RenderComponentの状態を確認
			bool isDynamicMode = false;
			if (auto render = sel->GetComponent<RenderComponent>())
			{
				isDynamicMode = render->IsDynamic();
			}

			// ---- Collision Types ----
			ImGui::Text("Collision Types");

			if (isDynamicMode)
			{
				// Dynamic
				ImGui::TextDisabled("Fixed: Bump");
				// Dynamicの場合は通常Bump固定などの制約がある場合が多いが、ここでは表示のみ
			}
			else
			{
				// Static
				UINT type = collider->GetCollisionType();

				// Preview
				std::string typePreview = "";
				if (type & KdCollider::TypeGround) typePreview += "Gro,";
				if (type & KdCollider::TypeBump)   typePreview += "Bum,";
				if (type & KdCollider::TypeDamage) typePreview += "Dmg,";
				if (type & KdCollider::TypeSight)  typePreview += "Sgt,";
				if (type & KdCollider::TypeEvent)  typePreview += "Evt,";
				if (!typePreview.empty())		   typePreview.pop_back();
				if (typePreview.empty())		   typePreview = "None";

				if (ImGui::BeginCombo("##CollisionTypes", typePreview.c_str()))
				{
					bool isGround = (type & KdCollider::TypeGround);
					bool isBump = (type & KdCollider::TypeBump);
					bool isDamage = (type & KdCollider::TypeDamage);
					bool isSight = (type & KdCollider::TypeSight);
					bool isEvent = (type & KdCollider::TypeEvent);

					bool changed = false;
					if (ImGui::Checkbox("Ground", &isGround))		changed = true;
					if (ImGui::Checkbox("Bump", &isBump))		changed = true;
					if (ImGui::Checkbox("Damage", &isDamage))		changed = true;
					if (ImGui::Checkbox("Sight", &isSight))		changed = true;
					if (ImGui::Checkbox("Event", &isEvent))		changed = true;

					if (changed)
					{
						UINT newType = 0;
						if (isGround) newType |= KdCollider::TypeGround;
						if (isBump)   newType |= KdCollider::TypeBump;
						if (isDamage) newType |= KdCollider::TypeDamage;
						if (isSight)  newType |= KdCollider::TypeSight;
						if (isEvent)  newType |= KdCollider::TypeEvent;
						collider->SetCollisionType(newType);
					}
					ImGui::EndCombo();
				}
			}

			ImGui::Separator();

			if (isDynamicMode)
			{
				// Dynamic
				ImGui::Text("Character Shapes");

				// Sphere
				bool useSphere = collider->GetEnableSphere();
				if (ImGui::Checkbox("Sphere", &useSphere))
				{
					collider->SetEnableSphere(useSphere);
				}
				if (useSphere)
				{
					ImGui::Indent();
					float radius = collider->GetSphereRadius();
					if (ImGui::DragFloat("Radius", &radius, 0.1f, 0.0f))
					{
						collider->SetSphereRadius(radius);
					}
					ImGui::Unindent();
				}

				// Box
				bool useBox = collider->GetEnableBox();
				if (ImGui::Checkbox("Box", &useBox))
				{
					collider->SetEnableBox(useBox);
				}
				if (useBox)
				{
					ImGui::Indent();
					Math::Vector3 extents = collider->GetBoxExtents();
					if (ImGui::DragFloat3("Half Size", &extents.x, 0.1f, 0.0f))
					{
						collider->SetBoxExtents(extents);
					}
					ImGui::Unindent();
				}
			}
			else
			{
				// Static
				ImGui::Text("Mesh Collider");
				bool useModel = collider->GetEnableModel();
				if (ImGui::Checkbox("Model Collision", &useModel))
				{
					collider->SetEnableModel(useModel);
				}
			}
		}
	}
}

namespace EditorPanels
{
	void InspectorPanel::Draw(EditorManager& editor)
	{
		ImGui::Begin("Inspector");

		// EditorManagerから選択エンティティを取得
		auto sel = editor.GetSelectedEntity();

		if (sel)
		{
			// ---- 名前編集 ----
			char nameBuffer[256] = "";
			if (!sel->GetName().empty()) {
				strncpy_s(nameBuffer, sel->GetName().c_str(), _TRUNCATE);
			}

			if (ImGui::InputText("Name", nameBuffer, sizeof(nameBuffer)))
			{
				sel->SetName(nameBuffer);
			}
			ImGui::Separator();

			// 各コンポーネント描画
			DrawComponentTransform(sel);
			DrawComponentRender(sel);
			DrawComponentCollider(sel);

			// Action
			if (auto player = sel->GetComponent<ActionPlayerComponent>())
			{
				player->DrawInspector();
			}

			ImGui::Separator();

			// ---- Add Component ボタン ----
			if (ImGui::Button("Add Component"))
			{
				ImGui::OpenPopup("AddComponentPopup");
			}

			if (ImGui::BeginPopup("AddComponentPopup"))
			{
				// ポップアップ内で sel の生存確認 (念のため)
				// 既に持っていない場合のみ表示
				if (!sel->HasComponent<TransformComponent>())
				{
					if (ImGui::MenuItem("Transform"))
					{
						sel->AddComponent(std::make_shared<TransformComponent>());
					}
				}

				if (!sel->HasComponent<RenderComponent>())
				{
					if (ImGui::MenuItem("Render"))
					{
						auto render = std::make_shared<RenderComponent>();
						// デフォルトはStaticにしておく
						render->SetModelData("");
						sel->AddComponent(render);
					}
				}

				if (!sel->HasComponent<ColliderComponent>())
				{
					if (ImGui::MenuItem("Collider"))
					{
						sel->AddComponent(std::make_shared<ColliderComponent>());
					}
				}

				if (!sel->HasComponent<ActionPlayerComponent>())
				{
					if (ImGui::MenuItem("Action Player"))
					{
						sel->AddComponent(std::make_shared<ActionPlayerComponent>());
					}
				}

				ImGui::Separator();
				ImGui::TextDisabled("-- Presets --");

				// Presets (Render + Collider)
				if (!sel->HasComponent<RenderComponent>())
				{
					// Static Model
					if (ImGui::MenuItem("Stage Object"))
					{
						auto render = std::make_shared<RenderComponent>();
						render->SetModelData("");
						sel->AddComponent(render);

						if (!sel->HasComponent<ColliderComponent>())
						{
							auto collider = std::make_shared<ColliderComponent>();
							collider->SetEnableModel(true);
							collider->SetEnableSphere(false);
							collider->SetEnableBox(false);
							collider->SetCollisionType(KdCollider::TypeGround);
							sel->AddComponent(collider);
						}
						ImGui::CloseCurrentPopup();
					}

					// Dynamic Model
					if (ImGui::MenuItem("Character Object"))
					{
						auto render = std::make_shared<RenderComponent>();
						render->SetModelWork("");
						sel->AddComponent(render);

						if (!sel->HasComponent<ColliderComponent>())
						{
							auto collider = std::make_shared<ColliderComponent>();
							collider->SetEnableModel(false);
							collider->SetEnableSphere(true);
							collider->SetEnableBox(true);
							collider->SetCollisionType(KdCollider::TypeBump);
							sel->AddComponent(collider);
						}
						ImGui::CloseCurrentPopup();
					}
				}
				ImGui::EndPopup();
			}
		}
		else
		{
			ImGui::Text("No Selection");
		}

		ImGui::End();
	}
}
