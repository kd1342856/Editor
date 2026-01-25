#pragma once
// - エンティティリストの管理
// - 選択状態のハンドリング
// - ギズモとUIの描画

class CameraBase;
class EditorScene;
class EditorCamera;

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

private:
    // UI Helper Methods
    void DrawGameView();
    void DrawHierarchy();
    void DrawInspector();

    // Component UI Helpers
    void DrawComponentTransform(std::shared_ptr<Entity> sel);
    void DrawComponentRender(std::shared_ptr<Entity> sel);
    void DrawComponentCollider(std::shared_ptr<Entity> sel);
    
    // DrawFileBrowserPopup Removed

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

    // ImGuizmo
    int m_gizmoType = -1; // -1: None, or ImGuizmo::TRANSLATE etc.
    bool m_isGizmoUsing = false;
    Math::Matrix m_gizmoStartMatrix; // ドラッグ開始時の行列
};