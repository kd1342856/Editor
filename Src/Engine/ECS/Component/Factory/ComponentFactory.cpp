#include "ComponentFactory.h"

void InitComponentFactory()
{
	auto& factory = ComponentFactory::Instance();
	factory.Register<TransformComponent>("Transform");
	factory.Register<RenderComponent>("Render");
	factory.Register<ColliderComponent>("Collider");
	factory.Register<ActionPlayerComponent>("ActionPlayer");
}
