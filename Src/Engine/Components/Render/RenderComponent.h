#pragma once

class RenderComponent : public Component
{
public:
	RenderComponent() {}
	~RenderComponent() override {}

	void Init() override;
	void DrawLit() override;
	void DrawUnLit() override {} // Empty for now

	void SetModel(const std::string& filePath);
	
	// Editor Support
	void SetModelWork(const std::string& filePath) 
    { 
        m_isDynamic = true; 
        SetModel(filePath); 
    }
	void SetModelData(const std::string& filePath) 
    { 
        m_isDynamic = false; 
        SetModel(filePath); 
    }
	const std::string& GetModelPath() const { return m_filePath; }
	bool IsDynamic() const { return m_isDynamic; }
    
	// Accessors
	const std::shared_ptr<KdModelWork>& GetModelWork() const { return m_modelWork; }
	const std::shared_ptr<KdModelData>& GetModelData() const { return m_modelData; }

	void Serialize(nlohmann::json& j) const override;
	void Deserialize(const nlohmann::json& j) override;

	const char* GetType() const override { return "Render"; }

private:
	std::shared_ptr<KdModelWork> m_modelWork;
	std::shared_ptr<KdModelData> m_modelData;
	std::string m_filePath;
	bool m_isDynamic = false;
};
