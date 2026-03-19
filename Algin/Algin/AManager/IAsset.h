#pragma once
#include "../Header/pch.h"
#include <zlib.h>

enum ASSET_TYPE
{
	UNKNOWN,
	TEXTURE,
	MODEL,
	AUDIO,
	SCRIPT,
	BHT,
	AGCUBEMAP,
	FONT,
	DDS,
	AGSKINNEDMESH,
	PREFAB,
	SCENE,
	AGMATERIAL,
	AGTEXTURE,
	AGSTATICMESH,
	AGDYNAMICMESH,
	AGAUDIO,
	AGFONT,
};

bool CompressBuffer(const std::vector<unsigned char>& input, std::vector<unsigned char>& output);

bool DecompressBuffer(const std::vector<unsigned char>& input, std::vector<unsigned char>& output, size_t originalSize);


struct IAsset
{
	std::string asset_name;
	std::string ori_path;
	virtual ~IAsset() = default;
	virtual ASSET_TYPE GetType() const = 0;
	std::string GetName() const { return asset_name; }

	virtual void Load(std::filesystem::path file) = 0;
	virtual void Unload() = 0;
	virtual void WriteToBinary(std::string new_file) = 0;
	virtual void LoadFromBinary(std::filesystem::path path) = 0;
};

