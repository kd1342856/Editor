#pragma once
class TPSCamera;
class FPSCamera;

// プレイヤーの操作・挙動を管理するコンポーネント
class ActionPlayerComponent : public Component
{
public:
	enum class CameraMode
	{
		FPS,
		TPS
	};

	ActionPlayerComponent() {}
	~ActionPlayerComponent() override {}

	void Init() override;
	void Update() override;
	void DrawInspector() override; // パラメータ調整用

	// Serialization
	void Serialize(nlohmann::json& j) const override;
	void Deserialize(const nlohmann::json& j) override;

	const char* GetType() const override { return "ActionPlayer"; }

	// Camera Control
	void SetCameraActive(bool active) { m_isCameraActive = active; }
	std::shared_ptr<CameraBase> GetCamera() const;

private:
	// パラメータ
	float m_speed = 5.0f;       // 移動速度
	float m_jumpPower = 8.0f;   // ジャンプ力
    float m_gravity = 0.3f;     // 重力

    // 状態
    Math::Vector3 m_uGravity = Math::Vector3::Zero; // 現在の重力ベクトル（累積）
    bool m_isGround = false;    // 接地フラグ

	// Camera
	std::shared_ptr<FPSCamera> m_fpsCamera = nullptr;
	std::shared_ptr<TPSCamera> m_tpsCamera = nullptr;
	CameraMode m_cameraMode = CameraMode::TPS;
	bool m_isCameraActive = false;
	bool m_prevV = false;
};
