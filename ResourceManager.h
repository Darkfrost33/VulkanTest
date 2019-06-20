#pragma once
#include "Singleton.h"

struct MeshData;

class ResourceManager:public Singleton<ResourceManager>
{
public:
	~ResourceManager();

	MeshData* LoadModel(std::string name, std::string path);
	//LoadImage()
	std::unordered_map<std::string, MeshData*> mMeshes;
private:
	friend class Singleton<ResourceManager>;
	ResourceManager();
	MeshData* BoxMesh();
	MeshData* SphereMesh();
	//std::unordered_map<std::string, Image*> mImages;
};