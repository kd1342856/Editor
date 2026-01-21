#pragma once

#include "../../ECS/Entity.h"
#include "../../../Framework/Shader/KdRenderTargetChange.h"
#include "EditorCamera/EditorCamera.h"
#include <filesystem>

// 最小限のエディタマネージャ
// - エンティティリストの管理
// - 選択状態のハンドリング
// - ギズモとUIの描画

class CameraBase;
class EditorScene;

class EditorManager
{
public:
	void Init();
	void Update();
	void Draw(); // シーン描画

	// UI描画 (ImGui)
	void DrawUI();

    // ユニーク名生成ヘルパー
    std::string GetUniqueName(const std::string& baseName);

    // アクセサ
	std::vector<std::shared_ptr<Entity>>& GetEntities() { return m_entities; }
    void AddEntity(const std::shared_ptr<Entity>& e) { m_entities.push_back(e); }

    // カメラへのアクセス
    void SetCameras(const std::shared_ptr<CameraBase>& tps, const std::shared_ptr<CameraBase>& build, const std::shared_ptr<CameraBase>& editor);

private:
    // UI Helper Methods
    void DrawGameView();
    void DrawHierarchy();
    void DrawInspector();

    // Component UI Helpers
    void DrawComponentTransform(std::shared_ptr<Entity> sel);
    void DrawComponentRender(std::shared_ptr<Entity> sel);
    void DrawComponentCollider(std::shared_ptr<Entity> sel);
    
    // File Browser
    bool DrawFileBrowserPopup(const std::string& popupName, std::string& outPath, const std::vector<std::string>& extensions = {});

	std::vector<std::shared_ptr<Entity>> m_entities;
    std::weak_ptr<Entity> m_selectedEntity;
    
    // エディタカメラ (参照)
    std::weak_ptr<CameraBase> m_camera;
    // 実体を持つエディタカメラ
    std::shared_ptr<EditorCamera> m_editorCamera;
    
    // 単純な状態管理
    bool m_isEditorMode = true;

    // ゲームビュー用レンダーターゲット
    KdRenderTargetPack m_gameRT;
    KdRenderTargetChanger m_rtChanger;
};