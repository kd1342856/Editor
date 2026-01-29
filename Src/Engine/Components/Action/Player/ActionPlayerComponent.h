#pragma once

// プレイヤーの操作・挙動を管理するコンポーネント
class ActionPlayerComponent : public Component
{
public:
	ActionPlayerComponent() {}
	~ActionPlayerComponent() override {}

	void Init() override;
	void Update() override;
	void DrawInspector() override; // パラメータ調整用

	// Serialization
	void Serialize(nlohmann::json& j) const override;
	void Deserialize(const nlohmann::json& j) override;

	const char* GetType() const override { return "ActionPlayer"; }

private:
	// パラメータ
	float m_speed = 5.0f;       // 移動速度
	float m_jumpPower = 8.0f;   // ジャンプ力
    float m_gravity = 0.3f;     // 重力

    // 状態
    Math::Vector3 m_uGravity = Math::Vector3::Zero; // 現在の重力ベクトル（累積）
    bool m_isGround = false;    // 接地フラグ
};
