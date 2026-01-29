#include "ColliderComponent.h"
#include "../../Serializer/JsonUtils.h"

using json = nlohmann::json;

void ColliderComponent::Init()
{
	
    m_collider = std::make_unique<KdCollider>();
    m_debugWire = std::make_unique<KdDebugWireFrame>();
    RegisterShape();
}

void ColliderComponent::Update()
{
    if (m_isDirty)
    {
        RegisterShape();
        m_isDirty = false;
    }
}

void ColliderComponent::RegisterShape()
{
    if (!m_collider) return;

    // まず古い形状を解放する手立てが必要だが、KdColliderにはClear機能が無いかもしれない。
    // RegisterCollisionShape は map に追加していくので、同じ名前で登録すれば上書きされるか、
    // あるいは毎回作り直すのが安全かもしれないが...
    // KdColliderの実装を見ると unordered_map<string, ...> なので名前がキー。
    // 名前を変えて管理する必要がある。
    
    // とりあえず "Sphere" と "Box" で登録し、無効な場合は登録しない（というよりEnableフラグ制御が必要だが、
    // KdColliderにRemove機能がない場合、登録しっぱなしになる可能性がある。
    // 今回は簡易的に、フラグを見て再生成(Reset)する。）
    
    m_collider = std::make_unique<KdCollider>(); // Reset everything

    if (m_enableSphere)
    {
        m_collider->RegisterCollisionShape("Sphere", m_offset, m_sphereRadius, m_collisionType);
    }
    
    if (m_enableBox)
    {
        DirectX::BoundingBox box;
        box.Center = m_offset;
        box.Extents = m_boxExtents;
        m_collider->RegisterCollisionShape("Box", box, m_collisionType);
    }

    if (m_enableModel)
    {
        auto rc = GetOwner()->GetComponent<RenderComponent>();
        if (rc)
        {
            // Try Static Data first (Common for terrain)
            auto modelData = rc->GetModelData();
            if (modelData)
            {
                 m_collider->RegisterCollisionShape("Model", modelData, m_collisionType);
            }
            else
            {
                // Fallback to Work if only Work exists (though SetModel ensures Data exists)
                auto modelWork = rc->GetModelWork();
                if (modelWork)
                {
                    m_collider->RegisterCollisionShape("Model", modelWork, m_collisionType);
                }
            }
        }
    }
}

void ColliderComponent::DrawDebug()
{
    if (!m_enable || !m_debugDraw) return;

    // Get Owner's World Matrix
    Math::Matrix worldMat = GetOwner()->GetMatrix();

    if (m_enableSphere)
    {
        // Position: World Pos + Rotated Offset
        Math::Vector3 finalPos = Math::Vector3::Transform(m_offset, worldMat);
        
		m_debugWire->AddDebugSphere(finalPos, m_sphereRadius, {0.0f, 1.0f, 0.0f, 1.0f});
    }
    
    if (m_enableBox)
	{   
		m_debugWire->AddDebugBox(
            worldMat, 
            m_boxExtents, 
            m_offset, 
            true, 
            {0.0f, 1.0f, 0.0f, 1.0f}
        );
    }
    
    // 描画実行
    m_debugWire->Draw();
}

void ColliderComponent::Serialize(json& j) const
{
	j = json{
		{ "Enable",         m_enable },
		{ "DebugDraw",      m_debugDraw },
		{ "CollisionType",  m_collisionType },
		
		{ "EnableSphere",   m_enableSphere },
		{ "SphereRadius",   m_sphereRadius },
		
		{ "EnableBox",      m_enableBox },
		{ "BoxExtents",     m_boxExtents },
		
		{ "EnableModel",    m_enableModel },
		
		{ "Offset",         m_offset }
	};
}

void ColliderComponent::Deserialize(const json& j)
{
	// 値取り出し。無ければ初期値(または現状維持)
	m_enable			= j.value("Enable", m_enable);
	m_debugDraw			= j.value("DebugDraw", m_debugDraw);
	m_collisionType		= j.value("CollisionType", m_collisionType);

	m_enableSphere		= j.value("EnableSphere", m_enableSphere);
	m_sphereRadius		= j.value("SphereRadius", m_sphereRadius);

	m_enableBox			= j.value("EnableBox", m_enableBox);
	m_boxExtents		= j.value("BoxExtents", m_boxExtents); // Uses JsonUtils

	m_enableModel		= j.value("EnableModel", m_enableModel);

	m_offset			= j.value("Offset", m_offset); // Uses JsonUtils

	m_isDirty = true;
}
