#pragma once
#include "../ECS/Component.h"
#include "../ECS/Entity.h"
#include "../../Framework/Math/KdCollider.h"

class ColliderComponent : public Component
{
public:
	ColliderComponent() {}
	~ColliderComponent() override {}

	void Init() override;
	void Update() override; 
    void DrawDebug() override;       

    void SetEnableSphere(bool enable)					 { m_enableSphere = enable; m_isDirty = true; }
    bool GetEnableSphere() const						 { return m_enableSphere; }

    void SetEnableBox(bool enable)						 { m_enableBox = enable; m_isDirty = true; }
    bool GetEnableBox() const							 { return m_enableBox; }

    void SetEnableModel(bool enable)					 { m_enableModel = enable; m_isDirty = true; }
    bool GetEnableModel() const							 { return m_enableModel; }

    // スフィア
    void SetSphereRadius(float radius)					 { m_sphereRadius = radius; m_isDirty = true; }
    float GetSphereRadius() const						 { return m_sphereRadius; }

    // AABB
    void SetBoxExtents(const Math::Vector3& extents)	 { m_boxExtents = extents; m_isDirty = true; }
    const Math::Vector3& GetBoxExtents() const			 { return m_boxExtents; }

    void SetOffset(const Math::Vector3& offset)			 { m_offset = offset; m_isDirty = true; }
    const Math::Vector3& GetOffset() const				 { return m_offset; }

    void SetEnable(bool enable)							 { m_enable = enable; }
    bool IsEnable() const								 { return m_enable; }

    void SetDebugDrawEnabled(bool enable)				 { m_debugDraw = enable; }
    bool IsDebugDrawEnabled() const						 { return m_debugDraw; }
		
    void SetCollisionType(UINT type)					 { m_collisionType = type; m_isDirty = true; }
    UINT GetCollisionType() const						 { return m_collisionType; }

    bool Intersects(const KdCollider::RayInfo& target, std::list<KdCollider::CollisionResult>* pResults) const
    {
        if (!m_collider || !m_enable) return false;
        
        std::shared_ptr<Entity> spOwner = GetOwner();
        if (spOwner)
        {
             return m_collider->Intersects(target, spOwner->GetMatrix(), pResults);
        }
        return false;
    }

private:
    void RegisterShape(); 

    std::unique_ptr<KdCollider>			m_collider;
    std::unique_ptr<KdDebugWireFrame>	m_debugWire;
    
    // Config
    bool m_enableSphere	 = false;
    bool m_enableBox	 = false;
    bool m_enableModel	 = false;

    UINT m_collisionType = KdCollider::TypeBump; // Default to Bump

    // Parameters
    float m_sphereRadius		 = 1.0f;
    Math::Vector3 m_boxExtents	 = { 0.5f, 0.5f, 0.5f }; // Half-size
    Math::Vector3 m_offset		 = Math::Vector3::Zero;

    // State
    bool m_enable	 = true;
    bool m_debugDraw = true;
    bool m_isDirty	 = true;
};
