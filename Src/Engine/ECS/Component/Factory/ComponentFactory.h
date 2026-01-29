#pragma once

class Component;

class ComponentFactory
{
public:
	static ComponentFactory& Instance()
	{
		static ComponentFactory instance;
		return instance;
	}

	using Creator = std::function<std::shared_ptr<Component>()>;

	// Register a component type
	template <typename T>
	void Register(const std::string& typeName)
	{
		m_creators[typeName] = []() -> std::shared_ptr<Component> {
			return std::make_shared<T>();
		};
	}

	// Create a component by name
	std::shared_ptr<Component> Create(const std::string& typeName)
	{
		auto it = m_creators.find(typeName);
		if (it != m_creators.end())
		{
			return it->second();
		}
		return nullptr;
	}

private:
	std::map<std::string, Creator> m_creators;

	ComponentFactory() {}
	~ComponentFactory() {}
};

// Initializer function to register default components
void InitComponentFactory();
