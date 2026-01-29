#include "TransformComponent.h"
#include "../../Serializer/JsonUtils.h"

using json = nlohmann::json;

const Math::Matrix& TransformComponent::GetWorldMatrix()
{
	Math::Matrix S = Math::Matrix::CreateScale(m_scale);
	Math::Matrix R = Math::Matrix::CreateFromYawPitchRoll(
		DirectX::XMConvertToRadians(m_rotation.y),
		DirectX::XMConvertToRadians(m_rotation.x),
		DirectX::XMConvertToRadians(m_rotation.z)
	);
	Math::Matrix T = Math::Matrix::CreateTranslation(m_position);

	m_worldMatrix = S * R * T;
	return m_worldMatrix;
}

void TransformComponent::Serialize(json& j) const
{
	j = json{
		{ "Position", m_position },
		{ "Rotation", m_rotation },
		{ "Scale",    m_scale }
	};
}

void TransformComponent::Deserialize(const json& j)
{
	m_position = j.value("Position", Math::Vector3::Zero);
	m_rotation = j.value("Rotation", Math::Vector3::Zero);
	m_scale    = j.value("Scale",    Math::Vector3::One);
}
