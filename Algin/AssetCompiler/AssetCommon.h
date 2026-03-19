#pragma once
#include <iostream>
#include <istream>
#include <fstream>
#include <vector>
#include <string>
#include <filesystem>
#include "GLFW/glfw3.h"
#include "glm/glm.hpp"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <zlib.h>

bool CompressBuffer(const std::vector<unsigned char>& input, std::vector<unsigned char>& output);

bool DecompressBuffer(const std::vector<unsigned char>& input, std::vector<unsigned char>& output, size_t originalSize);


enum ASSET_TYPE
{
	UNKNOWN,
	TEXTURE,
	MODEL,
	AUDIO,
	SCRIPT,
	AGCUBEMAP,
	FONT,
	DDS,
	AGSKINNEDMESH,
	AGTEXTURE,
	AGSTATICMESH,
	AGDYNAMICMESH,
	AGAUDIO,
	AGFONT,
};

struct Vertex
{
	float position[3];
	float normal[3];
	float color[4];
	float tangent[3];
	float bitangent[3];
	float texcoord[2];
	float texid;

	/* for animation */
	int   boneIDs[4];
	float weights[4];

	Vertex() = default;
	Vertex(float x, float y, float z, float s, float t) :
		position{ x, y, z },
		normal{ 0.0f, 0.0f, 0.0f },
		color{ 1.0f, 1.0f, 1.0f, 1.0f },
		texcoord{ s, t },
		texid(0.0f),
		tangent{ 0.f,0.f,0.f },
		bitangent{ 0.f,0.f,0.f } {
	}

	inline void AddBoneData(int boneID, float weight)
	{
		for (int i = 0; i < 4; ++i)
		{
			if (weights[i] == 0.0f)
			{
				boneIDs[i] = boneID;
				weights[i] = weight;
				return;
			}
		}

		// all 4 full → replace smallest weight if this one is stronger
		int smallest = 0;
		for (int i = 1; i < 4; ++i)
			if (weights[i] < weights[smallest])
				smallest = i;

		if (weight > weights[smallest])
		{
			boneIDs[smallest] = boneID;
			weights[smallest] = weight;
		}
	}

	// normalize weights to sum to 1.0 (optional but good for GPU precision)
	inline void NormalizeWeights()
	{
		float sum = weights[0] + weights[1] + weights[2] + weights[3];
		if (sum > 0.0f)
		{
			for (int i = 0; i < 4; ++i)
				weights[i] /= sum;
		}
	}
};


struct AABB
{
	glm::vec3 min;
	glm::vec3 max;

	AABB() : min(glm::vec3(-0.5f)), max(glm::vec3(0.5f)) {}
	AABB(const glm::vec3& min_, const glm::vec3& max_) : min(min_), max(max_) {}
	AABB(float x, float y, float z) : min(glm::vec3(-x * 0.5f, -y * 0.5f, -z * 0.5f)), max(glm::vec3(x * 0.5f, y * 0.5f, z * 0.5f)) {}
	AABB(const glm::vec3 size) : min(-size * 0.5f), max(size * 0.5f) {}
	bool contains(glm::vec3 point);
	AABB transform(glm::mat4 m2w);
};

struct SubMesh
{
	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;
	std::string name;
	AABB minmax;
};

struct StaticMeshAsset 
{
	std::vector<SubMesh> submeshes;
	std::string original_name;
	StaticMeshAsset() {}
	~StaticMeshAsset() = default;

	void Load(std::filesystem::path file);
	void WriteToBinary(std::string new_file);
};