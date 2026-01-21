#include "EditorManager.h"
#include "EditorUI/EditorUI.h" 
#include "../../Components/TransformComponent.h"
#include "../../Components/RenderComponent.h"
#include "../../Components/ColliderComponent.h"
#include "imgui/imgui.h"

void EditorManager::Init()
{
	// ゲームビュー用レンダーターゲット作成 (デフォルト 720p)
	m_gameRT.CreateRenderTarget(1280, 720, true, DXGI_FORMAT_R8G8B8A8_UNORM);
	m_gameRT.ClearTexture(Math::Color(0, 0, 0, 1)); // 黒でクリア

    // 最低限のテストシーン
    auto e = std::make_shared<Entity>();
    e->SetName("TestCube");
    
    auto tc = std::make_shared<TransformComponent>();
    // 位置 0,0,0
    e->AddComponent(tc);

    auto rc = std::make_shared<RenderComponent>();
    // rc->SetModelData("Asset/Models/Terrain/Box/Box.gltf"); // サンプルBox - クラッシュ回避のためコメントアウト
    e->AddComponent(rc);

    e->Init();
    
    m_entities.push_back(e);

    // エディタカメラ
    m_editorCamera = std::make_shared<EditorCamera>();
    m_editorCamera->Init();
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
    DrawGameView();
    DrawHierarchy();
    DrawInspector();
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
			// 将来的なリサイズのために利用可能サイズを取得（現在は固定）
			ImVec2 size = ImGui::GetContentRegionAvail();
			
			// アスペクト比を維持するかストレッチするか？とりあえず埋める
			ImGui::Image((void*)m_gameRT.m_RTTexture->GetSRView(), size);
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

            if (ImGui::MenuItem("Create Cube"))
            {
                auto e = std::make_shared<Entity>();
                e->SetName(GetUniqueName("Cube"));
                e->AddComponent(std::make_shared<TransformComponent>());
                
                auto rc = std::make_shared<RenderComponent>();
                rc->SetModelData("Asset/Models/Terrain/Box/Box.gltf");
                e->AddComponent(rc);

                auto cc = std::make_shared<ColliderComponent>();
                cc->SetEnableBox(true); // Box On
                cc->SetBoxExtents({0.5f, 0.5f, 0.5f});
                e->AddComponent(cc);

                e->Init();
                m_entities.push_back(e);
            }

            if (ImGui::MenuItem("Create Sphere"))
            {
                auto e = std::make_shared<Entity>();
                e->SetName(GetUniqueName("Sphere"));
                e->AddComponent(std::make_shared<TransformComponent>());
                
                // Render (PrimitiveがなければBoxで代用 or 空)
                auto rc = std::make_shared<RenderComponent>();
                // rc->SetModelData("Asset/Models/Primitive/Sphere.gltf");
                e->AddComponent(rc);

                // Collider
                auto cc = std::make_shared<ColliderComponent>();
                cc->SetEnableSphere(true); // Sphere On
                cc->SetSphereRadius(0.5f);
                e->AddComponent(cc);

                e->Init();
                m_entities.push_back(e);
            }

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
			// ポップアップ内で sel の生存確認 (念のため)
			if (!sel->HasComponent<RenderComponent>())
			{
				// Static Model (Stage/Terrain)
				if (ImGui::MenuItem("Render (Stage/Static)"))
				{
					auto rc = std::make_shared<RenderComponent>();
					rc->SetModelData(""); // Initialize as Static, empty path
					sel->AddComponent(rc);

					// Auto-add Collider (Static Defaults)
					if (!sel->HasComponent<ColliderComponent>()) {
						auto cc = std::make_shared<ColliderComponent>();
						// Static: Enable Model, Disable Sphere/Box
						cc->SetEnableModel(true); 
						cc->SetEnableSphere(false);
						cc->SetEnableBox(false);
						cc->SetCollisionType(KdCollider::TypeGround); // Default to Ground for Stage
						sel->AddComponent(cc);
					}
					ImGui::CloseCurrentPopup();
				}

				// Dynamic Model (Character/Player)
				if (ImGui::MenuItem("Render (Character/Dynamic)"))
				{
					auto rc = std::make_shared<RenderComponent>();
					rc->SetModelWork(""); // Initialize as Dynamic, empty path
					sel->AddComponent(rc);

					// Auto-add Collider (Dynamic Defaults)
					if (!sel->HasComponent<ColliderComponent>()) {
						auto cc = std::make_shared<ColliderComponent>();
						// Dynamic: Disable Model, Enable Sphere/Box
						cc->SetEnableModel(false);
						cc->SetEnableSphere(true);  // Default On
						cc->SetEnableBox(true);     // Default On
						cc->SetCollisionType(KdCollider::TypeBump); // Fixed to Bump
						sel->AddComponent(cc);
					}
					ImGui::CloseCurrentPopup();
				}
			}

			// 他のコンポーネント...
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
        bool enable = cc->IsEnable();
        if (ImGui::Checkbox("Enable", &enable)) {
            cc->SetEnable(enable);
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

        // ---- Collision Types (Layers) ----
        ImGui::Text("Collision Types");
        
        if (isDynamicMode)
        {
            // Dynamic: Fixed to Bump (as per request)
            ImGui::TextDisabled("Fixed: Bump");
            // 内部値が違っていたら強制的に直す？ユーザーが"unchangeable"と言ったのでUI操作不可にする。
            if (cc->GetCollisionType() != KdCollider::TypeBump) {
                // ここで直すと毎フレームSetが走る可能性があるので、表示だけFixに見せておくか、
                // あるいは初期化時に設定したものを信じる。
                // ユーザー要望「Unchangeable」なので、変更UIを出さないだけでOK。
            }
        }
        else
        {
            // Static: Type Selection Only
            UINT type = cc->GetCollisionType();
            
            // Preview
            std::string typePreview = "";
            if (type & KdCollider::TypeGround) typePreview += "Gro,";
            if (type & KdCollider::TypeBump)   typePreview += "Bum,";
            if (type & KdCollider::TypeDamage) typePreview += "Dmg,";
            if (type & KdCollider::TypeSight)  typePreview += "Sgt,";
            if (type & KdCollider::TypeEvent)  typePreview += "Evt,";
            if (!typePreview.empty()) typePreview.pop_back();
            if (typePreview.empty())  typePreview = "None";

            if (ImGui::BeginCombo("##CollisionTypes", typePreview.c_str()))
            {
                bool isGround = (type & KdCollider::TypeGround);
                bool isBump   = (type & KdCollider::TypeBump);
                bool isDamage = (type & KdCollider::TypeDamage);
                bool isSight  = (type & KdCollider::TypeSight);
                bool isEvent  = (type & KdCollider::TypeEvent);

                bool changed = false;
                if (ImGui::Checkbox("Ground", &isGround)) changed = true;
                if (ImGui::Checkbox("Bump", &isBump)) changed = true;
                if (ImGui::Checkbox("Damage", &isDamage)) changed = true;
                if (ImGui::Checkbox("Sight", &isSight)) changed = true;
                if (ImGui::Checkbox("Event", &isEvent)) changed = true;

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

        // ---- Shapes ----
        
        if (isDynamicMode)
        {
            // Dynamic (Character): Sphere / Box Only. Model is disabled/hidden.
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

            // Force disable model if it was somehow enabled
            if (cc->GetEnableModel()) cc->SetEnableModel(false);
        }
        else
        {
            // Static (Stage): Model Only. Sphere/Box hidden/disabled.
            ImGui::Text("Stage Mesh");
            ImGui::Text("Mesh Collision: Enabled (Always)");
            
            // Force enable model
            if (!cc->GetEnableModel()) cc->SetEnableModel(true);
            if (cc->GetEnableSphere()) cc->SetEnableSphere(false);
            if (cc->GetEnableBox())    cc->SetEnableBox(false);

            // No other shapes to configure for now
        }

        ImGui::Separator();
        
        // Common Offset (Only show for Dynamic? Static usually uses model origin)
        if (isDynamicMode)
        {
            Math::Vector3 offset = cc->GetOffset();
            if (ImGui::DragFloat3("Offset", &offset.x, 0.1f)) {
                cc->SetOffset(offset);
            }
        }
    }
}

// ファイルブラウザポップアップ描画 (汎用)
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
                        for (const auto& targetExt : extensions) {
                            if (ext == targetExt) {
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
                        // Single click
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
                        if (isDir) {
                            currentPath = path;
                        } else {
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