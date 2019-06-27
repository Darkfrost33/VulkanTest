#include "pch.h"
#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>
#include "ResourceManager.h"
#include "vulkanAPI.h"

namespace std {
	template<> struct hash<VertexData> {
		size_t operator()(VertexData const& vertex) const {
			return ((hash<glm::vec3>()(vertex.pos) ^
				(hash<glm::vec3>()(vertex.normal) << 1)) >> 1) ^
				(hash<glm::vec2>()(vertex.texCoord) << 1);
		}
	};
}

void MathPrint(glm::vec3 v)
{
	std::cout << v[0] << ", " << v[1] << ", " << v[2] << std::endl;
}

void MathPrint(glm::mat4 m)
{
	for (int i = 0; i < 4; ++i)
	{
		std::cout << m[0][i] << ", " << m[1][i] << ", " << m[2][i] << ", " << m[3][i] << std::endl;
	}
}

ResourceManager::ResourceManager()
{
}
ResourceManager::~ResourceManager()
{
	for (auto &mesh : mMeshes)
	{
		delete mesh.second;
	}
	mMeshes.clear();
	mTextures.clear();
}

void ResourceManager::Initialize_Models()
{
	mMeshes["box"] = BoxMesh();
	mMeshes["sphere"] = SphereMesh();
	//mMeshes["house"] = LoadModel("house", "Models/chalet.obj");
}

void ResourceManager::Initialize_Textures()
{
	LoadTexture("Textures/texture.jpg");
	LoadTexture("Textures/2k_earth.jpg");
	LoadTexture("Textures/chalet.jpg");
}

MeshData * ResourceManager::LoadModel(std::string name, std::string path)
{
	if (mMeshes.find(name) != mMeshes.end())
	{
		return mMeshes[name];
	}
	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	std::string warn, err;
	MeshData* meshdata = new MeshData();
	meshdata->name = name;
	if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, path.c_str())) {
		throw std::runtime_error(warn + err);
	}

	std::unordered_map<VertexData, uint32_t> uniqueVertices = {};

	for (auto shape : shapes) {
		for (auto& index : shape.mesh.indices) {
			VertexData vertex = {};

			vertex.pos = {
				attrib.vertices[3 * index.vertex_index + 0],
				attrib.vertices[3 * index.vertex_index + 1],
				attrib.vertices[3 * index.vertex_index + 2]
			};

			//MathPrint(vertex.pos);

			if (attrib.normals.size() > 0) {
				vertex.normal = {
					attrib.normals[3 * index.normal_index + 0],
					attrib.normals[3 * index.normal_index + 1],
					attrib.normals[3 * index.normal_index + 2]
				};
			}

			vertex.texCoord = {
				attrib.texcoords[2 * index.texcoord_index + 0],
				1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
			};

			if (uniqueVertices.count(vertex) == 0) {
				uniqueVertices[vertex] = static_cast<uint32_t>(meshdata->vertices.size());
				meshdata->vertices.push_back(vertex);
			}
			meshdata->indices.push_back(uniqueVertices[vertex]);
		}
	}
	std::cout << "vertices count: " << meshdata->vertices.size() << std::endl;
	std::cout << "face count: " << meshdata->indices.size()/3 << std::endl;
	return meshdata;
}

uint32_t ResourceManager::LoadTexture(std::string path)
{
	if (mTextures.find(path) != mTextures.end())
	{
		return mTextures[path];
	}

	mTextures[path] = VulkanAPI::GetInstance()->UploadTexture(path);

	return mTextures[path];
}

MeshData* ResourceManager::BoxMesh()
{
	if (mMeshes.find("box") != mMeshes.end())
	{
		return mMeshes["box"];
	}
	glm::mat4 face[6] = {
		glm::mat4(1.0f),
		glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
		glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
		glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
		glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f)),
		glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f)) };

	glm::mat4 half = glm::scale(glm::mat4(1.0f), glm::vec3(0.5f, 0.5f, 0.5f));
	MeshData* meshdata = new MeshData();
	meshdata->name = "box";
	for (unsigned int f = 0; f < 6; f++) {
		glm::mat4 m4 = face[f]*half;
		for (unsigned int i = 0; i < 2; i++) {
			for (unsigned int j = 0; j < 2; j++) {
				glm::vec4 p = m4 * glm::vec4 (float(2 * i) - 1.0f, float(2 * j) - 1.0f, 1.0f, 1.0f);
				glm::vec4 tnrm = m4 * glm::vec4(0.0f, 0.0f, 1.0f, 0.0f);
				meshdata->vertices.push_back(VertexData(glm::vec3(p[0], p[1], p[2]),
					glm::vec3(glm::normalize(tnrm)),
					glm::vec2(float(i), float(j))));
			}
		}
		meshdata->indices.push_back(4 * f + 0);
		meshdata->indices.push_back(4 * f + 3);
		meshdata->indices.push_back(4 * f + 1);

		meshdata->indices.push_back(4 * f + 0);
		meshdata->indices.push_back(4 * f + 2);
		meshdata->indices.push_back(4 * f + 3);
	}
	return meshdata;
}

MeshData * ResourceManager::SphereMesh()
{
	if (mMeshes.find("sphere") != mMeshes.end())
	{
		return mMeshes["sphere"];
	}

	MeshData* meshdata = new MeshData();
	meshdata->name = "sphere";
	unsigned int n = 20;
	float PI = glm::pi<float>();
	for (unsigned int i = 0; i <= n * 2; i++) {
		float s = i * 2.0f*PI / float(n * 2);
		for (unsigned int j = 0; j <= n; j++) {
			float t = j * PI / float(n);
			float x = 0.5f*cosf(s)*sinf(t);
			float y = 0.5f*sinf(s)*sinf(t);
			float z = 0.5f*cosf(t);
			meshdata->vertices.push_back(VertexData(glm::vec3 (x, y, z),
				glm::vec3(x, y, z),
				glm::vec2(s / (2 * PI), t / PI)));
			if (i > 0 && j > 0) {
				meshdata->indices.push_back((i - 1)*(n + 1) + (j - 1));
				meshdata->indices.push_back((i - 1)*(n + 1) + (j));
				meshdata->indices.push_back((i)*(n + 1) + (j));

				meshdata->indices.push_back((i - 1)*(n + 1) + (j - 1));
				meshdata->indices.push_back((i)*(n + 1) + (j));
				meshdata->indices.push_back((i)*(n + 1) + (j - 1));
			}
		}
	}
	return meshdata;
}
