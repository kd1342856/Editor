#pragma once
#include "../../../Engine/Scene/Scene.h"

class BaseScene : public Scene
{
public:
	BaseScene() {}
	virtual ~BaseScene() override {}

	virtual void Init() override;
	virtual void Release() override;

protected:
	std::string m_name;

public:
	void SetName(const std::string& name) { m_name = name; }
	const std::string& GetName() const { return m_name; }
};
