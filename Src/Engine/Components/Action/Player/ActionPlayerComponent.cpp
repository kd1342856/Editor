#include "ActionPlayerComponent.h"

void ActionPlayerComponent::Init()
{
}

void ActionPlayerComponent::Update()
{
    // 所有者の取得
    auto owner = GetOwner();
    if (!owner) return;

    // TransformComponentの取得
    auto cTrans = owner->GetComponent<TransformComponent>();
    if (!cTrans) return;

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

    // カメラの向き基準で移動ベクトルを作成 (とりあえず簡易的にカメラ無視でワールド基準で動かす)
    // ※後でTPSカメラ対応する
    Math::Vector3 moveDir = Math::Vector3(inputMove.x, 0, inputMove.y);

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
