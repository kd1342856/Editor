#include "TransformComponent.h"

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
