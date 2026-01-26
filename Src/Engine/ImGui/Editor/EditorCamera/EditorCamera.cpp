#include "EditorCamera.h"

EditorCamera::EditorCamera()
{
	m_focusPoint = { 5.0f, 7.0f, 0.0f };	// 高さを上げる
    m_distance = 0.0f;						// 距離も少し離す
    m_DegAng = { 20.0f, -45.0f, 0.0f };		// 角度も見下ろし気味に
}

void EditorCamera::Init()
{
	CameraBase::Init();

	// Proj
	m_spCamera->SetProjectionMatrix(60.0f);
}

void EditorCamera::Update()
{
	if (m_inputEnabled)
	{
		UpdateMouseInput();
	}

	// 注視点と角度から位置を計算
	Math::Matrix rotation = Math::Matrix::CreateFromYawPitchRoll(
		DirectX::XMConvertToRadians(m_DegAng.y),
		DirectX::XMConvertToRadians(m_DegAng.x),
		0.0f);

	Math::Vector3 direction = Math::Vector3::TransformNormal(Math::Vector3::Backward, rotation);
	Math::Vector3 position = m_focusPoint + direction * m_distance;

	// ワールド行列の作成
    // 回転を含めてワールド行列を再構築
    m_mWorld = rotation * Math::Matrix::CreateTranslation(position);

	// KdCamera を更新
	if (m_spCamera)
	{
		m_spCamera->SetCameraMatrix(m_mWorld);
	}

	// 入力が無効なら何もしない
	if (!m_inputEnabled)
	{
		// 入力無効時はカーソルを表示させる
		m_isFlyMode = false;
		return; 
	}

	ImGuiIO& io = ImGui::GetIO();


    // ---- モード切替 (ホイールクリック) ----
    if (ImGui::IsMouseClicked(ImGuiMouseButton_Middle))
    {
        // モード切替時の「瞬間移動」を防ぐ処理
        Math::Matrix rotMat = Math::Matrix::CreateFromYawPitchRoll(
            DirectX::XMConvertToRadians(m_DegAng.y),
            DirectX::XMConvertToRadians(m_DegAng.x),
            0.0f);
        Math::Vector3 forward = rotMat.Backward();

        if (!m_isFlyMode) // これからFlyModeに入る場合 (Orbit -> Fly)
        {
            // Orbitモードの注視点から、カメラ位置(Flyモードの視点)へ移動
            // CameraPos = Target + Backward * Distance
            m_focusPoint += forward * m_distance;
            m_justEnteredFlyMode = true; // フラグを立てる
        }
        else // これからOrbitModeに入る場合 (Fly -> Orbit)
        {
            // Flyモードの視点(カメラ位置)から、注視点へ移動
            // Target = CameraPos - Backward * Distance
            m_focusPoint -= forward * m_distance;
        }

        m_isFlyMode = !m_isFlyMode;
    }

	if (m_isFlyMode)
	{
        // ==== フライモード (WASD + マウスルック) ====
        
        // カーソル非表示
        ImGui::SetMouseCursor(ImGuiMouseCursor_None);
        
        // --- WinAPIによるマウス位置の中央固定とデルタ計算 ---
        POINT cursorPos;
        GetCursorPos(&cursorPos); // スクリーン座標取得

        // ウィンドウの中心を計算
        ImVec2 windowPos = ImGui::GetWindowPos();
        ImVec2 windowSize = ImGui::GetWindowSize();
        int centerX = (int)(windowPos.x + windowSize.x * 0.5f);
        int centerY = (int)(windowPos.y + windowSize.y * 0.5f);

        // 切り替え直後のフレームは強制リセットのみ行う (差分計算しない)
        if (m_justEnteredFlyMode)
        {
            SetCursorPos(centerX, centerY);
            m_justEnteredFlyMode = false;
        }
        else
        {
            // 中心との差分
            float dx = (float)(cursorPos.x - centerX);
            float dy = (float)(cursorPos.y - centerY);

            // クランプ処理 (ウィンドウ切り替え直後などの飛び防止)
            if (std::abs(dx) < 500.0f && std::abs(dy) < 500.0f)
            {
                m_DegAng.y += dx * m_rotateSpeed;
                m_DegAng.x += dy * m_rotateSpeed;
                m_DegAng.x = std::clamp(m_DegAng.x, -89.0f, 89.0f);
            }

            // マウス位置を中心に戻す
            SetCursorPos(centerX, centerY);
        }
        
        // ImGui側にも伝える (念のため)
        // io.MousePos = ImVec2((float)centerX, (float)centerY);
        // io.WantSetMousePos = true;

        // --- 移動 (WinAPI GetAsyncKeyState) ---
        Math::Vector3 moveDir = Math::Vector3::Zero;

        Math::Matrix rotation = Math::Matrix::CreateFromYawPitchRoll(
            DirectX::XMConvertToRadians(m_DegAng.y),
            DirectX::XMConvertToRadians(m_DegAng.x),
            0.0f);

        Math::Vector3 forward = rotation.Backward(); 
        Math::Vector3 right   = rotation.Right();
        Math::Vector3 up      = Math::Vector3::Up;

        // WinAPI で直接キー入力判定
        if (GetAsyncKeyState('W') & 0x8000) moveDir += forward;
        if (GetAsyncKeyState('S') & 0x8000) moveDir -= forward;
        if (GetAsyncKeyState('D') & 0x8000) moveDir += right;
        if (GetAsyncKeyState('A') & 0x8000) moveDir -= right;
        if (GetAsyncKeyState(VK_SPACE) & 0x8000) moveDir += up;      // 上昇
        if (GetAsyncKeyState(VK_LSHIFT) & 0x8000) moveDir -= up;     // 下降

        if (moveDir.LengthSquared() > 0)
        {
            moveDir.Normalize();
            m_focusPoint += moveDir * m_moveSpeed * io.DeltaTime;
        }
	}
	else
	{
        // ==== 通常 (オービット) モード ====
        
        // 回転 (右ドラッグ)
        if (ImGui::IsMouseDown(ImGuiMouseButton_Right))
        {
            float dx = io.MouseDelta.x;
            float dy = io.MouseDelta.y;

            m_DegAng.y += dx * m_rotateSpeed;
            m_DegAng.x += dy * m_rotateSpeed;
            m_DegAng.x = std::clamp(m_DegAng.x, -89.0f, 89.0f);
        }

        // ズーム (ホイール)
        // if (io.MouseWheel != 0.0f)
        // {
        //     m_distance -= io.MouseWheel * m_zoomSpeed; 
        //     // if (m_distance < 0.1f) m_distance = 0.1f;
        // }
        
        // パン (中ドラッグ)
        if (ImGui::IsMouseDragging(ImGuiMouseButton_Middle))
        {
            float dx = io.MouseDelta.x * m_panSpeed * (m_distance * 0.1f);
            float dy = io.MouseDelta.y * m_panSpeed * (m_distance * 0.1f);

            Math::Matrix rotation = Math::Matrix::CreateFromYawPitchRoll(
                DirectX::XMConvertToRadians(m_DegAng.y),
                DirectX::XMConvertToRadians(m_DegAng.x),
                0.0f);
            
            Math::Vector3 right = rotation.Right();
            Math::Vector3 up = rotation.Up();
            
            m_focusPoint -= right * dx;
            m_focusPoint += up * dy;
        }
	}

    // 行列の更新
    Math::Matrix rotMat = Math::Matrix::CreateFromYawPitchRoll(
        DirectX::XMConvertToRadians(m_DegAng.y),
        DirectX::XMConvertToRadians(m_DegAng.x),
        0.0f);
    
    Math::Matrix transMat;

    if (m_isFlyMode)
    {
        transMat = Math::Matrix::CreateTranslation(m_focusPoint);
    }
    else
    {
        Math::Vector3 camPos = m_focusPoint + rotMat.Backward() * m_distance;
        transMat = Math::Matrix::CreateTranslation(camPos);
    }

    m_mWorld = rotMat * transMat;
}

void EditorCamera::UpdateMouseInput()
{
}
