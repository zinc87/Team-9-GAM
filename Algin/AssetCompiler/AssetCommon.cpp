#include "AssetCommon.h"

bool CompressBuffer(const std::vector<unsigned char>& input, std::vector<unsigned char>& output)
{
	uLongf outSize = compressBound(static_cast<uLong>(input.size()));
	output.resize(outSize);

	int res = compress(
		reinterpret_cast<Bytef*>(output.data()),
		&outSize, // <-- FIX: pass pointer, not cast
		reinterpret_cast<const Bytef*>(input.data()),
		static_cast<uLong>(input.size())
	);

	if (res != Z_OK)
		return false;

	output.resize(outSize); // shrink to actual size
	return true;
}

// ------------------------
// Decompress helper
// ------------------------
bool DecompressBuffer(const std::vector<unsigned char>& input, std::vector<unsigned char>& output, size_t originalSize)
{
	output.resize(originalSize);
	uLongf outSize = static_cast<uLongf>(originalSize);

	int res = uncompress(
		reinterpret_cast<Bytef*>(output.data()),
		&outSize, // <-- FIX: pass pointer, not cast
		reinterpret_cast<const Bytef*>(input.data()),
		static_cast<uLong>(input.size())
	);

	return (res == Z_OK);
}

void StaticMeshAsset::Load(std::filesystem::path file)
{
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(
		file.string(),
		aiProcess_Triangulate |
		aiProcess_GenSmoothNormals |
		aiProcess_FlipUVs |
		aiProcess_JoinIdenticalVertices |
		aiProcess_PreTransformVertices
	);

	if (!scene || !scene->HasMeshes())
	{
		std::cout << "Failed to load mesh: " << file << std::endl;
		return;
	}
	original_name = file.filename().string();
	submeshes.clear();
	submeshes.reserve(scene->mNumMeshes);

	for (unsigned int m = 0; m < scene->mNumMeshes; ++m)
	{
		aiMesh* aimesh = scene->mMeshes[m];
		SubMesh sub;
		sub.name = std::string(aimesh->mName.C_Str());
		sub.name += "_mat" + std::to_string(aimesh->mMaterialIndex);

		// --- Vertices ---
		sub.vertices.reserve(aimesh->mNumVertices);
		for (unsigned int v = 0; v < aimesh->mNumVertices; ++v)
		{
			Vertex vert{};

			// Position
			vert.position[0] = aimesh->mVertices[v].x;
			vert.position[1] = aimesh->mVertices[v].y;
			vert.position[2] = aimesh->mVertices[v].z;

			// Normal
			if (aimesh->HasNormals())
			{
				vert.normal[0] = aimesh->mNormals[v].x;
				vert.normal[1] = aimesh->mNormals[v].y;
				vert.normal[2] = aimesh->mNormals[v].z;
			}

			// Texcoords
			if (aimesh->HasTextureCoords(0))
			{
				vert.texcoord[0] = aimesh->mTextureCoords[0][v].x;
				vert.texcoord[1] = 1.0f - aimesh->mTextureCoords[0][v].y; // flip Y
			}

			// Tangent / Bitangent
			if (aimesh->HasTangentsAndBitangents())
			{
				vert.tangent[0] = aimesh->mTangents[v].x;
				vert.tangent[1] = aimesh->mTangents[v].y;
				vert.tangent[2] = aimesh->mTangents[v].z;

				vert.bitangent[0] = aimesh->mBitangents[v].x;
				vert.bitangent[1] = aimesh->mBitangents[v].y;
				vert.bitangent[2] = aimesh->mBitangents[v].z;
			}
			else
			{
				vert.tangent[0] = 1.f; vert.tangent[1] = 0.f; vert.tangent[2] = 0.f;
				vert.bitangent[0] = 0.f; vert.bitangent[1] = 1.f; vert.bitangent[2] = 0.f;
			}

			// Default color
			vert.color[0] = vert.color[1] = vert.color[2] = 1.0f;
			vert.color[3] = 1.0f;

			sub.vertices.push_back(vert);
		}

		// --- Indices ---
		for (unsigned int f = 0; f < aimesh->mNumFaces; ++f)
		{
			const aiFace& face = aimesh->mFaces[f];
			for (unsigned int i = 0; i < face.mNumIndices; ++i)
				sub.indices.push_back(face.mIndices[i]);
		}

		// Setup VAO/VBO/IBO for the submesh

		submeshes.push_back(sub);
	}

}

