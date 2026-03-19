#include "pch.h"
#include "CubeMapAsset.h"

void CubeMapAsset::GenerateCubeMap(std::vector<size_t>& faces, std::string output_name)
{
	if (faces.size() != 6)
	{
		std::cerr << "[CubeMapAsset] Error: Expected 6 faces for cubemap but got "
			<< faces.size() << std::endl;
		return;
	}

	// Store metadata
	asset_name = output_name;

	// Generate cubemap texture
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

	int targetWidth = -1, targetHeight = -1;

	for (unsigned int i = 0; i < faces.size(); i++)
	{
		auto asset = AssetManager::GetInstance().GetAsset(faces[i]).lock();
		auto tex_asset = std::dynamic_pointer_cast<TextureAsset>(asset);

		if (!tex_asset)
		{
			AG_CORE_WARN("[CubeMapAsset] Error: Face texture not found or invalid: {}", faces[i]);
			continue;
		}

		int width = tex_asset->size.first;
		int height = tex_asset->size.second;
		int channel = tex_asset->channel;
		GLenum format = (channel == 4) ? GL_RGBA : GL_RGB;

		// Use the first face’s size as reference
		if (targetWidth == -1 && targetHeight == -1)
		{
			targetWidth = width;
			targetHeight = height;
			size = { targetWidth, targetHeight }; // store in asset
		}
		else
		{
			// Warn if not consistent
			if (width != targetWidth || height != targetHeight)
			{
				AG_CORE_WARN("[CubeMapAsset] Face {} size {}x{} does not match expected {}x{}",
					faces[i], width, height, targetWidth, targetHeight);
			}
		}

		// Download pixels from the 2D texture
		glBindTexture(GL_TEXTURE_2D, tex_asset->textureID);
		std::vector<unsigned char> pixels(width * height * channel);
		glGetTexImage(GL_TEXTURE_2D, 0, format, GL_UNSIGNED_BYTE, pixels.data());

		// Upload into cubemap face
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
			0, format, targetWidth, targetHeight, 0, // enforce uniform size
			format, GL_UNSIGNED_BYTE,
			pixels.data());

		std::cout << "[CubeMapAsset] Attached face " << i
			<< " (" << faces[i] << ")"
			<< " size=" << width << "x" << height
			<< " channels=" << channel
			<< " texID=" << tex_asset->textureID
			<< std::endl;
	}

	// Set filtering/wrapping for cubemap
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

	std::cout << "[CubeMapAsset] Cubemap generated: " << output_name
		<< " (ID=" << textureID << ", size=" << size.first << "x" << size.second << ")" << std::endl;
}
void CubeMapAsset::WriteToBinary(std::string new_file)
{
	if (textureID == 0)
	{
		std::cerr << "[CubeMapAsset] No cubemap texture to write!" << std::endl;
		return;
	}

	int width = size.first;
	int height = size.second;
	int channels = 4; // store as RGBA always
	GLenum format = GL_RGBA;
	int faceCount = 6;

	std::ofstream out(new_file, std::ios::binary);
	if (!out.is_open())
	{
		std::cerr << "[CubeMapAsset] Failed to open file for writing: " << new_file << std::endl;
		return;
	}

	// --- HEADER ---
	out.write(reinterpret_cast<char*>(&width), sizeof(int));
	out.write(reinterpret_cast<char*>(&height), sizeof(int));
	out.write(reinterpret_cast<char*>(&channels), sizeof(int));
	out.write(reinterpret_cast<char*>(&faceCount), sizeof(int));

	std::vector<unsigned char> pixels(width * height * channels);

	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

	for (int i = 0; i < faceCount; ++i)
	{
		// Fetch each cubemap face
		glGetTexImage(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, format, GL_UNSIGNED_BYTE, pixels.data());

		// Compress the face
		std::vector<unsigned char> compressed;
		if (!CompressBuffer(pixels, compressed))
		{
			std::cerr << "[CubeMapAsset] Compression failed for face " << i << std::endl;
			return;
		}

		// Write sizes and data
		size_t originalSize = pixels.size();
		size_t compressedSize = compressed.size();

		out.write(reinterpret_cast<char*>(&originalSize), sizeof(size_t));
		out.write(reinterpret_cast<char*>(&compressedSize), sizeof(size_t));
		out.write(reinterpret_cast<char*>(compressed.data()), compressedSize);
	}

	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
	out.close();

	std::cout << "[CubeMapAsset] Compressed cubemap written to " << new_file
		<< " (" << width << "x" << height << ", " << channels
		<< " channels, 6 faces)" << std::endl;
}

void CubeMapAsset::LoadFromBinary(std::filesystem::path path)
{
	std::ifstream in(path, std::ios::binary);
	if (!in.is_open())
	{
		std::cerr << "[CubeMapAsset] Failed to open file for reading: " << path << std::endl;
		return;
	}

	// --- HEADER ---
	int width, height, channels, faceCount;
	in.read(reinterpret_cast<char*>(&width), sizeof(int));
	in.read(reinterpret_cast<char*>(&height), sizeof(int));
	in.read(reinterpret_cast<char*>(&channels), sizeof(int));
	in.read(reinterpret_cast<char*>(&faceCount), sizeof(int));

	if (faceCount != 6)
	{
		std::cerr << "[CubeMapAsset] Invalid face count in binary file: " << faceCount << std::endl;
		return;
	}

	size = { width, height };
	GLenum format = (channels == 4) ? GL_RGBA : GL_RGB;

	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

	for (int i = 0; i < faceCount; ++i)
	{
		// Read sizes
		size_t originalSize = 0, compressedSize = 0;
		in.read(reinterpret_cast<char*>(&originalSize), sizeof(size_t));
		in.read(reinterpret_cast<char*>(&compressedSize), sizeof(size_t));

		// Read compressed data
		std::vector<unsigned char> compressed(compressedSize);
		in.read(reinterpret_cast<char*>(compressed.data()), compressedSize);

		// Decompress
		std::vector<unsigned char> pixels;
		if (!DecompressBuffer(compressed, pixels, originalSize))
		{
			std::cerr << "[CubeMapAsset] Decompression failed for face " << i << std::endl;
			return;
		}

		// Upload to GPU
		glTexImage2D(
			GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
			0, format,
			width, height, 0,
			format, GL_UNSIGNED_BYTE,
			pixels.data()
		);
	}

	// Set cubemap parameters
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
	in.close();

	std::cout << "[CubeMapAsset] Loaded (decompressed) cubemap from " << path
		<< " (" << width << "x" << height << ", " << channels << " channels)" << std::endl;
}
