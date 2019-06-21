#pragma once
#include "Singleton.h"

struct MeshData;

class ResourceManager:public Singleton<ResourceManager>
{
public:
	~ResourceManager();

	void Initialize_Models();
	void Initialize_Textures();

	MeshData* LoadModel(std::string name, std::string path);
	uint32_t LoadTexture(std::string path);
	//LoadImage()
	std::unordered_map<std::string, MeshData*> mMeshes;
	std::unordered_map<std::string, uint32_t> mTextures;

private:
	friend class Singleton<ResourceManager>;
	ResourceManager();
	MeshData* BoxMesh();
	MeshData* SphereMesh();
	//std::unordered_map<std::string, Image*> mImages;
};