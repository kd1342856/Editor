#include "EditorManager.h"
#include "EditorUI/EditorUI.h" 
#include "../../Components/ActionPlayerComponent.h"
#include "../../Scene/SceneManager.h"
#include "../../Core/Thread/Profiler/Profiler.h"
#include "File/ImGuiFileBrowser.h"
#include "../../Serializer/SceneSerializer.h"
#include "EditorCamera/EditorCamera.h"
#include "Command/CommandManager.h"
#include "Command/CmdTransform.h"
#include "Command/CommandBase.h"

void EditorManager::Init()
{
	// ゲームビュー用レンダーターゲット作成 (デフォルト 720p)
	m_gameRT.CreateRenderTarget(1280, 720, true, DXGI_FORMAT_R8G8B8A8_UNORM);
	m_gameRT.ClearTexture(Math::Color(0, 0, 0, 1)); // 黒でクリア

    // 最低限のテストシーン
    // NOTE: SceneManager will handle scene initialization. EditorManager shouldn't force create entities unless it's in a blank state.
    // For now, let SceneManager handle initial entities via Scene loading.

    // エディタカメラ
    m_editorCamera = std::make_shared<EditorCamera>();
    m_editorCamera->Init();

    // ログ初期化 (特になくても動くが、起動ログ出す)
    Logger::Log("System", "Editor Initialized");
}

void EditorManager::Update()
{
    // NO-OP: SceneManager updates entities. EditorManager just observes or manipulates via UI.
}

void EditorManager::Draw()
{
	// ゲームビューのクリア (バックバッファの期待に合わせて標準的な青を使用)
	m_gameRT.ClearTexture(Math::Color(0.0f, 0.0f, 1.0f, 1.0f)); 

	// ゲームビュー用レンダーターゲットに切り替え
	m_rtChanger.ChangeRenderTarget(m_gameRT);

	// シェーダーへのカメラ情報更新
    if (m_editorCamera)
    {
         m_editorCamera->WorkCamera()->SetToShader();
    }
    
	// 重要: 描画前にライティング定数を更新！
	KdShaderManager::Instance().WorkAmbientController().Draw();

    // 全エンティティ描画 (SceneManagerから取得)
    const auto& entities = SceneManager::Instance().GetEntityList();

    KdShaderManager::Instance().m_StandardShader.BeginLit();
    for (auto& e : entities)
    {
        e->DrawLit();

        auto cc = e->GetComponent<ColliderComponent>();
        if (cc)
        {
            cc->DrawDebug();
        }
    }
    KdShaderManager::Instance().m_StandardShader.EndLit();

	// レンダーターゲットを復元 (バックバッファまたは以前のものへ)
	m_rtChanger.UndoRenderTarget();
}

