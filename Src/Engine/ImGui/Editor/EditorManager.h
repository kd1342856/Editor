#pragma once
// - エンティティリストの管理
// - 選択状態のハンドリング
// - ギズモとUIの描画

class CameraBase;
class EditorScene;
class EditorCamera;
namespace EditorPanels {
	class HierarchyPanel;
	class InspectorPanel;
}

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

    // カメラへのアクセス
    void SetCameras(const std::shared_ptr<CameraBase>& tps, const std::shared_ptr<CameraBase>& build, const std::shared_ptr<CameraBase>& editor);

	void SetSelectedEntity(const std::shared_ptr<Entity>& entity) { m_selectedEntity = entity; }
	std::shared_ptr<Entity> GetSelectedEntity() const { return m_selectedEntity.lock(); }

private:
    void DrawGameView();
    // void DrawHierarchy(); // Moved to HierarchyPanel
    // void DrawInspector(); // Moved to InspectorPanel

    

    std::weak_ptr<Entity> m_selectedEntity;

    // エディタカメラ (参照)
    std::weak_ptr<CameraBase> m_camera;
    // 実体を持つエディタカメラ
    std::shared_ptr<EditorCamera> m_editorCamera;
    
    // 単純な状態管理
    bool m_isEditorMode = true;
    bool m_isPlayerView = false;
    bool m_prevAltV = false;

    // ゲームビュー用レンダーターゲット
    KdRenderTargetPack m_gameRT;
    KdRenderTargetChanger m_rtChanger;

    // ImGuizmo
    int m_gizmoType = -1;
    bool m_isGizmoUsing = false;
    Math::Matrix m_gizmoStartMatrix;

    // Panels
    std::shared_ptr<EditorPanels::HierarchyPanel> m_hierarchyPanel;
    std::shared_ptr<EditorPanels::InspectorPanel> m_inspectorPanel;
};