#include "pch.h"
#include "Mesh.h"
#include "vulkanAPI.h"
#include "ResourceManager.h"
Mesh::Mesh()
{
}

Mesh::~Mesh()
{
}

void Mesh::Start()
{
}

void Mesh::Update(float deltaTime)
{
}

void Mesh::Serialize(XMLElement * _element)
{
}

void Mesh::Initialize()
{
	Component::Initialize();
}

void Mesh::BindMesh(std::string name)
{
	mMesh = ResourceManager::GetInstance()->LoadModel(name, "");
}
