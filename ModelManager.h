#pragma once
class ModelManager
{
public:
	static ModelManager* GetInstance();
	void Release();

	~ModelManager() = default;
	ModelManager(const ModelManager& other) = delete;
	ModelManager(ModelManager&& other) noexcept = delete;
	ModelManager& operator=(const ModelManager& other) = delete;
	ModelManager& operator=(ModelManager&& other) noexcept = delete;

	void Init();

	std::vector<Vertex> GetVerts() const { return m_Verts; };
	std::vector<uint32_t> GetIndices() const { return m_Indices; };

private:
	ModelManager();
	static ModelManager* m_Instance;

	std::vector<Vertex> m_Verts{};
	std::vector<uint32_t> m_Indices{};
};

