//#include "ImGuiAnimeDebug.h"
//
//void ImGuiAnimeDebug::Draw(AnimationController* controller)
//{
//	if (!controller) return;
//	// ------------------------------------
//	// デバッグモード：ImGuiによる制御 (Windowあり)
//	// ------------------------------------
//	if (ImGui::Begin("Animation Debug"))
//	{
//		DrawWidgets(controller);
//	}
//	ImGui::End();
//}
//
//void ImGuiAnimeDebug::DrawWidgets(AnimationController* controller)
//{
//	if (!controller) return;
//
//	ImGui::Text("Current State: %s", controller->GetCurrentStateName().c_str());
//
//	// ステート強制切替
//	if (ImGui::Button("Force Idle"))
//	{
//		controller->ChangeState(std::make_shared<PlayerIdleState>());
//	}
//	ImGui::SameLine();
//	if (ImGui::Button("Force Run"))
//	{
//		controller->ChangeState(std::make_shared<PlayerRunState>());
//	}
//
//	ImGui::Separator();
//
//	// アニメーション直接再生
//	if (auto* animator = controller->GetAnimator())
//	{
//		std::vector<std::string> animNames = animator->GetAnimationNames();
//		if (!animNames.empty())
//		{
//			// コンボボックス用の const char* 配列を作成
//			std::vector<const char*> items;
//			for (const auto& name : animNames) items.push_back(name.c_str());
//
//			ImGui::Combo("Animation", &m_selectedAnim, items.data(), (int)items.size());
//			ImGui::DragFloat("Rate", &m_playRate, 0.1f, 0.0f, 5.0f);
//
//			if (ImGui::Button("Play Selected"))
//			{
//				if (m_selectedAnim >= 0 && m_selectedAnim < animNames.size())
//				{
//					animator->Play(animNames[m_selectedAnim], true, m_playRate);
//				}
//			}
//		}
//		else
//		{
//			ImGui::Text("No animations found.");
//		}
//	}
//}
