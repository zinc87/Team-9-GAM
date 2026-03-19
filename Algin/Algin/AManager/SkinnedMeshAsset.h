#pragma once
#include "pch.h"

struct Bone
{
	std::string name;
	int parentIndex = -1;              // -1 if root bone

	glm::mat4 bindPose = glm::mat4(1.0f);         // Bone's local transform in bind pose
	glm::mat4 inverseBindPose = glm::mat4(1.0f);  // Converts from model space → bone space
	glm::mat4 localTransform = glm::mat4(1.0f);   // Current local transform (can change during animation)
};

struct AnimationKey
{
	float time;
	glm::vec3 translation;
	glm::quat rotation;
	glm::vec3 scale;
};

struct BoneAnimation
{
	std::string boneName;
	std::vector<AnimationKey> keyframes;
};

struct AnimationClip
{
	std::string name;
	float duration = 0.f;
	float ticksPerSecond = 25.f;
	std::vector<BoneAnimation> boneAnimations;
};


struct SubMeshSkinned
{
	GLuint VAO = 0, VBO = 0, IBO = 0;

	//scanning
	bool scanAffected;

	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;
	std::string name;
	AABB minmax;
	void Setup();
	static AABB ComputeAABB(const std::vector<Vertex>& vertices);
	//GLuint materialUBO;
	//matAsset mat;
	//int albedoIdx = 0;
	//int normalMapIdx = 0;
	//int currMatIdx = 0;
};

struct SkinnedMeshAsset : public IAsset
{

	SkinnedMeshAsset() : correction(1.0f) {}

	std::string original_name;


	// Multiple submeshes, like your static model
	std::vector<SubMeshSkinned> submeshes;

	// Bones & animation data
	std::vector<AnimationClip> animations;
	std::vector<Bone> bones;
	std::unordered_map<std::string, int> boneNameToIndex;

	// axis up correction
	glm::mat4 correction;
	~SkinnedMeshAsset();

	void Load(std::filesystem::path file) override;
	void Unload() override;
	void WriteToBinary(std::string new_file) override;
	void LoadFromBinary(std::filesystem::path path) override;

	ASSET_TYPE GetType() const override { return ASSET_TYPE::MODEL; }
	void GetMetaData(std::string new_file, int& vert, int& subm, std::vector<std::string>& mats);
	void SetUpMinMax();


};
