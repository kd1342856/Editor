#include "main.h"
#include "../Engine/Scene/SceneManager.h"
#include "Scene/TitleScene/TitleScene.h"
#include "Scene/GameScene/GameScene.h"
#include "Scene/ResultScene/ResultScene.h"

int WINAPI WinMain(_In_ HINSTANCE, _In_opt_  HINSTANCE, _In_ LPSTR, _In_ int)
{
	// メモリリークを知らせる
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	// COM初期化
	if (FAILED(CoInitializeEx(nullptr, COINIT_MULTITHREADED)))
	{
		CoUninitialize();

		return 0;
	}

	// mbstowcs_s関数で日本語対応にするために呼ぶ
	// ImGuiとファイルシステムの整合性を保つためUTF-8に変更
	setlocale(LC_ALL, ".UTF8");

	//===================================================================
	// 実行
	//===================================================================
	// Engine初期化 & 実行

	Engine::Instance().Execute();

	// COM解放
	CoUninitialize();

	return 0;
}