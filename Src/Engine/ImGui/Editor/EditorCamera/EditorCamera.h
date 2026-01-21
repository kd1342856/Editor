#pragma once
#include "../../../../Application/GameObject/Camera/CameraBase.h"

class EditorCamera : public CameraBase
{
public:
	EditorCamera();
	~EditorCamera() override {}

	void Init() override;
	void Update() override;

	// 入力制御
	void SetInputEnabled(bool enable) { m_inputEnabled = enable; }
    
    // 注視点 (Focus Point) へのアクセス
    void SetFocusPoint(const Math::Vector3& focus) { m_focusPoint = focus; }
    const Math::Vector3& GetFocusPoint() const { return m_focusPoint; }
    
    // 注視点からの距離
    void SetDistance(float dist) { m_distance = dist; }

private:
	void UpdateMouseInput();

	bool m_inputEnabled = false;

	// オービット (軌道) パラメータ
	Math::Vector3 m_focusPoint = Math::Vector3::Zero;
	float m_distance = 10.0f;
    
    // マウス感度
    float m_rotateSpeed = 0.2f;
    float m_panSpeed = 0.05f;
    float m_zoomSpeed = 1.0f;

    // フライモード (WASD移動)
    bool m_isFlyMode = false;
    bool m_justEnteredFlyMode = false; // モード切り替え直後のフレーム判定用
    float m_moveSpeed = 5.0f; // フライモード時の移動速度
};
