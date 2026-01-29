#include "Engine.h"
#include "../Scene/SceneManager.h"
#include "../ImGui/ImGuiManager.h"
#include "Thread/ThreadManager.h"
#include "Thread/Asset/AsyncAssetLoader.h"
#include "Thread/Profiler/Profiler.h"
#include "../../Application/main.h"
#include "../ECS/Entity/EntityManager.h"
#include "../Render/Renderer.h"
#include "../ECS/Component/Factory/ComponentFactory.h"

bool Engine::Init(int width, int height)
{
	//===================================================================
	// ウィンドウ作成
	//===================================================================
	if (m_window.Create(width, height, "3D GameProgramming", "Window") == false) {
		MessageBoxA(nullptr, "ウィンドウ作成に失敗", "エラー", MB_OK);
		return false;
	}

	//===================================================================
	// フルスクリーン確認
	//===================================================================
	bool bFullScreen = false;
//	if (MessageBoxA(m_window.GetWndHandle(), "フルスクリーンにしますか？", "確認", MB_YESNO | MB_ICONQUESTION | MB_DEFBUTTON2) == IDYES) {
//		bFullScreen = true;
//	}

	// デバイスのデバッグモードを有効にする
	bool deviceDebugMode = false;
#ifdef _DEBUG
	deviceDebugMode = true;
#endif

	// Direct3D初期化
	std::string errorMsg;
	if (KdDirect3D::Instance().Init(m_window.GetWndHandle(), width, height, deviceDebugMode, errorMsg) == false)
	{
		MessageBoxA(m_window.GetWndHandle(), errorMsg.c_str(), "Direct3D初期化失敗", MB_OK | MB_ICONSTOP);
		return false;
	}

	// フルスクリーン設定
	if (bFullScreen) 
	{
		HRESULT hr;

		hr = KdDirect3D::Instance().SetFullscreenState(TRUE, 0);
		if (FAILED(hr))
		{
			MessageBoxA(m_window.GetWndHandle(), "フルスクリーン設定失敗", "Direct3D初期化失敗", MB_OK | MB_ICONSTOP);
			return false;
		}
	}

	// imgui初期化
	ImGuiManager::Instance().GuiInit();

	// シェーダー初期化
	KdShaderManager::Instance().Init();

	// オーディオ初期化
	KdAudioManager::Instance().Init();

	// スレッドプール初期化
	ThreadManager::Instance().Init();
	
	// 非同期ローダー初期化
	AsyncAssetLoader::Instance().Init();

	// コンポーネントファクトリ初期化 (これがないとロード時にコンポーネントが生成されない)
	InitComponentFactory();

	// 非同期テクスチャロードをKdAssetsに登録
	KdAssets::Instance().m_textures.SetCustomLoader([](const std::string& filename)
	{
		return AsyncAssetLoader::Instance().LoadTextureAsync(filename);
	});

	// 非同期モデルロードをKdAssetsに登録
	KdAssets::Instance().m_modeldatas.SetCustomLoader([](const std::string& filename)
	{
		return AsyncAssetLoader::Instance().LoadModelAsync(filename);
	});

	return true;
}

void Engine::SetMouseGrabbed(bool enable)
{
	ShowCursor(!enable);
	
	if (enable)
	{
		RECT rc;
		GetClientRect(m_window.GetWndHandle(), &rc);
		ClientToScreen(m_window.GetWndHandle(), (POINT*)&rc.left);
		ClientToScreen(m_window.GetWndHandle(), (POINT*)&rc.right);
		ClipCursor(&rc);
	}
	else
	{
		ClipCursor(nullptr);
	}
}

void Engine::Execute()
{
	KdCSVData windowData("Asset/Data/WindowSettings.csv");
	const std::vector<std::string>& sizeData = windowData.GetLine(0);

	//===================================================================
	// 初期設定(ウィンドウ作成、Direct3D初期化など)
	//===================================================================
	if (Engine::Instance().Init(atoi(sizeData[0].c_str()), atoi(sizeData[1].c_str())) == false) 
	{
		return;
	}

	// アプリケーション初期化 (シーン登録など)
	Application::Instance().Init();

	KdFPSController fpsCtrl;
	fpsCtrl.Init();

	while (true)
	{
		Profiler::Instance().ResetFrame();
		fpsCtrl.UpdateStartTime();

		if (m_endFlag) break;

		// Window Message Processing
		m_window.ProcessMessage();
		if (!m_window.IsCreated()) break;

		if (GetAsyncKeyState(VK_ESCAPE)&0x0001)
		{
			if (MessageBoxA(m_window.GetWndHandle(), "Quit?", "Confirm", MB_YESNO) == IDYES)
			{
				break;
			}
		}

		
		// アプリケーション更新処理
		

		KdBeginUpdate();
		{
			PreUpdate();

			Update();

			PostUpdate();
		}
		KdPostUpdate();

		
		// アプリケーション描画処理
		

		KdBeginDraw();
		{
			PreDraw();

			Draw();

			PostDraw();

			DrawSprite();
		}
		KdPostDraw();

		fpsCtrl.Update();
	}
	Release();
}

void Engine::Release()
{
	if (m_isReleased) return;

	AsyncAssetLoader::Instance().Release();
	ThreadManager::Instance().Release();
	SceneManager::Instance().Release();
	ImGuiManager::Instance().GuiRelease();
	KdShaderManager::Instance().Release();
	KdAudioManager::Instance().Release();
	KdDirect3D::Instance().Release();
	m_window.Release();

	m_isReleased = true;
}

void Engine::KdBeginUpdate()
{
	// 入力状況の更新
	KdInputManager::Instance().Update();

	// 非同期ローダー更新
	AsyncAssetLoader::Instance().Update();

	// 空間環境の更新
	KdShaderManager::Instance().WorkAmbientController().Update();
}

void Engine::KdPostUpdate()
{
	// 3DSoundListnerの行列を更新
	KdAudioManager::Instance().SetListnerMatrix(KdShaderManager::Instance().GetCameraCB().mView.Invert());
	EntityManager::Instance().PostUpdate();
}

void Engine::KdBeginDraw(bool usePostProcess)
{
	KdDirect3D::Instance().ClearBackBuffer();

	KdShaderManager::Instance().WorkAmbientController().Draw();

	if (!usePostProcess) return;
	KdShaderManager::Instance().m_postProcessShader.Draw();
}

void Engine::KdPostDraw()
{
	// BackBuffer -> 画面表示
	KdDirect3D::Instance().WorkSwapChain()->Present(0, 0);
}

void Engine::PreUpdate()
{
}

void Engine::Update()
{
	PROFILE_FUNCTION();
	SceneManager::Instance().Update();
	EntityManager::Instance().Update();
}

void Engine::PostUpdate()
{
	EntityManager::Instance().PostUpdate();
}

void Engine::PreDraw()
{
	EntityManager::Instance().PreDraw();
}

void Engine::Draw()
{
	PROFILE_FUNCTION();
	SceneManager::Instance().Draw();
	Renderer::Instance().Draw();
}

void Engine::DrawSprite()
{
	Renderer::Instance().DrawSprite();
}

void Engine::PostDraw()
{	
	PROFILE_FUNCTION();

	// 画面のぼかしや被写界深度処理の実施
	KdShaderManager::Instance().m_postProcessShader.PostEffectProcess();

	// ImGui Process (Draw UI on top of everything)
	ImGuiManager::Instance().GuiProcess();
}