void StaticMeshAsset::WriteToBinary(std::string new_file)
{
	std::cout << "Write To Binary" << std::endl;
	std::ofstream out(new_file, std::ios::binary);
	if (!out.is_open())
	{
		return;
	}

	// -------------------------------------------------------
	// Step 1: Write Human-readable metadata
	// -------------------------------------------------------
	size_t totalVertices = 0;
	size_t totalIndices = 0;
	for (const auto& sub : submeshes)
	{
		totalVertices += sub.vertices.size();
		totalIndices += sub.indices.size();
	}

	std::string fname = std::filesystem::path(new_file).filename().generic_string();

	out << "=== StaticMesh Metadata ===\n";
	out << "File: " << fname << "\n";
	out << "Original Asset: " << original_name << "\n";
	out << "No. of submeshes: " << submeshes.size() << "\n";
	out << "Total vertices: " << totalVertices << "\n";
	out << "Total indices: " << totalIndices << "\n";
	out << "----------------------------\n";
	out.flush();

	// -------------------------------------------------------
	// Step 2: Prepare binary payload in memory
	// -------------------------------------------------------
	std::vector<unsigned char> rawBinary;
	{
		std::ostringstream oss(std::ios::binary);
		// Original name
		uint32_t origNameLen = static_cast<uint32_t>(original_name.size());
		oss.write(reinterpret_cast<const char*>(&origNameLen), sizeof(uint32_t));
		oss.write(original_name.data(), origNameLen);

		// Submesh count
		uint32_t submeshCount = static_cast<uint32_t>(submeshes.size());
		oss.write(reinterpret_cast<const char*>(&submeshCount), sizeof(uint32_t));

		// Each submesh
		for (const auto& sub : submeshes)
		{
			// name
			uint32_t nameLen = static_cast<uint32_t>(sub.name.size());
			oss.write(reinterpret_cast<const char*>(&nameLen), sizeof(uint32_t));
			oss.write(sub.name.data(), nameLen);

			// vertex/index count
			uint32_t vCount = static_cast<uint32_t>(sub.vertices.size());
			uint32_t iCount = static_cast<uint32_t>(sub.indices.size());
			oss.write(reinterpret_cast<const char*>(&vCount), sizeof(uint32_t));
			oss.write(reinterpret_cast<const char*>(&iCount), sizeof(uint32_t));

			// vertex + index data
			if (!sub.vertices.empty())
				oss.write(reinterpret_cast<const char*>(sub.vertices.data()), sizeof(Vertex) * vCount);
			if (!sub.indices.empty())
				oss.write(reinterpret_cast<const char*>(sub.indices.data()), sizeof(unsigned int) * iCount);
		}

		const std::string& temp = oss.str();
		rawBinary.assign(temp.begin(), temp.end());
	}

	// -------------------------------------------------------
	// Step 3: Compress the binary data
	// -------------------------------------------------------
	std::vector<unsigned char> compressedData;
	if (!CompressBuffer(rawBinary, compressedData))
	{
		out.close();
		return;
	}

	uint64_t compressedSize = compressedData.size();
	uint64_t originalSize = rawBinary.size();

	// Write the compression info header
	out << "CompressedSize: " << compressedSize << "\n";
	out << "OriginalSize: " << originalSize << "\n";
	out << "[BINARY_START]\n";
	out.flush();

	// Write compressed payload
	out.write(reinterpret_cast<const char*>(compressedData.data()), compressedData.size());

	out.close();
}