// ユニーク名生成
std::string EditorManager::GetUniqueName(const std::string& baseName)
{
    std::string name = baseName;
    int count = 0;
    const auto& entities = SceneManager::Instance().GetEntityList();

    // 重複チェック
    auto checkDuplicate = [&](const std::string& n) {
        for (const auto& e : entities) {
            if (e->GetName() == n) return true;
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

void EditorManager::DrawHierarchy()
{
    if (ImGui::Begin("Hierarchy"))
    {
        // 作成用の右クリックコンテキストメニュー
        if (ImGui::BeginPopupContextWindow("HierarchyContextMenu"))
        {
            if (ImGui::MenuItem("Create Empty Object"))
            {
                auto e = std::make_shared<Entity>();
                e->SetName(GetUniqueName("Empty Object"));
                e->AddComponent(std::make_shared<TransformComponent>());
                e->Init();
                SceneManager::Instance().AddEntity(e);
            }

			ImGui::Separator();

            ImGui::EndPopup();
        }

        const auto& entities = SceneManager::Instance().GetEntityList();

        // 削除処理 (Deleteキー)
        if (ImGui::IsWindowFocused() && ImGui::IsKeyPressed(ImGuiKey_Delete))
        {
            if (auto selected = m_selectedEntity.lock())
            {
                // Remove from SceneManager
                SceneManager::Instance().RemoveEntity(selected);
                m_selectedEntity.reset(); // 選択解除
            }
        }

        // エンティティ一覧
        auto displayEntities = entities; 
        
        for (int i=0; i<displayEntities.size(); ++i)
        {
            auto& e = displayEntities[i];
            
            // ID重複回避のために ##Address を付与
            std::string label = e->GetName() + "##" + std::to_string((uintptr_t)e.get());
            
            if(ImGui::Selectable(label.c_str(), m_selectedEntity.lock() == e))
            {
                m_selectedEntity = e;
            }
        }
    }
    ImGui::End();
}

void EditorManager::DrawUI()
{
    // フレーム開始
    ImGuizmo::BeginFrame();

    // メインメニューバー
    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            // 名前付きで保存とかは後で。とりあえず固定パス
            if (ImGui::MenuItem("Save Scene"))
            {
                // カメラ情報の保存を試みる（なくても保存はする）
                std::shared_ptr<CameraBase> cam = m_camera.lock();
                if (!cam) cam = m_editorCamera; // フォールバック

                std::string currentScene = SceneManager::Instance().GetCurrentSceneName();
                if (!currentScene.empty())
                {
                    std::string path = "Asset/Data/Scene/" + currentScene + ".json";
                    const auto& entities = SceneManager::Instance().GetEntityList();
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
                            SceneManager::Instance().ClearEntities();
                            for(auto& e : loadedEntities) SceneManager::Instance().AddEntity(e);
                            
                            // シーン名をファイル名から推測して更新してもいいかも？
                            // SceneManager::Instance().ChangeScene(std::filesystem::path(path).stem().string());
                        }
                    });
            }
            ImGui::EndMenu();
        }

        // Scene Menu
        if (ImGui::BeginMenu("Scene"))
        {
            std::string currentScene = SceneManager::Instance().GetCurrentSceneName();

            // List all scenes
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

    // Modal for Creating New Scene
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
                memset(buf, 0, sizeof(buf)); // Reset buffer
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
    DrawHierarchy();
    DrawInspector();
    
    // Logger描画
    Logger::DrawImGui();

    // プロファイラ描画
    Profiler::Instance().DrawProfilerWindow();

    // File Browser Draw
    ImGuiFileBrowser::Instance().Draw();
}

void EditorManager::DrawGameView()
{
	// ゲームビューウィンドウ
	if (ImGui::Begin("Game View"))
	{
        // カメラ制御のためにフォーカスチェック
        if (m_editorCamera)
        {
            // ウィンドウがフォーカスされている時のみ入力を許可
            m_editorCamera->SetInputEnabled(ImGui::IsWindowFocused());
            
            // カメラロジック更新 (入力処理)
            m_editorCamera->Update();
        }

		// ウィンドウ内にゲーム画面テクスチャを描画
		if (m_gameRT.m_RTTexture)
		{
			// 利用可能サイズを取得
			ImVec2 availSize = ImGui::GetContentRegionAvail();
			
			// 16:9 のアスペクト比を計算
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
            // ImGui::Image が描画された領域に合わせるのがベストだが、
            // 簡易的にウィンドウサイズを使う
            ImVec2 vMin = ImGui::GetWindowContentRegionMin();
            ImVec2 vMax = ImGui::GetWindowContentRegionMax();
            vMin.x += windowPos.x;
            vMin.y += windowPos.y;
            vMax.x += windowPos.x;
            vMax.y += windowPos.y;

            ImGuizmo::SetRect(vMin.x, vMin.y, vMax.x - vMin.x, vMax.y - vMin.y);

            // カメラ行列の取得
            const Math::Matrix& viewMat = m_editorCamera->GetCamera()->GetCameraViewMatrix(); // View行列
            const Math::Matrix& projMat = m_editorCamera->GetCamera()->GetProjMatrix();

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
                auto tc = sel->GetComponent<TransformComponent>();
                
                // 現在の Transform から行列を作成
                Math::Matrix worldMat = tc->GetWorldMatrix(); // SRT合成済み行列を貰うか、自分で作る
                // TransformComponent::GetWorldMatrix() はキャッシュかもしれんので都度計算する方が安全かも
                // ここでは tc->GetPosition() などから再構築
                Math::Vector3 pos = tc->GetPosition();
                Math::Vector3 rot = tc->GetRotation();
                Math::Vector3 scale = tc->GetScale();
                
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
                    // 変化があった場合のみコマンド登録
                    // (浮動小数点の誤差を考慮してもいいが、単純比較でも一旦OK)
                    // if (memcmp(&m_gizmoStartMatrix, &worldMat, sizeof(Math::Matrix)) != 0) 
                    // Matrixの比較演算子 != があればそれを使う
                    
                    // ここでの worldMat は「操作後」の値になっているはずだが、
                    // Manipulate関数は「描画＆入力反映」を行うため、
                    // IsUsing() が false になったフレームでは worldMat は「最終確定値」である。
                    
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

                    tc->SetPosition({ matrixTranslation[0], matrixTranslation[1], matrixTranslation[2] });
                    tc->SetRotation({ matrixRotation[0], matrixRotation[1], matrixRotation[2] });
                    tc->SetScale({ matrixScale[0], matrixScale[1], matrixScale[2] });
                }
            }
            // クリップ矩形の復元
            ImGui::PopClipRect();
        }

        // --- Undo/Redo Shortcuts (Game View Focused) ---
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


void EditorManager::DrawInspector()
{
    ImGui::Begin("Inspector");

    auto sel = m_selectedEntity.lock();
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
		if (auto apc = sel->GetComponent<ActionPlayerComponent>())
		{
			apc->DrawInspector();
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
					auto rc = std::make_shared<RenderComponent>();
					// デフォルトはStaticにしておく
					rc->SetModelData(""); 
					sel->AddComponent(rc);
				}
			}

			if (!sel->HasComponent<ColliderComponent>())
			{
				if (ImGui::MenuItem("Collider"))
				{
					sel->AddComponent(std::make_shared<ColliderComponent>());
				}
			}

			// Action Components
			if (!sel->HasComponent<ActionPlayerComponent>())
			{
				if (ImGui::MenuItem("Action Player"))
				{
					sel->AddComponent(std::make_shared<ActionPlayerComponent>());
				}
			}

			ImGui::Separator();
			ImGui::TextDisabled("-- Presets --");

			// Presets (Render + Collider combos)
			if (!sel->HasComponent<RenderComponent>())
			{
				// ... (既存の便利セット)
				// Static Model (Stage/Terrain)
				if (ImGui::MenuItem("Stage Object"))
				{
					auto rc = std::make_shared<RenderComponent>();
					rc->SetModelData(""); 
					sel->AddComponent(rc);

					if (!sel->HasComponent<ColliderComponent>()) {
						auto cc = std::make_shared<ColliderComponent>();
						cc->SetEnableModel(true); 
						cc->SetEnableSphere(false);
						cc->SetEnableBox(false);
						cc->SetCollisionType(KdCollider::TypeGround);
						sel->AddComponent(cc);
					}
					ImGui::CloseCurrentPopup();
				}

				// Dynamic Model (Character/Player)
				if (ImGui::MenuItem("Character Object"))
				{
					auto rc = std::make_shared<RenderComponent>();
					rc->SetModelWork(""); 
					sel->AddComponent(rc);

					if (!sel->HasComponent<ColliderComponent>()) {
						auto cc = std::make_shared<ColliderComponent>();
						cc->SetEnableModel(false);
						cc->SetEnableSphere(true);  
						cc->SetEnableBox(true);     
						cc->SetCollisionType(KdCollider::TypeBump);
						sel->AddComponent(cc);
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

void EditorManager::DrawComponentTransform(std::shared_ptr<Entity> sel)
{
    if (!sel->HasComponent<TransformComponent>()) return;

    if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen))
    {
        auto tc = sel->GetComponent<TransformComponent>();
        if (tc)
        {
            // Enable Toggle
            bool enable = tc->IsEnable();
            if (ImGui::Checkbox("Enable##Transform", &enable)) tc->SetEnable(enable);

            Math::Vector3 pos = tc->GetPosition();
            Math::Vector3 rot = tc->GetRotation();
            Math::Vector3 scale = tc->GetScale();

            if (ImGui::DragFloat3("Position", &pos.x, 0.1f)) tc->SetPosition(pos);
            if (ImGui::DragFloat3("Rotation", &rot.x, 0.1f)) tc->SetRotation(rot);
            if (ImGui::DragFloat3("Scale", &scale.x, 0.1f)) tc->SetScale(scale);
        }
    }
}

void EditorManager::DrawComponentRender(std::shared_ptr<Entity> sel)
{
    if (!sel->HasComponent<RenderComponent>()) return;

    if (ImGui::CollapsingHeader("Render", ImGuiTreeNodeFlags_DefaultOpen))
    {
        auto rc = sel->GetComponent<RenderComponent>();
        if (rc)
        {
            // Enable Toggle
            bool enable = rc->IsEnable();
            if (ImGui::Checkbox("Enable##Render", &enable)) rc->SetEnable(enable);

            // モデルパス編集
            char pathBuffer[MAX_PATH] = "";
            if (!rc->GetModelPath().empty()) {
                strncpy_s(pathBuffer, rc->GetModelPath().c_str(), _TRUNCATE);
            }

            // InputText
            if (ImGui::InputText("Model Path", pathBuffer, sizeof(pathBuffer), ImGuiInputTextFlags_EnterReturnsTrue))
            {
                if (rc->IsDynamic()) rc->SetModelWork(pathBuffer);
                else rc->SetModelData(pathBuffer);
            }
            ImGui::SameLine();
            if (ImGui::Button("...##ModelSelect"))
            {
                ImGuiFileBrowser::Instance().Open(
                    "SelectModel", 
                    "Select Model File", 
                    { ".gltf", ".glb", ".obj", ".fbx" }, // Common model formats
                    [rc](const std::string& path) 
                    {
                        // Convert absolute path to relative if possible? 
                        // For now, use full path or maybe just filename if in specific dir. 
                        // Let's assume path is usable. 
                        // Actually, Asset system usually expects relative path from Asset/
                        // But FileBrowser returns full path. 
                        // Let's try to make it relative to "Current Directory" if run from project root.
                        // std::filesystem::relative(path, std::filesystem::current_path()).string();

                        // Simple hack: if path contains "Asset", substring it.
                        // Ideally we should use std::filesystem::relative.
                        std::string setPath = path;
                        try {
                            setPath = std::filesystem::relative(path, std::filesystem::current_path()).string();
                        } catch (...) {}

                        if (rc->IsDynamic()) rc->SetModelWork(setPath);
                        else rc->SetModelData(setPath);
                    });
            }

            // ドラッグ＆ドロップ (ターゲット)
            if (ImGui::BeginDragDropTarget())
            {
                if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
                {
                    const char* path = (const char*)payload->Data;
                    if (rc->IsDynamic())
                        rc->SetModelWork(path);
                    else
                        rc->SetModelData(path);
                }
                ImGui::EndDragDropTarget();
            }
            
            // Dynamic Toggle
            bool isDynamic = rc->IsDynamic();
            if (ImGui::Checkbox("Dynamic (Animation)", &isDynamic))
            {
                // Re-load with new mode
                if (isDynamic)
                    rc->SetModelWork(rc->GetModelPath());
                else
                    rc->SetModelData(rc->GetModelPath());
            }

            // Reload & File Select
            if (ImGui::Button("Reload"))
            {
                if (rc->IsDynamic()) rc->SetModelWork(rc->GetModelPath());
                else rc->SetModelData(rc->GetModelPath());
            }
            ImGui::SameLine();
            
            if (ImGui::Button("..."))
            {
                ImGuiFileBrowser::Instance().Open(
                    "ModelSelectPopup",
                    "Select Model",
                    { ".gltf", ".glb", ".obj", ".fbx" },
                    [rc](const std::string& path)
                    {
                        // リソースパスの相対化などは適宜
                        std::string setPath = path;
                        try {
                            setPath = std::filesystem::relative(path, std::filesystem::current_path()).string();
                        } catch(...) {}

                        if (rc->IsDynamic()) rc->SetModelWork(setPath);
                        else rc->SetModelData(setPath);
                    });
            }
        }
    }
}

void EditorManager::DrawComponentCollider(std::shared_ptr<Entity> sel)
{
    if (!sel->HasComponent<ColliderComponent>()) return;

    if (ImGui::CollapsingHeader("Collider", ImGuiTreeNodeFlags_DefaultOpen))
    {
        auto cc = sel->GetComponent<ColliderComponent>();

        // Enable / Disable
        bool enable = cc->IsEnable(); // uses Component::IsEnable if not shadowed, or derived
        if (ImGui::Checkbox("Enable##Collider", &enable)) {
            cc->SetEnable(enable); // uses virtual or derived
        }

        ImGui::SameLine();

        // Debug Draw Toggle
        bool debug = cc->IsDebugDrawEnabled();
        if (ImGui::Checkbox("Show Wireframe", &debug)) {
            cc->SetDebugDrawEnabled(debug);
        }

        ImGui::Separator();
        
        // RenderComponentの状態を確認
        bool isDynamicMode = false;
        if (auto rc = sel->GetComponent<RenderComponent>())
        {
            isDynamicMode = rc->IsDynamic();
        }

        // ---- Collision Types ----
        ImGui::Text("Collision Types");
        
        if (isDynamicMode)
        {
            // Dynamic
            ImGui::TextDisabled("Fixed: Bump");
            if (cc->GetCollisionType() != KdCollider::TypeBump)
			{

            }
        }
        else
        {
            // Static
            UINT type = cc->GetCollisionType();
            
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
                bool isBump   = (type & KdCollider::TypeBump);
                bool isDamage = (type & KdCollider::TypeDamage);
                bool isSight  = (type & KdCollider::TypeSight);
                bool isEvent  = (type & KdCollider::TypeEvent);

                bool changed = false;
                if (ImGui::Checkbox("Ground",	&isGround))		changed = true;
                if (ImGui::Checkbox("Bump"	,	&isBump))		changed = true;
                if (ImGui::Checkbox("Damage",	&isDamage))		changed = true;
                if (ImGui::Checkbox("Sight"	,	&isSight))		changed = true;
                if (ImGui::Checkbox("Event"	,	&isEvent))		changed = true;

                if (changed)
                {
                    UINT newType = 0;
                    if (isGround) newType |= KdCollider::TypeGround;
                    if (isBump)   newType |= KdCollider::TypeBump;
                    if (isDamage) newType |= KdCollider::TypeDamage;
                    if (isSight)  newType |= KdCollider::TypeSight;
                    if (isEvent)  newType |= KdCollider::TypeEvent;
                    cc->SetCollisionType(newType);
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
            bool useSphere = cc->GetEnableSphere();
            if (ImGui::Checkbox("Sphere", &useSphere)) {
                cc->SetEnableSphere(useSphere);
            }
            if (useSphere)
            {
                ImGui::Indent();
                float radius = cc->GetSphereRadius();
                if (ImGui::DragFloat("Radius", &radius, 0.1f, 0.0f)) {
                    cc->SetSphereRadius(radius);
                }
                ImGui::Unindent();
            }

            // Box
            bool useBox = cc->GetEnableBox();
            if (ImGui::Checkbox("Box", &useBox)) {
                cc->SetEnableBox(useBox);
            }
            if (useBox)
            {
                ImGui::Indent();
                Math::Vector3 extents = cc->GetBoxExtents();
                if (ImGui::DragFloat3("Half Size", &extents.x, 0.1f, 0.0f)) {
                    cc->SetBoxExtents(extents);
                }
                ImGui::Unindent();
            }


            if (cc->GetEnableModel()) cc->SetEnableModel(false);
        }
        else
        {
            ImGui::Text("Stage Mesh");
            ImGui::Text("Mesh Collision: Enabled (Always)");
            
            if (!cc->GetEnableModel()) cc->SetEnableModel(true);
            if (cc->GetEnableSphere()) cc->SetEnableSphere(false);
            if (cc->GetEnableBox())    cc->SetEnableBox(false);

        }

        ImGui::Separator();
        
        if (isDynamicMode)
        {
            Math::Vector3 offset = cc->GetOffset();
            if (ImGui::DragFloat3("Offset", &offset.x, 0.1f)) {
                cc->SetOffset(offset);
            }
        }
    }
}


void EditorManager::SetCameras(const std::shared_ptr<CameraBase>& tps, const std::shared_ptr<CameraBase>& build, const std::shared_ptr<CameraBase>& editor)
{
    m_camera = editor;
}