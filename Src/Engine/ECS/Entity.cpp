#include "Entity.h"

void Entity::Init()
{
	for (auto& [type, comp] : m_components)
	{
		comp->Init();
	}
}

void Entity::Update()
{
	if (!m_visible) return;
	for (auto& [type, comp] : m_components)
	{
		if (!comp->IsEnable()) continue;
		comp->Update();
	}
}

void Entity::PostUpdate()
{
	if (!m_visible) return;
	for (auto& [type, comp] : m_components)
	{
		if (!comp->IsEnable()) continue;
		comp->PostUpdate();
	}
}

void Entity::PreDraw()
{
	if (!m_visible) return;
	for (auto& [type, comp] : m_components)
	{
		if (!comp->IsEnable()) continue;
		comp->PreDraw();
	}
}

void Entity::DrawLit()
{
	if (!m_visible || !IsVisible(VisibilityFlags::Lit)) return;
	for (auto& [type, comp] : m_components)
	{
		if (!comp->IsEnable()) continue;
		comp->DrawLit();
	}
}

void Entity::DrawUnLit()
{
	if (!m_visible || !IsVisible(VisibilityFlags::UnLit)) return;
	for (auto& [type, comp] : m_components)
	{
		if (!comp->IsEnable()) continue;
		comp->DrawUnLit();
	}
}

void Entity::DrawBright()
{
	if (!m_visible || !IsVisible(VisibilityFlags::Bright)) return;
	for (auto& [type, comp] : m_components)
	{
		if (!comp->IsEnable()) continue;
		comp->DrawBright();
	}
}

void Entity::GenerateDepthMapFromLight()
{
	if (!m_visible || !IsVisible(VisibilityFlags::Shadow)) return;
	for (auto& [type, comp] : m_components)
	{
		if (!comp->IsEnable()) continue;
		comp->GenerateDepthMapFromLight();
	}
}

void Entity::DrawSprite()
{
	if (!m_visible) return;
	for (auto& [type, comp] : m_components)
	{
		if (!comp->IsEnable()) continue;
		comp->DrawSprite();
	}
}

void Entity::DrawInspector()
{
	// ImGui logic will be separate or delegated
	for (auto& [type, comp] : m_components)
	{
		comp->DrawInspector();
	}
}

void Entity::SetVisibility(VisibilityFlags flag, bool enabled)
{
	if (enabled)
	{
		m_visibilityFlags |= static_cast<uint8_t>(flag);
	}
	else
	{
		m_visibilityFlags &= ~static_cast<uint8_t>(flag);
	}
}

bool Entity::IsVisible(VisibilityFlags flag) const
{
	return (m_visibilityFlags & static_cast<uint8_t>(flag)) != 0;
}

Math::Matrix Entity::GetMatrix() const
{
	auto tc = GetComponent<TransformComponent>();
	if (tc)
	{
		return tc->GetWorldMatrix();
	}
	return Math::Matrix::Identity;
}

void Entity::DrawDebug()
{
	if (!m_visible) return;
	for (auto& [type, comp] : m_components)
	{
		if (!comp->IsEnable()) continue;
		comp->DrawDebug();
	}
}
