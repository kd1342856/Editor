#include "ActionPlayerComponent.h"
#include "../../../../Application/GameObject/Camera/FPSCamera/FPSCamera.h"
#include "../../../../Application/GameObject/Camera/TPSCamera/TPSCamera.h"

void ActionPlayerComponent::Init()
{
	if (!m_fpsCamera)
	{
		m_fpsCamera = std::make_shared<FPSCamera>();
		m_fpsCamera->Init();
		m_fpsCamera->SetTarget(GetOwner());
	}
	if (!m_tpsCamera)
	{
		m_tpsCamera = std::make_shared<TPSCamera>();
		m_tpsCamera->Init();
		m_tpsCamera->SetTarget(GetOwner());
	}
}

void ActionPlayerComponent::Update()
{
	// Lazy Init: Ensure cameras exist even if Init() wasn't called by system
	if (!m_fpsCamera || !m_tpsCamera)
	{
		Init();
	}

    // 所有者の取得
    auto owner = GetOwner();
    if (!owner) return;

    // TransformComponentの取得
    auto cTrans = owner->GetComponent<TransformComponent>();
    if (!cTrans) return;

	// ----- カメラ更新 & モード切替 -----
	if (m_isCameraActive)
	{
		// 切替 (Vキーかつ、Altが押されていない時のみ)
		// Alt+V は EditorManager でハンドルされるため、単発Vと区別する
		bool currV   = (GetAsyncKeyState('V')     & 0x8000) != 0;
		bool currAlt = (GetAsyncKeyState(VK_MENU) & 0x8000) != 0;

		if (currV && !m_prevV && !currAlt)
		{
			if (m_cameraMode == CameraMode::TPS)
			{
				m_cameraMode = CameraMode::FPS;
			}
			else
			{
				m_cameraMode = CameraMode::TPS;
			}
		}
		m_prevV = currV;

		// カメラ更新 (マウス制御含む)
		if (GetCamera())
		{
			GetCamera()->PostUpdate();
		}
	
	    // ----- 入力処理 (InputManager経由) -----
		// 軸入力 (WASD / 十字キー)
		Math::Vector2 inputMove = Math::Vector2::Zero;
		if (GetAsyncKeyState('W') & 0x8000) inputMove.y += 1.0f;
		if (GetAsyncKeyState('S') & 0x8000) inputMove.y -= 1.0f;
		if (GetAsyncKeyState('A') & 0x8000) inputMove.x -= 1.0f;
		if (GetAsyncKeyState('D') & 0x8000) inputMove.x += 1.0f;

	    // 正規化
	    if (inputMove.LengthSquared() > 0)
	    {
	        inputMove.Normalize();
	    }

		// カメラの向き基準で移動ベクトルを作成
		Math::Vector3 moveDir = Math::Vector3::Zero;
		if (GetCamera())
		{
			// カメラのY軸回転行列を取得
			Math::Matrix funcRotY = GetCamera()->GetRotationYMatrix();
			Math::Vector3 vX = funcRotY.Right();
			Math::Vector3 vZ = funcRotY.Backward(); 
			
			Math::Vector3 input3D = { inputMove.x, 0, inputMove.y };
			moveDir = Math::Vector3::TransformNormal(input3D, funcRotY);
		}
		else
		{
			moveDir = Math::Vector3(inputMove.x, 0, inputMove.y);
		}

	    // ----- 重力処理 -----
	    m_uGravity.y -= m_gravity;

	    // ----- ジャンプ処理 -----
	    if (GetAsyncKeyState(VK_SPACE) & 0x8000)
	    {
	        if (m_isGround)
	        {
	            m_uGravity.y = m_jumpPower;
	            m_isGround = false;
	        }
	    }

	    // ----- 移動適用 -----
	    Math::Vector3 pos = cTrans->GetPosition();
	    pos += moveDir * m_speed * 0.016f; // 簡易DeltaTime
	    pos += m_uGravity * 0.016f;

	    // ----- 簡易接地判定 (y <= 0) -----
	    if (pos.y <= 0.0f)
	    {
	        pos.y = 0.0f;
	        m_uGravity.y = 0.0f;
	        m_isGround = true;
	    }
	    else
	    {
	        m_isGround = false;
	    }

	    cTrans->SetPosition(pos);
		
		// プレイヤー回転
		if (inputMove.LengthSquared() > 0)
		{
			// 移動方向を向く
			Math::Vector3 dir = moveDir;
			dir.y = 0;
			if (dir.LengthSquared() > 0)
			{
				// 目標角度
                float targetAngle = atan2(dir.x, dir.z); // Z+が前方
				targetAngle = DirectX::XMConvertToDegrees(targetAngle);
                targetAngle -= 90.0f; // モデルの向き補正

                // 現在の角度
                float currentAngle = cTrans->GetRotation().y;

                // 角度差を計算 (最短回転方向へ)
                float diff = targetAngle - currentAngle;
                
                // -180～180の範囲に収める (360度ループ対策)
                while (diff > 180.0f) diff -= 360.0f;
                while (diff < -180.0f) diff += 360.0f;

                // 補間処理 (ゆっくり回転)
                // 0.1f 程度でスムーズに。小さいほど遅くなる
                float lerpFactor = 0.1f; 
                currentAngle += diff * lerpFactor;

				cTrans->SetRotation({ 0, currentAngle, 0 });
			}
		}
	}
}

void ActionPlayerComponent::DrawInspector()
{
    if (ImGui::CollapsingHeader("Action Player", ImGuiTreeNodeFlags_DefaultOpen))
    {
        ImGui::DragFloat("Speed", &m_speed, 0.1f);
        ImGui::DragFloat("Jump Power", &m_jumpPower, 0.1f);
        ImGui::DragFloat("Gravity", &m_gravity, 0.01f);
        
        ImGui::Text("Is Ground: %s", m_isGround ? "True" : "False");
    }
}

void ActionPlayerComponent::Serialize(nlohmann::json& outJson) const
{
	outJson = nlohmann::json
	{
		{ "Speed",     m_speed },
		{ "JumpPower", m_jumpPower },
		{ "Gravity",   m_gravity }
	};
}

void ActionPlayerComponent::Deserialize(const nlohmann::json& inJson)
{
	m_speed     = inJson.value("Speed", m_speed);
	m_jumpPower = inJson.value("JumpPower", m_jumpPower);
	m_gravity   = inJson.value("Gravity", m_gravity);
}

std::shared_ptr<CameraBase> ActionPlayerComponent::GetCamera() const
{
	if (m_cameraMode == CameraMode::FPS) return m_fpsCamera;
	return m_tpsCamera;
}
