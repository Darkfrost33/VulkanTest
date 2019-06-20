#pragma once
#include "component.h"

struct MeshData;

class Mesh :public Component
{
public:
	Mesh();
	~Mesh();
	void Start() override;
	void Update(float deltaTime) override;
	void Serialize(XMLElement* _element) override;
	void Initialize() override;

	void BindMesh(std::string path);

	MeshData* mMesh;
};