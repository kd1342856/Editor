#include "EditorManager.h"
#include "../../Scene/SceneManager.h"
#include "../../Core/Thread/Profiler/Profiler.h"
#include "File/ImGuiFileBrowser.h"
#include "../../Serializer/SceneSerializer.h"
#include "EditorCamera/EditorCamera.h"
#include "Command/CommandManager.h"
#include "Command/CmdTransform.h"
#include "Command/CommandBase.h"
#include "Command/CommandBase.h"
#include "../../ECS/Entity/EntityManager.h"
#include "../../Render/RenderSystem.h"
#include "EditorUI/Panels/HierarchyPanel.h"
#include "EditorUI/Panels/InspectorPanel.h"

void EditorManager::Init()
{
	// ゲームビュー用レンダーターゲット作成 
	m_gameRT.CreateRenderTarget(1280, 720, true, DXGI_FORMAT_R8G8B8A8_UNORM);
	m_gameRT.ClearTexture();

    // エディタカメラ
    m_editorCamera = std::make_shared<EditorCamera>();
    m_editorCamera->Init();

    // パネル初期化
    m_hierarchyPanel = std::make_shared<EditorPanels::HierarchyPanel>();
    m_inspectorPanel = std::make_shared<EditorPanels::InspectorPanel>();

    m_isPlayerView = false;
    m_prevAltV = false;

    // ログ初期化
    Logger::Log("System", "Editor Initialized");
}

void EditorManager::Update()
{
	// Alt + V で視点切り替え
	bool currAlt = (GetAsyncKeyState(VK_MENU) & 0x8000) != 0;
	bool currV   = (GetAsyncKeyState('V')     & 0x8000) != 0;

    // Alt + V がそろった瞬間をトリガー
    // Alt + V がそろった瞬間をトリガー
    bool currTrigger = (currAlt && currV);

	if (currTrigger && !m_prevAltV)
	{
		m_isPlayerView = !m_isPlayerView;
        
        // カーソル制御: PlayerViewならマウスを掴んで非表示、EditorViewなら解放
        Engine::Instance().SetMouseGrabbed(m_isPlayerView);

        Logger::Log("EditorManager", "View Toggled! New State: " + std::to_string(m_isPlayerView));

        // プレイヤーコンポーネントを探してアクティブ状態を通知
        auto entities = EntityManager::Instance().GetEntityList();
        for (auto& entity : entities)
        {
            if (auto player = entity->GetComponent<ActionPlayerComponent>())
            {
                player->SetCameraActive(m_isPlayerView);
                break; 
            }
        }
	}
    m_prevAltV = currTrigger;
}

void EditorManager::Draw()
{
	// ゲームビューのクリア
	m_gameRT.ClearTexture(Math::Color(0.0f, 0.0f, 1.0f, 1.0f)); 

	// ゲームビュー用レンダーターゲットに切り替え
	m_rtChanger.ChangeRenderTarget(m_gameRT);

    // カメラセットアップ
    std::shared_ptr<CameraBase> activeCam = m_editorCamera;
    
    if (m_isPlayerView)
    {
        // プレイヤーを探す
        auto entities = EntityManager::Instance().GetEntityList();
        for (auto& entity : entities)
        {
            if (auto player = entity->GetComponent<ActionPlayerComponent>())
            {
                auto cam = player->GetCamera();
                if (cam)
                {
                    activeCam = cam;
                    // PlayerView時はカメラ行列再計算(PostUpdate)はActionPlayerComponent::Updateで既に呼ばれているはず
                    // しかし、描画行列のセットはここで行う必要がある(PreDraw or SetToShader)
                }
                break;
            }
        }
    }

	// シェーダーへのカメラ情報更新
    if (activeCam)
    {
        // View/Proj行列をシェーダーへ転送
         activeCam->PreDraw();
    }
    
	// 重要: 描画前にライティング定数を更新！
	KdShaderManager::Instance().WorkAmbientController().Draw();

    // SceneManager / EntityManagerから描画対象を収集してRenderSystemに送信
    // Game View用に再描画
    RenderSystem::Instance().BeginFrame();
    
    const auto& entities = EntityManager::Instance().GetEntityList();
    for (const auto& entity : entities)
    {
        RenderSystem::Instance().Submit(entity);
    }
    
    // 3D描画実行
    RenderSystem::Instance().Execute3D();
    
    // デバッグ描画
    RenderSystem::Instance().ExecuteDebug();

	// レンダーターゲットを復元 (バックバッファまたは以前のものへ)
	m_rtChanger.UndoRenderTarget();
}

