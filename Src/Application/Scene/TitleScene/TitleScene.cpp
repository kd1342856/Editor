#include "TitleScene.h"
#include "../../../Engine/Scene/Scene.h"
#include "../../../Engine/ECS/Entity.h"
#include "../../../Engine/Components/TransformComponent.h"
#include "../../../Engine/Scene/SceneManager.h"

void TitleScene::Init()
{
	// 基底クラスの初期化
	BaseScene::Init();

	// タイトルシーン特有の初期化（ロゴとか）
	// ここではテスト用にログを出す
	OutputDebugStringA("Title Scene Initialized.\n");
	
	// Visual Marker
	// auto e = std::make_shared<Entity>();
	// e->SetName("TitleScene");
	// e->AddComponent(std::make_shared<TransformComponent>());
	// // e->AddComponent(std::make_shared<RenderComponent>()); // Add if default model available, or just empty
	// SceneManager::Instance().AddEntity(e);
}
