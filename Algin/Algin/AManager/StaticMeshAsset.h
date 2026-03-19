#pragma once
#include "pch.h"
#include "../Graphics/Renderer.h"
#include "../Graphics/MaterialAsset.h"

struct SubMesh
{
	GLuint VAO = 0, VBO = 0, IBO = 0;

	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;
	std::string name;
	AABB minmax;
	void Setup();


	//GLuint materialUBO;
	//matAsset mat;
	//int albedoIdx = 0;
	//int normalMapIdx = 0;
	//int currMatIdx = 0;
};

struct StaticMeshAsset : public IAsset
{
	std::vector<SubMesh> submeshes;
	std::string original_name;
	StaticMeshAsset() {}
	~StaticMeshAsset();

	void Load(std::filesystem::path file) override;
	void Unload() override;
	void WriteToBinary(std::string new_file) override;
	void LoadFromBinary(std::filesystem::path path) override;

	ASSET_TYPE GetType() const override { return ASSET_TYPE::MODEL; }
	void GetMetaData(std::string new_file, int& vert, int& subm, std::vector<std::string>& mats);
	void SetUpMinMax();

	void ApplyScaling(float x, float y, float z);

};