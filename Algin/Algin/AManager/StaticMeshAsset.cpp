#include "pch.h"
#include "StaticMeshAsset.h"

void SubMesh::Setup()
{

	if (VAO == 0) glGenVertexArrays(1, &VAO);
	if (VBO == 0) glGenBuffers(1, &VBO);
	if (IBO == 0) glGenBuffers(1, &IBO);

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER,
		vertices.size() * sizeof(Vertex),
		vertices.data(), GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER,
		indices.size() * sizeof(unsigned int),
		indices.data(), GL_STATIC_DRAW);

	GLsizei stride = sizeof(Vertex);

	// position
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)offsetof(Vertex, position));

	// normal
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)offsetof(Vertex, normal));

	// texcoord
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (void*)offsetof(Vertex, texcoord));

	//tangent
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, stride, (void*)offsetof(Vertex, tangent));

	//bitangent
	glEnableVertexAttribArray(4);
	glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, stride, (void*)offsetof(Vertex, bitangent));

	glBindVertexArray(0);
}


StaticMeshAsset::~StaticMeshAsset()
{
	for (auto& sm : submeshes) {
		if (sm.VAO) glDeleteVertexArrays(1, &sm.VAO);
		if (sm.VBO) glDeleteBuffers(1, &sm.VBO);
		if (sm.IBO) glDeleteBuffers(1, &sm.IBO);
	}
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
		AG_CORE_WARN("Failed to load mesh: {}", file.generic_string());
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
		sub.Setup();

		submeshes.push_back(sub);
	}
	
}


void StaticMeshAsset::Unload()
{
	submeshes.clear();
}

