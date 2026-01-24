#include "EditorManager.h"
#include "EditorUI/EditorUI.h" 
#include "../../Core/Thread/Profiler/Profiler.h"
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
    auto e = std::make_shared<Entity>();    
    auto tc = std::make_shared<TransformComponent>();
    // 位置 0,0,0
    e->AddComponent(tc);

    auto rc = std::make_shared<RenderComponent>();
    e->AddComponent(rc);

    e->Init();
    
    m_entities.push_back(e);

    // エディタカメラ
    m_editorCamera = std::make_shared<EditorCamera>();
    m_editorCamera->Init();

    // ログ初期化 (特になくても動くが、起動ログ出す)
    Logger::Log("System", "Editor Initialized");
}

void EditorManager::Update()
{
    for (auto& e : m_entities)
    {
        e->Update();
    }
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
        // 描画前にスムーズな移動を保証するためにここで更新
         // 本来はUpdateループで呼ぶべきだが、エディタ入力の同期のため、DrawUIでフォーカスチェックをするのが適切
         // しかしシェーダーには今すぐ行列が必要
         m_editorCamera->WorkCamera()->SetToShader();
    }
    
	// 重要: 描画前にライティング定数を更新！
	KdShaderManager::Instance().WorkAmbientController().Draw();

    // 全エンティティ描画
    KdShaderManager::Instance().m_StandardShader.BeginLit();
    for (auto& e : m_entities)
    {
        // デバッグ: 描画動作確認のために単純なグリッドを描画
        e->DrawLit();

        // コライダーのデバッグ描画 (暫定: ここで描画)
        // 本来は e->DrawDebug() などを設けるべきだが、今回は Components を直接見るか、
        // Entity にデバッグ描画用関数を追加する
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

    // 重複チェック
    auto checkDuplicate = [&](const std::string& n) {
        for (const auto& e : m_entities) {
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

                SceneSerializer::Save("Asset/Data/Scene/Scene.json", m_entities, cam);
            }
            if (ImGui::MenuItem("Load Scene"))
            {
                std::shared_ptr<CameraBase> cam = m_camera.lock();
                if (!cam) cam = m_editorCamera;

                // ロード実行
                bool res = SceneSerializer::Load("Asset/Data/Scene/Scene.json", m_entities, cam);
                if (!res) 
				{
                     // 失敗時のログなどは内部で出ているはずだが、念のため
                }
            }
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }

    DrawGameView();
    DrawHierarchy();
    DrawInspector();
    
    // Logger描画
    Logger::DrawImGui();

    // プロファイラ描画
    Profiler::Instance().DrawProfilerWindow();
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
                m_entities.push_back(e);
            }

			ImGui::Separator();

            ImGui::EndPopup();
        }

        // 削除処理 (Deleteキー)
        if (ImGui::IsWindowFocused() && ImGui::IsKeyPressed(ImGuiKey_Delete))
        {
            if (auto selected = m_selectedEntity.lock())
            {
                // vectorから削除
                auto it = std::remove(m_entities.begin(), m_entities.end(), selected);
                if (it != m_entities.end())
                {
                    m_entities.erase(it, m_entities.end());
                    m_selectedEntity.reset(); // 選択解除
                }
            }
        }

        // エンティティ一覧
        for (int i=0; i<m_entities.size(); ++i)
        {
            auto& e = m_entities[i];
            
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
                ImGui::OpenPopup("FileBrowserPopup");
            }
            
            std::string selectedFile;
            if (DrawFileBrowserPopup("FileBrowserPopup", selectedFile, {".gltf"}))
            {
                if (rc->IsDynamic()) rc->SetModelWork(selectedFile);
                else rc->SetModelData(selectedFile);
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

// ファイルブラウザポップアップ描画 
bool EditorManager::DrawFileBrowserPopup(const std::string& popupName, std::string& outPath, const std::vector<std::string>& extensions)
{
    bool selected = false;
    
    if (ImGui::BeginPopup(popupName.c_str()))
    {
        static std::filesystem::path currentPath = "Asset";
        
        // ディレクトリ表示
        ImGui::Text("Dir: %s", currentPath.string().c_str());
        if (currentPath != "Asset")
        {
            if (ImGui::Button(".."))
            {
                currentPath = currentPath.parent_path();
            }
        }
        ImGui::Separator();

        // ファイルグリッド描画
        float padding = 16.0f;
        float thumbnailSize = 64.0f;
        float cellSize = thumbnailSize + padding;

        float panelWidth = ImGui::GetContentRegionAvail().x;
        int columnCount = (int)(panelWidth / cellSize);
        if (columnCount < 1) columnCount = 1;

        if (ImGui::BeginTable("FileBrowserGrid", columnCount))
        {
            if (std::filesystem::exists(currentPath) && std::filesystem::is_directory(currentPath))
            {
                for (const auto& entry : std::filesystem::directory_iterator(currentPath))
                {
                    const auto& path = entry.path();
                    std::string filename = path.filename().string();
                    bool isDir = entry.is_directory();
                    std::string ext = path.extension().string();

                    // フィルタリング (ディレクトリは常に表示、ファイルは拡張子一致のみ)
                    if (!isDir && !extensions.empty())
                    {
                        bool match = false;
                        for (const auto& targetExt : extensions) 
						{
                            if (ext == targetExt) 
							{
                                match = true;
                                break;
                            }
                        }
                        if (!match) continue; // 対象外のファイルはスキップ
                    }

                    ImGui::TableNextColumn();
                    ImGui::PushID(path.string().c_str());
                    
                    // アイコン描画
                    ImVec4 color = isDir ? ImVec4(0.8f, 0.7f, 0.2f, 1.0f) : ImVec4(0.5f, 0.5f, 0.5f, 1.0f);
                    ImGui::PushStyleColor(ImGuiCol_Button, color);

                    if (ImGui::Button("##Icon", ImVec2(thumbnailSize, thumbnailSize)))
                    {
                    }

                    // ドラッグ＆ドロップソース
                    if (!isDir && ImGui::BeginDragDropSource())
                    {
                        std::string pathStr = path.string();
                        std::replace(pathStr.begin(), pathStr.end(), '\\', '/');
                        ImGui::SetDragDropPayload("CONTENT_BROWSER_ITEM", pathStr.c_str(), pathStr.length() + 1);
                        ImGui::Text("%s", filename.c_str());
                        ImGui::EndDragDropSource();
                    }

                    // ダブルクリック判定
                    if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
                    {
                        if (isDir) 
						{
                            currentPath = path;
                        } 
						else 
						{
                            std::string selectedPath = path.string();
                            std::replace(selectedPath.begin(), selectedPath.end(), '\\', '/');
                            outPath = selectedPath;
                            selected = true;
                            ImGui::CloseCurrentPopup();
                        }
                    }

                    ImGui::PopStyleColor();
                    ImGui::TextWrapped("%s", filename.c_str());
                    ImGui::PopID();
                }
            }
            ImGui::EndTable();
        }
        ImGui::EndPopup();
    }
    
    return selected;
}

void EditorManager::SetCameras(const std::shared_ptr<CameraBase>& tps, const std::shared_ptr<CameraBase>& build, const std::shared_ptr<CameraBase>& editor)
{
    m_camera = editor;
}