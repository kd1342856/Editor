#pragma once

#include "../Engine/Scene/SceneManager.h"
#include "Scene/TitleScene/TitleScene.h"
#include "Scene/GameScene/GameScene.h"
#include "Scene/ResultScene/ResultScene.h"

//============================================================
// アプリケーションクラス
//	APP.～ でどこからでもアクセス可能
//============================================================
class Application
{
// メンバ
public:

	void Init()
	{
		// シーンの登録
		SceneManager::Instance().RegisterScene("Title", []() { 
			auto s = std::make_shared<TitleScene>(); 
			s->SetName("Title"); 
			return s; 
		});
		SceneManager::Instance().RegisterScene("Game", []() { 
			auto s = std::make_shared<GameScene>(); 
			s->SetName("Game"); 
			return s; 
		});
		SceneManager::Instance().RegisterScene("Result", []() { 
			auto s = std::make_shared<ResultScene>(); 
			s->SetName("Result"); 
			return s; 
		});

		// 最初のシーンへ遷移
		SceneManager::Instance().ChangeScene("Title");
	}

	void Release() {}
	void Execute() {}

private:

//=====================================================
// シングルトンパターン
//=====================================================
private:
	// 
	Application() {}

public:
	static Application &Instance(){
		static Application Instance;
		return Instance;
	}
};
