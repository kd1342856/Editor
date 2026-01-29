#pragma once

class TransformComponent : public Component
{
public:
	TransformComponent() {}
	~TransformComponent() override {}

	// --- Accessors ---
	void SetPosition(const Math::Vector3& pos) { m_position = pos; }
	void SetRotation(const Math::Vector3& rot) { m_rotation = rot; }
	void SetScale(const Math::Vector3& scale)  { m_scale = scale; }

	const Math::Vector3& GetPosition() const { return m_position; }
	const Math::Vector3& GetRotation() const { return m_rotation; }
	const Math::Vector3& GetScale() const    { return m_scale; }

	// Get World Matrix (Calculated from SRT)
	const Math::Matrix& GetWorldMatrix();

	void Serialize(nlohmann::json& j) const override;
	void Deserialize(const nlohmann::json& j) override;

	const char* GetType() const override { return "Transform"; }

private:
	Math::Vector3 m_position = Math::Vector3::Zero;
	Math::Vector3 m_rotation = Math::Vector3::Zero; // Degrees
	Math::Vector3 m_scale    = Math::Vector3::One;

	Math::Matrix  m_worldMatrix = Math::Matrix::Identity;
};
