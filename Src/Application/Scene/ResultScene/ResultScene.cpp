#include "ResultScene.h"
#include "../../../Engine/Core/Engine.h"
#include "../../../Engine/ECS/Entity.h"
#include "../../../Engine/Components/TransformComponent.h"
#include "../../../Engine/Scene/SceneManager.h"

void ResultScene::Init()
{
	// 基底クラスの初期化
	BaseScene::Init();

	// リザルトシーン特有の初期化
	OutputDebugStringA("Result Scene Initialized.\n");

	// Visual Marker
	// auto e = std::make_shared<Entity>();
	// e->SetName("ResultScene");
	// e->AddComponent(std::make_shared<TransformComponent>());
	// SceneManager::Instance().AddEntity(e);
}
