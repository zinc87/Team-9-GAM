#pragma once
#include "pch.h"
#include "IAsset.h"

enum class TextureCompressionFormat
{
	None,   // Uncompressed R8G8B8A8 (your current behavior)
	BC1,    // DXT1 (RGB, 1-bit alpha)
	BC3,    // DXT5 (RGBA)
	BC5,    // 2-channel (RRGG, for normal maps)
	BC7     // High-quality RGBA
};

struct DDSMetadataFooter
{
	uint32_t magic = 0xDEADBEEF;
	int colorSpace;       // 0 = Linear, 1 = sRGB
	int wrapMode;         // 0 = Repeat, 1 = Clamp, 2 = Mirror
	int filterMode;       // 0 = Nearest, 1 = Linear
	bool generateMipMap;  // True if mipmaps were generated
	int channel;          // 3 or 4
	int width;
	int height;

	TextureCompressionFormat compressionFormat;

};

struct TextureAsset : public IAsset
{

	GLuint textureID = (GLuint)0;
	std::pair<int, int> size;
	int channel = 0; // GL_RGB = 3 / GL_RGBA = 4
	int colorSpace = 0;     // 0 = Linear, 1 = sRGB
	int wrapMode = 0;       // 0 Repeat, 1 Clamp, 2 Mirror
	int filterMode = 1;     // 0 Nearest, 1 Linear
	bool generateMipMap = true;

	void Load(std::filesystem::path file) override;
	void LoadEx(std::filesystem::path file, int _colorSpace, int _wrapMode, int _filterMode, bool _generateMipMap);
	void Unload() override;
	void WriteToBinary(std::string new_file) override;
	void LoadFromBinary(std::filesystem::path path) override;

	bool ConvertToDDS(std::filesystem::path input,
		std::string output,
		int _colorSpace,
		int _wrapMode,
		int _filterMode,
		bool _generateMipMap,
		TextureCompressionFormat compression_Format, int maxTextureSizeIndex, bool generateNormalMap = false,  // <--- ADD THIS (with default)
		float normalMapStrength = 2.0f);
	bool LoadFromDDS(std::filesystem::path input);
	ASSET_TYPE GetType() const override { return ASSET_TYPE::TEXTURE; }

	TextureCompressionFormat compressionFormat = TextureCompressionFormat::None;

};
