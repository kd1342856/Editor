#pragma once

class Entity;

class CameraBase
{
public:
	CameraBase() {}
	virtual ~CameraBase() {}

	virtual void Init();
	virtual void Update(); // Added Update
	virtual void PreDraw();
	virtual void PostUpdate(){}
	
	// Setters
	void SetTarget(const std::weak_ptr<Entity>& target);
	void SetPosition(const Math::Vector3& pos) { m_mWorld.Translation(pos); }
	void SetRotation(const Math::Vector3& rot) { m_DegAng = rot; }
	void SetWorldMatrix(const Math::Matrix& m) { m_mWorld = m; }
    void SetActive(bool active) { m_active = active; } // Added Active state

	// Accessors
	const std::shared_ptr<KdCamera>& GetCamera() const { return m_spCamera; }
	std::shared_ptr<KdCamera> WorkCamera() const { return m_spCamera; }
	const Math::Matrix& GetWorldMatrix() const { return m_mWorld; }
	Math::Vector3 GetPosition() const { return m_mWorld.Translation(); }
	Math::Vector3 GetEulerDeg() const { return m_DegAng; } // Added for Editor use
    bool IsActive() const { return m_active; }
    void SetEulerDeg(const Math::Vector3& deg) { m_DegAng = deg; } // Added for Editor use

	const Math::Matrix GetRotationMatrix()const
	{
		return Math::Matrix::CreateFromYawPitchRoll(
		       DirectX::XMConvertToRadians(m_DegAng.y),
		       DirectX::XMConvertToRadians(m_DegAng.x),
		       DirectX::XMConvertToRadians(m_DegAng.z));
	}

	const Math::Matrix GetRotationYMatrix() const
	{
		return Math::Matrix::CreateRotationY(
			   DirectX::XMConvertToRadians(m_DegAng.y));
	}

	void RegistHitObject(const std::shared_ptr<Entity>& object)
	{
		m_wpHitObjectList.push_back(object);
	}

protected:
	// カメラ回転用角度
	Math::Vector3								m_DegAng		= Math::Vector3::Zero;

	void UpdateRotateByMouse();

	std::shared_ptr<KdCamera>					m_spCamera		= nullptr;
	std::weak_ptr<Entity>						m_wpTarget;
	std::vector<std::weak_ptr<Entity>>			m_wpHitObjectList{};

    // Transform
	Math::Matrix								m_mWorld		= Math::Matrix::Identity;
	Math::Matrix								m_mLocalPos		= Math::Matrix::Identity;
	Math::Matrix								m_mRotation		= Math::Matrix::Identity;

	// カメラ回転用マウス座標の差分
	POINT										m_FixMousePos{};
    
    bool                                        m_active        = true;
};