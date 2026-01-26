#pragma once
#include <typeindex>

class Entity : public std::enable_shared_from_this<Entity>
{
public:
	enum class VisibilityFlags : uint8_t
	{
		None   = 0,
		Lit    = 1 << 0,
		UnLit  = 1 << 1,
		Bright = 1 << 2,
		Shadow = 1 << 3,
	};

	Entity() {}
	virtual ~Entity() {}

	virtual void Init();
	virtual void Update();
	virtual void PostUpdate();
	virtual void PreDraw();
	virtual void DrawLit();
	virtual void DrawUnLit();
	virtual void DrawBright();
	virtual void GenerateDepthMapFromLight();
	virtual void DrawSprite();
	virtual void DrawInspector();
	virtual void DrawDebug();

	void SetName(const std::string& name) { m_name = name; }
	const std::string& GetName() const { return m_name; }

	void SetVisible(bool visible) { m_visible = visible; }
	bool IsVisible() const { return m_visible; }

	void SetVisibility(VisibilityFlags flag, bool enabled);
	bool IsVisible(VisibilityFlags flag) const;

	template <typename T>
	void AddComponent(const std::shared_ptr<T>& component);

	template <typename T>
	std::shared_ptr<T> GetComponent() const;

	template <typename T>
	bool HasComponent() const;

	Math::Matrix GetMatrix() const;

private:
	std::string m_name = "Entity";
	bool m_visible = true;
	uint8_t m_visibilityFlags = static_cast<uint8_t>(VisibilityFlags::Lit) | static_cast<uint8_t>(VisibilityFlags::UnLit) | static_cast<uint8_t>(VisibilityFlags::Shadow);

	std::unordered_map<std::type_index, std::shared_ptr<Component>> m_components;
};

template <typename T>
void Entity::AddComponent(const std::shared_ptr<T>& component)
{
	static_assert(std::is_base_of<Component, T>::value, "T must derive from Component");
	if (!component) return;

	component->SetOwner(shared_from_this());
    component->Init();
	m_components[std::type_index(typeid(T))] = component;
}

template <typename T>
std::shared_ptr<T> Entity::GetComponent() const
{
	auto it = m_components.find(std::type_index(typeid(T)));
	if (it != m_components.end())
	{
		return std::dynamic_pointer_cast<T>(it->second);
	}
	return nullptr;
}

template <typename T>
bool Entity::HasComponent() const
{
	return m_components.find(std::type_index(typeid(T))) != m_components.end();
}