// ユニーク名生成
std::string EditorManager::GetUniqueName(const std::string& baseName)
{
    std::string name = baseName;
    int count = 0;
    const auto& entities = EntityManager::Instance().GetEntityList();

    // 重複チェック
    auto checkDuplicate = [&](const std::string& n) 
		{
        for (const auto& entity : entities) {
            if (entity->GetName() == n) return true;
        }
        return false;
    };

    while (checkDuplicate(name))
    {
        count++;
        name = baseName + " (" + std::to_string(count) + ")";
    }
    
    return name;
}

/*
void EditorManager::DrawHierarchy()
{
   // Logic moved to HierarchyPanel
}
*/

void EditorManager::DrawUI()
{
	// フレーム開始
	ImGuizmo::BeginFrame();

    if (m_isPlayerView)
    {
        // マウスカーソルを強制的に隠す (OSカーソルはUpdateで掴んでいるが、ImGuiカーソルも消す)
        ImGui::SetMouseCursor(ImGuiMouseCursor_None);

        // 背景描画リストに直接描画 (Docking関係なく最背面へ -> Dockingの背景に隠れるため最前面へ変更)
        ImGuiViewport* viewport = ImGui::GetMainViewport();
        if (m_gameRT.m_RTTexture)
        {
            ImGui::GetForegroundDrawList()->AddImage(
                (ImTextureID)m_gameRT.m_RTTexture->GetSRView(),
                viewport->Pos,
                { viewport->Pos.x + viewport->Size.x, viewport->Pos.y + viewport->Size.y }
            );
        }
        
        // UI描画をスキップして終了
        // ただし、ImGuiのフレーム終了処理 (Render) は ImGuiManager が行うので、ここではこれ以上何もしない
        return; 
    }

	// メインメニューバー
	if (!m_isPlayerView)
	{
		if (ImGui::BeginMainMenuBar())
		{
			if (ImGui::BeginMenu("File"))
			{
				if (ImGui::MenuItem("Save Scene"))
				{
					// カメラ情報の保存を試みる
					std::shared_ptr<CameraBase> cam = m_camera.lock();
					if (!cam) cam = m_editorCamera; // フォールバック

					std::string currentScene = SceneManager::Instance().GetCurrentSceneName();
					if (!currentScene.empty())
					{
						std::string path = "Asset/Data/Scene/" + currentScene + ".json";
						const auto& entities = EntityManager::Instance().GetEntityList();
						SceneSerializer::Save(path, entities, cam);
					}
				}
				if (ImGui::MenuItem("Load Scene"))
				{
					// File Browserを開く
					ImGuiFileBrowser::Instance().Open(
						"LoadScene",
						"Load Scene JSON",
						{ ".json" },
						[this](const std::string& path)
						{
							std::shared_ptr<CameraBase> cam = m_camera.lock();
							if (!cam) cam = m_editorCamera;

							std::vector<std::shared_ptr<Entity>> loadedEntities;
							bool res = SceneSerializer::Load(path, loadedEntities, cam);
							if (res)
							{
								EntityManager::Instance().ClearEntities();
								for (auto& entity : loadedEntities) EntityManager::Instance().AddEntity(entity);
							}
						});
				}
				ImGui::EndMenu();
			}

			// Sceneメニュー
			if (ImGui::BeginMenu("Scene"))
			{
				std::string currentScene = SceneManager::Instance().GetCurrentSceneName();

				std::vector<std::string> scenes = SceneManager::Instance().GetSceneNames();
				for (const std::string& name : scenes)
				{
					bool isSelected = (name == currentScene);
					if (ImGui::MenuItem(name.c_str(), NULL, isSelected))
					{
						SceneManager::Instance().ChangeScene(name);
					}
				}

				ImGui::Separator();

				if (ImGui::MenuItem("Create New Scene"))
				{
					ImGui::OpenPopup("CreateNewScenePopup");
				}

				ImGui::EndMenu();
			}

			ImGui::EndMainMenuBar();
		}
	}

	if (!m_isPlayerView)
	{
		if (ImGui::BeginPopupModal("CreateNewScenePopup", NULL, ImGuiWindowFlags_AlwaysAutoResize))
		{
			static char buf[64] = "";
			ImGui::InputText("Scene Name", buf, 64);

			if (ImGui::Button("Create", ImVec2(120, 0)))
			{
				if (strlen(buf) > 0)
				{
					SceneManager::Instance().CreateScene(buf);
					ImGui::CloseCurrentPopup();
					memset(buf, 0, sizeof(buf));
				}
			}
			ImGui::SetItemDefaultFocus();
			ImGui::SameLine();
			if (ImGui::Button("Cancel", ImVec2(120, 0)))
			{
				ImGui::CloseCurrentPopup();
			}
			ImGui::EndPopup();
		}

		DrawGameView();

		if (!m_isPlayerView)
		{
			// DrawHierarchy();
			if (m_hierarchyPanel) m_hierarchyPanel->Draw(*this);
			// DrawInspector();
			if (m_inspectorPanel) m_inspectorPanel->Draw(*this);

			// Logger描画
			Logger::DrawImGui();

			// プロファイラ描画
			Profiler::Instance().DrawProfilerWindow();

			// ImGuiFileDialog描画
			ImGuiFileBrowser::Instance().Draw();
		}

		// デバッグ情報 (一時的)
		if (ImGui::Begin("Debug Info"))
		{
			ImGui::Text("Player View: %s", m_isPlayerView ? "ON" : "OFF");
			bool alt = (GetAsyncKeyState(VK_MENU) & 0x8000) != 0;
			bool v = (GetAsyncKeyState('V') & 0x8000) != 0;
			bool p = (GetAsyncKeyState('P') & 0x8000) != 0;
			bool trigger = (alt && v) || p;
			ImGui::Text("Input: Alt=%d, V=%d, P=%d", alt, v, p);
			ImGui::Text("Trigger: %d, Prev: %d", trigger, m_prevAltV);

			bool foundPlayer = false;
			auto entities = EntityManager::Instance().GetEntityList();
			for (auto& entity : entities)
			{
				if (entity->GetComponent<ActionPlayerComponent>())
				{
					foundPlayer = true;
					break;
				}
			}
			ImGui::Text("Player Component Found: %s", foundPlayer ? "YES" : "NO");
		}
		ImGui::End();
	}
}
void EditorManager::DrawGameView()
{
	// ゲームビューウィンドウ
	bool isOpen = true;
	ImGuiWindowFlags flags = 0;

	if (m_isPlayerView)
	{
		// 全画面モード設定
		ImGuiViewport* viewport = ImGui::GetMainViewport();
		ImGui::SetNextWindowPos(viewport->Pos);
		ImGui::SetNextWindowSize(viewport->Size);
		ImGui::SetNextWindowViewport(viewport->ID);

		flags |= ImGuiWindowFlags_NoDecoration;
		flags |= ImGuiWindowFlags_NoMove;
		flags |= ImGuiWindowFlags_NoResize;
		flags |= ImGuiWindowFlags_NoBringToFrontOnFocus;
		flags |= ImGuiWindowFlags_NoNavFocus;
		flags |= ImGuiWindowFlags_NoDocking; // ドッキングも無効化
	}

	if (ImGui::Begin("Game View", &isOpen, flags))
	{
		// カメラ制御のためにフォーカスチェック
		if (m_editorCamera)
		{
			// ウィンドウがフォーカスされている時のみ入力を許可
			m_editorCamera->SetInputEnabled(ImGui::IsWindowFocused());

			// カメラロジック更新 (入力処理)
			// プレイヤー視点時はエディタカメラを動かさない
			if (!m_isPlayerView)
			{
				m_editorCamera->Update();
			}
		}

		// ウィンドウ内にゲーム画面テクスチャを描画
		if (m_gameRT.m_RTTexture)
		{
			// 利用可能サイズを取得
			ImVec2 availSize = ImGui::GetContentRegionAvail();

			// 16:9 アスペクト比
			float targetAspect = 16.0f / 9.0f;
			float availAspect = availSize.x / availSize.y;

			ImVec2 imageSize;
			if (availAspect > targetAspect)
			{
				// 横長すぎる -> 高さに合わせる
				imageSize.y = availSize.y;
				imageSize.x = imageSize.y * targetAspect;
			}
			else
			{
				// 縦長すぎる -> 幅に合わせる
				imageSize.x = availSize.x;
				imageSize.y = imageSize.x / targetAspect;
			}

			// 中央寄せ
			float offsetX = (availSize.x - imageSize.x) * 0.5f;
			float offsetY = (availSize.y - imageSize.y) * 0.5f;

			ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX() + offsetX, ImGui::GetCursorPosY() + offsetY));

			// 描画
			ImGui::Image((void*)m_gameRT.m_RTTexture->GetSRView(), imageSize);
		}

		// ---- ImGuizmo 処理 ----
		// 選択中のエンティティがあればギズモを表示
		auto sel = m_selectedEntity.lock();
		if (sel && m_editorCamera)
		{
			ImGuizmo::SetDrawlist();

			// ウィンドウの位置とサイズを ImGuizmo に教える
			ImVec2 windowPos = ImGui::GetWindowPos();
			ImVec2 contentMin = ImGui::GetWindowContentRegionMin();
			ImVec2 contentMax = ImGui::GetWindowContentRegionMax();
			ImGui::PushClipRect(ImVec2(windowPos.x + contentMin.x, windowPos.y + contentMin.y),
				ImVec2(windowPos.x + contentMax.x, windowPos.y + contentMax.y), true);

			// GameView の矩形範囲 (タイトルバーなどを考慮)
			ImVec2 vMin = ImGui::GetWindowContentRegionMin();
			ImVec2 vMax = ImGui::GetWindowContentRegionMax();
			vMin.x += windowPos.x;
			vMin.y += windowPos.y;
			vMax.x += windowPos.x;
			vMax.y += windowPos.y;

			ImGuizmo::SetRect(vMin.x, vMin.y, vMax.x - vMin.x, vMax.y - vMin.y);

			// カメラ行列の取得
			Math::Matrix viewMat = Math::Matrix::Identity;
			Math::Matrix projMat = Math::Matrix::Identity;

			std::shared_ptr<CameraBase> activeCam = m_editorCamera;
			if (m_isPlayerView)
			{
				// プレイヤーを探す
				auto entities = EntityManager::Instance().GetEntityList();
				for (auto& entity : entities)
				{
					if (auto player = entity->GetComponent<ActionPlayerComponent>())
					{
						auto cam = player->GetCamera();
						if (cam)
						{
							activeCam = cam;
						}
						break;
					}
				}
			}

			if (activeCam && activeCam->GetCamera())
			{
				viewMat = activeCam->GetCamera()->GetCameraViewMatrix();
				projMat = activeCam->GetCamera()->GetProjMatrix();
			}

			// Blenderライク ショートカット
			if (!ImGui::IsMouseDown(ImGuiMouseButton_Right)) // カメラ操作中でない時
			{
				if (ImGui::IsKeyPressed(ImGuiKey_G)) m_gizmoType = ImGuizmo::TRANSLATE;
				if (ImGui::IsKeyPressed(ImGuiKey_R)) m_gizmoType = ImGuizmo::ROTATE;
				if (ImGui::IsKeyPressed(ImGuiKey_S)) m_gizmoType = ImGuizmo::SCALE;
			}

			// ギズモ有効時
			if (m_gizmoType != -1 && sel->HasComponent<TransformComponent>())
			{
				auto trans = sel->GetComponent<TransformComponent>();

				// 現在の Transform から行列を作成
				Math::Matrix worldMat = trans->GetWorldMatrix();
				Math::Vector3 pos = trans->GetPosition();
				Math::Vector3 rot = trans->GetRotation();
				Math::Vector3 scale = trans->GetScale();

				Math::Matrix rotMat = Math::Matrix::CreateFromYawPitchRoll(
					DirectX::XMConvertToRadians(rot.y),
					DirectX::XMConvertToRadians(rot.x),
					DirectX::XMConvertToRadians(rot.z));

				worldMat = Math::Matrix::CreateScale(scale) * rotMat * Math::Matrix::CreateTranslation(pos);

				// 操作実行
				bool manipulated = ImGuizmo::Manipulate(
					&viewMat._11, &projMat._11,
					(ImGuizmo::OPERATION)m_gizmoType,
					ImGuizmo::LOCAL, // or WORLD
					&worldMat._11);

				// --- Undo/Redo Logic for Gizmo ---
				bool isUsing = ImGuizmo::IsUsing();

				// 使用開始: 現在の行列を保存
				if (isUsing && !m_isGizmoUsing)
				{
					m_gizmoStartMatrix = worldMat; // 操作前の行列
				}

				// 使用終了: コマンド生成
				if (!isUsing && m_isGizmoUsing)
				{
					// コマンド作成
					auto cmd = std::make_shared<CmdTransform>(sel, m_gizmoStartMatrix, worldMat);
					CommandManager::Instance().Execute(cmd);
				}

				m_isGizmoUsing = isUsing;

				if (manipulated)
				{
					// 操作されたら値を戻す
					float matrixTranslation[3], matrixRotation[3], matrixScale[3];
					ImGuizmo::DecomposeMatrixToComponents(&worldMat._11, matrixTranslation, matrixRotation, matrixScale);

					trans->SetPosition({ matrixTranslation[0]	, matrixTranslation[1]	, matrixTranslation[2] });
					trans->SetRotation({ matrixRotation[0]	, matrixRotation[1]		, matrixRotation[2] });
					trans->SetScale({ matrixScale[0]		, matrixScale[1]		, matrixScale[2] });
				}
			}
			// クリップ矩形の復元
			ImGui::PopClipRect();
		}

		// --- Undo/Redo
		if (ImGui::IsWindowFocused())
		{
			if (ImGui::GetIO().KeyCtrl)
			{
				if (ImGui::IsKeyPressed(ImGuiKey_Z))
				{
					CommandManager::Instance().Undo();
				}
				if (ImGui::IsKeyPressed(ImGuiKey_Y))
				{
					CommandManager::Instance().Redo();
				}
			}
		}
	}
	ImGui::End();
}


void EditorManager::SetCameras(const std::shared_ptr<CameraBase>& tps, const std::shared_ptr<CameraBase>& build, const std::shared_ptr<CameraBase>& editor)
{
    m_camera = editor;
}