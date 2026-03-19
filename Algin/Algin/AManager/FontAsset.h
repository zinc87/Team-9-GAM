#pragma once
#include "pch.h"

struct Glyphs
{
	float advanceX;
	float advanceY;

	float width;
	float height;

	float bearingX;
	float bearingY;

	int atlasX;
	int atlasY;


	float texCoord[4][2];
	/*
	texCoord[0][0]
	top left
	top right
	bot right
	bot left
	*/

};

struct FontAsset : public IAsset
{
	std::string m_original_name = "";
	int			m_channel = 0;
	int			m_width = 0;
	int			m_height = 0;
	GLuint		m_textureID = 0;

	std::unordered_map<char,Glyphs>	m_glyphs_data;

	void Load(std::filesystem::path path) override;
	void Unload() override;
	void WriteToBinary(std::string new_file) override;
	void LoadFromBinary(std::filesystem::path path) override;

	void GenerateCubeMap(std::vector<std::size_t>& faces, std::string output_name);
	ASSET_TYPE GetType() const override { return ASSET_TYPE::AGFONT; }
};

