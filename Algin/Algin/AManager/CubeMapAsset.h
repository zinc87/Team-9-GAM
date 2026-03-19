#pragma once
#include "pch.h"

struct CubeMapAsset : public IAsset
{
	GLuint textureID = 0;
	std::vector<std::string> faces_name;
	std::pair<int, int> size;
	// generate from loaded texture, front back left right top down sequence;
	void Load(std::filesystem::path path) override { (void)path; }
	void Unload() override {}
	void WriteToBinary(std::string new_file) override;
	void LoadFromBinary(std::filesystem::path path) override;

	void GenerateCubeMap(std::vector<std::size_t>& faces, std::string output_name);
	ASSET_TYPE GetType() const override { return ASSET_TYPE::AGCUBEMAP; }
};