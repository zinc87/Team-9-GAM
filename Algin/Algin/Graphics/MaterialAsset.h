#pragma once
#include "../Header/pch.h"

#include <rapidjson.h>
#include <fstream>


struct MaterialParams {
	glm::vec4 baseColor;        // rgba
	glm::vec<4, float> mru_pad; // {metallic, roughness, uvScale, pad}
};


struct MaterialObj {
	glm::vec3 pos;
	glm::vec3 rot;
	glm::vec3 scale;

	MaterialParams params;
	size_t albedoHashID;
	size_t normalMapHashID;
};


//for saving and loading a material asset
struct MaterialAsset {

	std::string name;
	glm::vec4 baseColor;
	float metallic;
	float roughness;
	float UvScale;
	float tint;

	size_t albedoHASHid;
	size_t normalMapHashID;

	MaterialAsset() : name{ "My Material" }, baseColor{ glm::vec4(1.f,1.f,1.f,1.f) },
		metallic{ 0.f }, roughness{ 0.f }, UvScale{ 1.f }, tint{ 1.f },
		albedoHASHid{ 0 }, normalMapHashID{ 0 } {
	}

	static MaterialAsset save_mat_ass;
};

struct matAsset {
	MaterialParams params;
	size_t albedoHashID;
	size_t normalMapHashID;

	matAsset();
};


class MaterialSetup : public AG::Pattern::ISingleton<MaterialSetup> {
public:

	matAsset matParams;

	std::vector<MaterialAsset> materialAssets;
	std::vector<std::string> materialPaths;

	void init();
	bool parseMaterial(const std::string& filePath, MaterialAsset& outMat);
	bool saveMatToJS(const MaterialAsset& mat, std::string& filePath);
};