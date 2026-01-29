#include "RenderComponent.h"

using json = nlohmann::json;

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
				// 非同期ロードでモデルデータの中身が入れ替わった場合、
				// Work側のノードリストとサイズが合わなくなるので再セットアップする
				if (m_modelWork->GetNodes().size() != m_modelWork->GetDataNodes().size())
				{
					m_modelWork->SetModelData(m_modelWork->GetData());
				}

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

	// KdAssetsからデータを取得 (非同期ロード対応)
	m_modelData = KdAssets::Instance().m_modeldatas.GetData(filePath);

	// ダイナミックの場合のみWorkを
	if (m_isDynamic)
	{
		m_modelWork = std::make_shared<KdModelWork>();
		m_modelWork->SetModelData(m_modelData);
	}
}

void RenderComponent::Serialize(json& j) const
{
	j = json{
		{ "ModelPath", m_filePath },
		{ "IsDynamic", m_isDynamic }
	};
}

void RenderComponent::Deserialize(const json& j)
{
	std::string path = j.value("ModelPath", "");
	bool isDynamic   = j.value("IsDynamic", false);

	if (isDynamic)
	{
		SetModelWork(path);
	}
	else
	{
		SetModelData(path);
	}
}