void StaticMeshAsset::WriteToBinary(std::string new_file)
{
	std::ofstream out(new_file, std::ios::binary);
	if (!out.is_open())
	{
		AG_CORE_WARN("Failed to write mesh to binary: {}", new_file);
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


	glm::vec3 min(std::numeric_limits<float>::max());
	glm::vec3 max(std::numeric_limits<float>::lowest());

	// 1. Find the Bounding Box (Min and Max)
	for (const auto& sub : submeshes)
	{
		totalVertices += sub.vertices.size();
		for (const auto& vertex : sub.vertices)
		{
			// Find minimum coordinates
			min.x = std::min(min.x, vertex.position[0]);
			min.y = std::min(min.y, vertex.position[1]);
			min.z = std::min(min.z, vertex.position[2]);

			// Find maximum coordinates
			max.x = std::max(max.x, vertex.position[0]);
			max.y = std::max(max.y, vertex.position[1]);
			max.z = std::max(max.z, vertex.position[2]);
		}
	}

	// 2. Calculate the Centroid (Center of the Bounding Box)
	glm::vec3 centroid = (min + max) * 0.5f;

	// 3. Apply the Translation (Shift all vertices by the negative centroid)
	if (totalVertices > 0)
	{
		for (auto& sub : submeshes) // Use auto& to modify the vertices directly
		{
			for (auto& vertex : sub.vertices)
			{
				// Shift the position vector of the vertex
				vertex.position[0] -= centroid.x;
				vertex.position[1] -= centroid.y;
				vertex.position[2] -= centroid.z;
			}
		}
	}


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
		AG_CORE_WARN("Failed to compress mesh data: {}", new_file);
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


void StaticMeshAsset::LoadFromBinary(std::filesystem::path path)
{
	std::ifstream in(path, std::ios::binary);
	if (!in.is_open())
	{
		AG_CORE_WARN("Failed to load mesh from binary: {}", path.string());
		return;
	}

	submeshes.clear();

	// -------------------------------------------------------
	// Step 1: Read metadata until [BINARY_START]
	// -------------------------------------------------------
	std::string line;
	size_t compressedSize = 0;
	size_t originalSize = 0;
	while (std::getline(in, line))
	{
		if (line.rfind("CompressedSize:", 0) == 0)
			compressedSize = std::stoull(line.substr(15));
		else if (line.rfind("OriginalSize:", 0) == 0)
			originalSize = std::stoull(line.substr(13));
		else if (line == "[BINARY_START]")
			break;
	}

	if (compressedSize == 0 || originalSize == 0)
	{
		AG_CORE_WARN("Malformed mesh file or missing compression info: {}", path.string());
		return;
	}

	// -------------------------------------------------------
	// Step 2: Read compressed binary data
	// -------------------------------------------------------
	std::vector<unsigned char> compressedData(compressedSize);
	in.read(reinterpret_cast<char*>(compressedData.data()), compressedSize);
	in.close();

	// -------------------------------------------------------
	// Step 3: Decompress
	// -------------------------------------------------------
	std::vector<unsigned char> rawData;
	if (!DecompressBuffer(compressedData, rawData, originalSize))
	{
		AG_CORE_WARN("Failed to decompress mesh data: {}", path.string());
		return;
	}

	// -------------------------------------------------------
	// Step 4: Deserialize mesh structures
	// -------------------------------------------------------
	std::istringstream iss(std::string(rawData.begin(), rawData.end()), std::ios::binary);

	// Original name
	uint32_t origNameLen = 0;
	iss.read(reinterpret_cast<char*>(&origNameLen), sizeof(uint32_t));
	original_name.resize(origNameLen);
	if (origNameLen > 0)
		iss.read(&original_name[0], origNameLen);

	// Submesh count
	uint32_t submeshCount = 0;
	iss.read(reinterpret_cast<char*>(&submeshCount), sizeof(uint32_t));

	for (uint32_t i = 0; i < submeshCount; ++i)
	{
		SubMesh sub;

		uint32_t nameLen = 0;
		iss.read(reinterpret_cast<char*>(&nameLen), sizeof(uint32_t));
		sub.name.resize(nameLen);
		if (nameLen > 0)
			iss.read(&sub.name[0], nameLen);

		uint32_t vCount = 0, iCount = 0;
		iss.read(reinterpret_cast<char*>(&vCount), sizeof(uint32_t));
		iss.read(reinterpret_cast<char*>(&iCount), sizeof(uint32_t));

		sub.vertices.resize(vCount);
		if (vCount > 0)
			iss.read(reinterpret_cast<char*>(sub.vertices.data()), sizeof(Vertex) * vCount);

		sub.indices.resize(iCount);
		if (iCount > 0)
			iss.read(reinterpret_cast<char*>(sub.indices.data()), sizeof(unsigned int) * iCount);

		sub.Setup();
		submeshes.push_back(std::move(sub));
	}
	SetUpMinMax();
}


void StaticMeshAsset::GetMetaData(std::string new_file, int& vert, int& subm, std::vector<std::string>& mats)
{
	Assimp::Importer importer;

	const aiScene* scene = importer.ReadFile(
		new_file,
		aiProcess_Triangulate | aiProcess_JoinIdenticalVertices
	);

	if (!scene || !scene->mRootNode)
	{
		AG_CORE_WARN("Failed to load mesh: {}", new_file);
		vert = 0;
		subm = 0;
		mats.clear();
		return;
	}

	int totalVertices = 0;
	int totalSubmeshes = 0;
	std::set<std::string> uniqueTextures;

	for (unsigned int i = 0; i < scene->mNumMeshes; ++i)
	{
		aiMesh* mesh = scene->mMeshes[i];
		totalVertices += mesh->mNumVertices;
		totalSubmeshes++;

		int matIndex = mesh->mMaterialIndex;
		if (matIndex >= 0 && matIndex < (int)scene->mNumMaterials)
		{
			aiMaterial* material = scene->mMaterials[matIndex];
			aiString texPath;

			// Albedo / Diffuse
			if (material->GetTexture(aiTextureType_DIFFUSE, 0, &texPath) == AI_SUCCESS)
				uniqueTextures.insert(std::filesystem::path(texPath.C_Str()).filename().string());

			// Normal map (check multiple slots)
			if (material->GetTexture(aiTextureType_NORMALS, 0, &texPath) == AI_SUCCESS ||
				material->GetTexture(aiTextureType_HEIGHT, 0, &texPath) == AI_SUCCESS ||
				material->GetTexture(aiTextureType_UNKNOWN, 0, &texPath) == AI_SUCCESS)
			{
				uniqueTextures.insert(std::filesystem::path(texPath.C_Str()).filename().string());
			}

			// Metallic
			if (material->GetTexture(aiTextureType_METALNESS, 0, &texPath) == AI_SUCCESS)
				uniqueTextures.insert(std::filesystem::path(texPath.C_Str()).filename().string());

			// Roughness
			if (material->GetTexture(aiTextureType_DIFFUSE_ROUGHNESS, 0, &texPath) == AI_SUCCESS)
				uniqueTextures.insert(std::filesystem::path(texPath.C_Str()).filename().string());

			// AO
			if (material->GetTexture(aiTextureType_AMBIENT_OCCLUSION, 0, &texPath) == AI_SUCCESS)
				uniqueTextures.insert(std::filesystem::path(texPath.C_Str()).filename().string());

		}
	}

	vert = totalVertices;
	subm = totalSubmeshes;

	mats.assign(uniqueTextures.begin(), uniqueTextures.end());
}

void StaticMeshAsset::SetUpMinMax()
{
	for (auto& subm : submeshes)
	{
		// Initialize with extreme values
		subm.minmax.min = glm::vec3(
			std::numeric_limits<float>::max(),
			std::numeric_limits<float>::max(),
			std::numeric_limits<float>::max()
		);

		subm.minmax.max = glm::vec3(
			std::numeric_limits<float>::lowest(),
			std::numeric_limits<float>::lowest(),
			std::numeric_limits<float>::lowest()
		);

		// Iterate through vertices to find bounds
		for (const auto& v : subm.vertices)
		{
			glm::vec3 glm_v = { v.position[0], v.position[1],v.position[2] };
			subm.minmax.min = glm::min(subm.minmax.min, glm_v);
			subm.minmax.max = glm::max(subm.minmax.max, glm_v);
		}
	}
}


void StaticMeshAsset::ApplyScaling(float x, float y, float z)
{
	// 1. Optimization: If scale is 1,1,1, do nothing.
	if (x == 1.0f && y == 1.0f && z == 1.0f) return;

	// 2. Check if scaling is non-uniform (e.g., 1, 2, 1)
	// If it is, normals need special math (Inverse Transpose).
	bool isNonUniform = (std::abs(x - y) > 1e-5f || std::abs(y - z) > 1e-5f || std::abs(x - z) > 1e-5f);

	// Pre-calculate inverse scale for normals (avoid division in loop)
	float invX = (x != 0.0f) ? 1.0f / x : 0.0f;
	float invY = (y != 0.0f) ? 1.0f / y : 0.0f;
	float invZ = (z != 0.0f) ? 1.0f / z : 0.0f;

	for (auto& sub : submeshes)
	{
		for (auto& v : sub.vertices)
		{
			// --- POSITION ---
			// Transform by Model Matrix (Scale)
			v.position[0] *= x;
			v.position[1] *= y;
			v.position[2] *= z;

			// --- NORMALS ---
			// Transform by Inverse Transpose of Model Matrix
			if (isNonUniform)
			{
				v.normal[0] *= invX;
				v.normal[1] *= invY;
				v.normal[2] *= invZ;
			}
			// Note: If scale is uniform (e.g., 2,2,2), the direction 
			// of the normal doesn't change, so we skip the multiply.

			// Always re-normalize normals to ensure they are length 1.0
			glm::vec3 n(v.normal[0], v.normal[1], v.normal[2]);
			n = glm::normalize(n);
			v.normal[0] = n.x; v.normal[1] = n.y; v.normal[2] = n.z;

			// --- TANGENTS & BITANGENTS ---
			// These lie ON the surface, so they scale with the geometry.
			// However, we must re-normalize them to keep lighting correct.

			// Tangent
			v.tangent[0] *= x;
			v.tangent[1] *= y;
			v.tangent[2] *= z;
			glm::vec3 t(v.tangent[0], v.tangent[1], v.tangent[2]);
			if (glm::length(t) > 0.0f) {
				t = glm::normalize(t);
				v.tangent[0] = t.x; v.tangent[1] = t.y; v.tangent[2] = t.z;
			}

			// Bitangent
			v.bitangent[0] *= x;
			v.bitangent[1] *= y;
			v.bitangent[2] *= z;
			glm::vec3 b(v.bitangent[0], v.bitangent[1], v.bitangent[2]);
			if (glm::length(b) > 0.0f) {
				b = glm::normalize(b);
				v.bitangent[0] = b.x; v.bitangent[1] = b.y; v.bitangent[2] = b.z;
			}
		}
	}

	// 3. CRITICAL: Re-calculate the AABB (Bounding Box)
	// Since the positions changed, the old bounding box is wrong.
	SetUpMinMax();
}

