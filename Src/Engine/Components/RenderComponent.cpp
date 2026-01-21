#include "RenderComponent.h"
#include "TransformComponent.h"
#include "../ECS/Entity.h"

void RenderComponent::Init()
{
}

void RenderComponent::DrawLit()
{
	if (!m_modelWork && !m_modelData) return;
	
	// Access Owner's Transform
	if (auto owner = GetOwner())
	{
		if (auto transform = owner->GetComponent<TransformComponent>())
		{
			if(m_modelWork)
			{
				KdShaderManager::Instance().m_StandardShader.DrawModel(*m_modelWork, transform->GetWorldMatrix());
			}
			
			if(m_modelData)
			{
				KdShaderManager::Instance().m_StandardShader.DrawModel(*m_modelData, transform->GetWorldMatrix());
			}
		}
	}
}

void RenderComponent::SetModel(const std::string& filePath)
{
	m_filePath = filePath;
    
    if (filePath.empty())
    {
        m_modelData = nullptr;
        m_modelWork = nullptr;
        return;
    }

	// 静的データを読み込む
	m_modelData = std::make_shared<KdModelData>();
	m_modelData->Load(filePath);
	// ダイナミックの場合のみWorkを
	if (m_isDynamic)
	{
		m_modelWork = std::make_shared<KdModelWork>();
		m_modelWork->SetModelData(filePath);
	}
}
