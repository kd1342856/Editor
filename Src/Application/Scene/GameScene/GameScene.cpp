#include "GameScene.h"
#include "../../../Engine/Scene/Scene.h"
#include "../../../Engine/Core/Engine.h"
#include "../../../Engine/ECS/Entity.h"
#include "../../../Engine/Components/TransformComponent.h"
#include "../../../Engine/Scene/SceneManager.h"

void GameScene::Init()
{
	// 基底クラスの初期化
	BaseScene::Init();

	// ゲームシーン特有の初期化
	OutputDebugStringA("Game Scene Initialized.\n");

	// Visual Marker
	// auto e = std::make_shared<Entity>();
	// e->SetName("GameScene");
	// e->AddComponent(std::make_shared<TransformComponent>());
	// SceneManager::Instance().AddEntity(e);
}
