#include "pch.h"
#include "SkinnedMeshAsset.h"

SkinnedMeshAsset::~SkinnedMeshAsset()
{
}

void SubMeshSkinned::Setup()
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

    //boneID
    glEnableVertexAttribArray(5);
    glVertexAttribIPointer(5, 4, GL_INT, stride, (void*)offsetof(Vertex, boneIDs));

    //weights
    glEnableVertexAttribArray(6);
    glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, stride, (void*)offsetof(Vertex, weights));

    glBindVertexArray(0);
    minmax = ComputeAABB(vertices);
}


void SkinnedMeshAsset::Load(std::filesystem::path file)
{
    Assimp::Importer importer;

    const aiScene* scene = importer.ReadFile(
        file.string(),
        aiProcess_Triangulate |
        aiProcess_GenSmoothNormals |
        aiProcess_LimitBoneWeights |
        aiProcess_JoinIdenticalVertices |
        aiProcess_CalcTangentSpace
    );

    if (!scene || !scene->mRootNode)
    {
        AG_CORE_WARN("[ASSET][SKINNED_MESH] Failed to load FBX: {}", file.string());
        return;
    }

    original_name = file.filename().string();
    submeshes.clear();
    bones.clear();
    animations.clear();
    boneNameToIndex.clear();
    std::unordered_map<std::string, int> nameCounts;

    //  Fix FBX coordinate mismatch (Y-up to OpenGL)
    int UpAxis = -1;
	int AxisSign = 0;
    scene->mMetaData->Get("UpAxis", UpAxis);
	scene->mMetaData->Get("UpAxisSign", AxisSign);
	//AG_CORE_ERROR("[ASSET][SKINNED_MESH] FBX UpAxis: {}, UpAxisSign: {}", UpAxis, AxisSign);

    if (UpAxis == 0)// X-up
    {
        //AG_CORE_ERROR("[ASSET][SKINNED_MESH] FBX file uses X-up axis, applying correction.");

    }
    else if (UpAxis == 1) // Y-UP
    {
        //AG_CORE_ERROR("[ASSET][SKINNED_MESH] FBX file uses Y-up axis, applying correction.");
        correction = glm::rotate(glm::mat4(1.0f),
            glm::radians(-90.0f),
            glm::vec3(1, 0, 0));

    }
    else if (UpAxis == 2) // Z-up
    {
        //AG_CORE_ERROR("[ASSET][SKINNED_MESH] FBX file uses Z-up axis, applying correction.");
    }


    // ─────────────────────────────
    // 1️⃣ Load submeshes
    // ─────────────────────────────
    for (unsigned int m = 0; m < scene->mNumMeshes; ++m)
    {
        aiMesh* mesh = scene->mMeshes[m];
        SubMeshSkinned sub{};

        std::string baseName = mesh->mName.C_Str();
        if (baseName.empty()) baseName = "SubMesh";

        int& count = nameCounts[baseName];
        sub.name = (count == 0) ? baseName : baseName + "_" + std::to_string(count);
        ++count;

        sub.vertices.resize(mesh->mNumVertices);
        sub.indices.reserve(mesh->mNumFaces * 3);

        // Vertex data
        for (unsigned int i = 0; i < mesh->mNumVertices; ++i)
        {
            Vertex& v = sub.vertices[i];

            if (mesh->HasPositions())
            {
                v.position[0] = mesh->mVertices[i].x;
                v.position[1] = mesh->mVertices[i].y;
                v.position[2] = mesh->mVertices[i].z;
            }
            if (mesh->HasNormals())
            {
                v.normal[0] = mesh->mNormals[i].x;
                v.normal[1] = mesh->mNormals[i].y;
                v.normal[2] = mesh->mNormals[i].z;
            }
            if (mesh->HasTextureCoords(0))
            {
                v.texcoord[0] = mesh->mTextureCoords[0][i].x;
                v.texcoord[1] = mesh->mTextureCoords[0][i].y;
            }
            if (mesh->HasTangentsAndBitangents())
            {
                v.tangent[0] = mesh->mTangents[i].x;
                v.tangent[1] = mesh->mTangents[i].y;
                v.tangent[2] = mesh->mTangents[i].z;

                v.bitangent[0] = mesh->mBitangents[i].x;
                v.bitangent[1] = mesh->mBitangents[i].y;
                v.bitangent[2] = mesh->mBitangents[i].z;
            }
        }

        // Index data
        for (unsigned int f = 0; f < mesh->mNumFaces; ++f)
        {
            const aiFace& face = mesh->mFaces[f];
            for (unsigned int j = 0; j < face.mNumIndices; ++j)
                sub.indices.push_back(face.mIndices[j]);
        }

        // ─────────────────────────────
        // 2️⃣ Load bones
        // ─────────────────────────────
        for (unsigned int b = 0; b < mesh->mNumBones; ++b)
        {
            aiBone* aBone = mesh->mBones[b];
            std::string boneName = aBone->mName.C_Str();

            int boneIndex = 0;
            auto it = boneNameToIndex.find(boneName);
            if (it == boneNameToIndex.end())
            {
                boneIndex = static_cast<int>(bones.size());
                Bone bone{};
                bone.name = boneName;

                // ✅ Assimp offset matrix = inverse bind pose (model → bone)
                bone.inverseBindPose = glm::transpose(glm::make_mat4(&aBone->mOffsetMatrix.a1));
                bone.bindPose = glm::inverse(bone.inverseBindPose);
                bone.localTransform = glm::mat4(1.0f);

                bones.push_back(bone);
                boneNameToIndex[boneName] = boneIndex;
            }
            else
            {
                boneIndex = it->second;
            }

            // Assign vertex weights
            for (unsigned int w = 0; w < aBone->mNumWeights; ++w)
            {
                unsigned int vertexId = aBone->mWeights[w].mVertexId;
                float weight = aBone->mWeights[w].mWeight;
                sub.vertices[vertexId].AddBoneData(boneIndex, weight);
            }
        }

        for (auto& v : sub.vertices)
            v.NormalizeWeights();



        for (auto& v : sub.vertices)
        {
            glm::vec3 pos(v.position[0], v.position[1], v.position[2]);
            glm::vec3 nrm(v.normal[0], v.normal[1], v.normal[2]);
            glm::vec3 tan(v.tangent[0], v.tangent[1], v.tangent[2]);
            glm::vec3 bit(v.bitangent[0], v.bitangent[1], v.bitangent[2]);

            pos = glm::vec3(correction * glm::vec4(pos, 1.0f));
            nrm = glm::normalize(glm::vec3(correction * glm::vec4(nrm, 0.0f)));
            tan = glm::normalize(glm::vec3(correction * glm::vec4(tan, 0.0f)));
            bit = glm::normalize(glm::vec3(correction * glm::vec4(bit, 0.0f)));

            v.position[0] = pos.x; v.position[1] = pos.y; v.position[2] = pos.z;
            v.normal[0] = nrm.x; v.normal[1] = nrm.y; v.normal[2] = nrm.z;
            v.tangent[0] = tan.x; v.tangent[1] = tan.y; v.tangent[2] = tan.z;
            v.bitangent[0] = bit.x; v.bitangent[1] = bit.y; v.bitangent[2] = bit.z;
        }

        sub.Setup();
        submeshes.push_back(std::move(sub));
    }

    // ─────────────────────────────
    // 3️⃣ Build hierarchy (parent indices only)
    // ─────────────────────────────
    std::function<void(aiNode*, int)> buildHierarchy;
    buildHierarchy = [&](aiNode* node, int parent)
        {
            std::string nodeName = node->mName.C_Str();
            auto it = boneNameToIndex.find(nodeName);
            int currentIndex = -1;

            if (it != boneNameToIndex.end())
            {
                currentIndex = it->second;
                bones[currentIndex].parentIndex = parent;
            }

            for (unsigned int i = 0; i < node->mNumChildren; ++i)
                buildHierarchy(node->mChildren[i], currentIndex);
        };
    buildHierarchy(scene->mRootNode, -1);

    for (auto& bone : bones)
    {
        bone.bindPose = correction * bone.bindPose;
        bone.inverseBindPose = glm::inverse(bone.bindPose);
    }

    // ─────────────────────────────
    // 4️⃣ Load animation clips
    // ─────────────────────────────
    for (unsigned int a = 0; a < scene->mNumAnimations; ++a)
    {
        aiAnimation* anim = scene->mAnimations[a];
        AnimationClip clip;
        clip.name = anim->mName.C_Str();
        clip.duration = (float)anim->mDuration;
        clip.ticksPerSecond = (float)(anim->mTicksPerSecond != 0 ? anim->mTicksPerSecond : 25.0f);

        for (unsigned int c = 0; c < anim->mNumChannels; ++c)
        {
            aiNodeAnim* channel = anim->mChannels[c];
            BoneAnimation boneAnim;
            boneAnim.boneName = channel->mNodeName.C_Str();

            unsigned int numKeys = std::max({
                channel->mNumPositionKeys,
                channel->mNumRotationKeys,
                channel->mNumScalingKeys });

            for (unsigned int k = 0; k < numKeys; ++k)
            {
                AnimationKey key{};
                key.time = 0.f;

                if (k < channel->mNumPositionKeys)
                {
                    key.translation = glm::make_vec3(&channel->mPositionKeys[k].mValue.x);
                    key.time = (float)channel->mPositionKeys[k].mTime / clip.ticksPerSecond;
                }
                if (k < channel->mNumRotationKeys)
                {
                    const aiQuatKey& q = channel->mRotationKeys[k];
                    key.rotation = glm::quat(q.mValue.w, q.mValue.x, q.mValue.y, q.mValue.z);
                    key.time = (float)q.mTime / clip.ticksPerSecond;
                }
                if (k < channel->mNumScalingKeys)
                {
                    key.scale = glm::make_vec3(&channel->mScalingKeys[k].mValue.x);
                    key.time = (float)channel->mScalingKeys[k].mTime / clip.ticksPerSecond;
                }

                boneAnim.keyframes.push_back(key);
            }

            clip.boneAnimations.push_back(std::move(boneAnim));
        }

        animations.push_back(std::move(clip));
    }

    //AG_CORE_INFO("[ASSET][SKINNED_MESH] Loaded Skinned Mesh: {} ({} bones, {} animations)", original_name, bones.size(), animations.size());
}



void SkinnedMeshAsset::Unload()
{
}

void SkinnedMeshAsset::WriteToBinary(std::string new_file) {
    std::ofstream out(new_file, std::ios::binary);
    if (!out.is_open()) return;

    // 1. Write uncompressed Header info first (Metadata)
    size_t nameLen = original_name.size();
    out.write((char*)&nameLen, sizeof(size_t));
    out.write(original_name.c_str(), nameLen);
    out.write((char*)&correction, sizeof(glm::mat4));

    // 2. Prepare a temporary buffer for all heavy data (Meshes, Bones, Anims)
    std::vector<unsigned char> rawData;
    auto WriteRaw = [&](const void* data, size_t size) {
        const unsigned char* p = static_cast<const unsigned char*>(data);
        rawData.insert(rawData.end(), p, p + size);
        };

    // --- Pack Submeshes ---
    size_t numSubmeshes = submeshes.size();
    WriteRaw(&numSubmeshes, sizeof(size_t));
    for (auto& sub : submeshes) {
        size_t sNameLen = sub.name.size();
        WriteRaw(&sNameLen, sizeof(size_t));
        WriteRaw(sub.name.c_str(), sNameLen);

        size_t numVerts = sub.vertices.size();
        WriteRaw(&numVerts, sizeof(size_t));
        WriteRaw(sub.vertices.data(), numVerts * sizeof(Vertex));

        size_t numIndices = sub.indices.size();
        WriteRaw(&numIndices, sizeof(size_t));
        WriteRaw(sub.indices.data(), numIndices * sizeof(unsigned int));
    }

    // --- Pack Bones ---
    size_t numBones = bones.size();
    WriteRaw(&numBones, sizeof(size_t));
    for (auto& bone : bones) {
        size_t bNameLen = bone.name.size();
        WriteRaw(&bNameLen, sizeof(size_t));
        WriteRaw(bone.name.c_str(), bNameLen);
        WriteRaw(&bone.parentIndex, sizeof(int));
        WriteRaw(&bone.bindPose, sizeof(glm::mat4));
        WriteRaw(&bone.inverseBindPose, sizeof(glm::mat4));
    }

    // --- Pack Animations ---
    size_t numAnims = animations.size();
    WriteRaw(&numAnims, sizeof(size_t));
    for (auto& clip : animations) {
        size_t cNameLen = clip.name.size();
        WriteRaw(&cNameLen, sizeof(size_t));
        WriteRaw(clip.name.c_str(), cNameLen);
        WriteRaw(&clip.duration, sizeof(float));
        WriteRaw(&clip.ticksPerSecond, sizeof(float));

        size_t numBoneAnims = clip.boneAnimations.size();
        WriteRaw(&numBoneAnims, sizeof(size_t));
        for (auto& ba : clip.boneAnimations) {
            size_t baNameLen = ba.boneName.size();
            WriteRaw(&baNameLen, sizeof(size_t));
            WriteRaw(ba.boneName.c_str(), baNameLen);
            size_t numKeys = ba.keyframes.size();
            WriteRaw(&numKeys, sizeof(size_t));
            WriteRaw(ba.keyframes.data(), numKeys * sizeof(AnimationKey));
        }
    }

    // 3. Compress and Write to Disk
    std::vector<unsigned char> compressed;
    if (CompressBuffer(rawData, compressed)) {
        size_t originalSize = rawData.size();
        size_t compressedSize = compressed.size();

        // Write sizes so the loader knows how much to allocate
        out.write((char*)&originalSize, sizeof(size_t));
        out.write((char*)&compressedSize, sizeof(size_t));
        out.write((char*)compressed.data(), compressedSize);
    }

    out.close();
}

void SkinnedMeshAsset::LoadFromBinary(std::filesystem::path path) {
    std::ifstream in(path, std::ios::binary);
    if (!in.is_open()) return;

    // 1. Read Metadata (Uncompressed)
    size_t nameLen;
    in.read((char*)&nameLen, sizeof(size_t));
    original_name.resize(nameLen);
    in.read(&original_name[0], nameLen);
    in.read((char*)&correction, sizeof(glm::mat4));

    // 2. Read Compression Info
    size_t originalSize, compressedSize;
    in.read((char*)&originalSize, sizeof(size_t));
    in.read((char*)&compressedSize, sizeof(size_t));

    std::vector<unsigned char> compressed(compressedSize);
    in.read((char*)compressed.data(), compressedSize);

    // 3. Decompress
    std::vector<unsigned char> decompressed;
    if (!DecompressBuffer(compressed, decompressed, originalSize)) return;

    // 4. Parse from decompressed memory buffer
    size_t offset = 0;
    auto ReadRaw = [&](void* dest, size_t size) {
        memcpy(dest, &decompressed[offset], size);
        offset += size;
        };

    // --- Parse Submeshes ---
    size_t numSubmeshes;
    ReadRaw(&numSubmeshes, sizeof(size_t));
    submeshes.resize(numSubmeshes);
    for (auto& sub : submeshes) {
        size_t sNameLen;
        ReadRaw(&sNameLen, sizeof(size_t));
        sub.name.resize(sNameLen);
        ReadRaw(&sub.name[0], sNameLen);

        size_t numVerts;
        ReadRaw(&numVerts, sizeof(size_t));
        sub.vertices.resize(numVerts);
        ReadRaw(sub.vertices.data(), numVerts * sizeof(Vertex));

        size_t numIndices;
        ReadRaw(&numIndices, sizeof(size_t));
        sub.indices.resize(numIndices);
        ReadRaw(sub.indices.data(), numIndices * sizeof(unsigned int));

        sub.Setup(); // GPU Upload
    }

    // --- Parse Bones ---
    size_t numBones;
    ReadRaw(&numBones, sizeof(size_t));
    bones.resize(numBones);
    boneNameToIndex.clear();
    for (int i = 0; i < (int)numBones; ++i) {
        size_t bNameLen;
        ReadRaw(&bNameLen, sizeof(size_t));
        bones[i].name.resize(bNameLen);
        ReadRaw(&bones[i].name[0], bNameLen);
        ReadRaw(&bones[i].parentIndex, sizeof(int));
        ReadRaw(&bones[i].bindPose, sizeof(glm::mat4));
        ReadRaw(&bones[i].inverseBindPose, sizeof(glm::mat4));
        boneNameToIndex[bones[i].name] = i;
    }

    // --- Parse Animations ---
    size_t numAnims;
    ReadRaw(&numAnims, sizeof(size_t));
    animations.resize(numAnims);
    for (auto& clip : animations) {
        size_t cNameLen;
        ReadRaw(&cNameLen, sizeof(size_t));
        clip.name.resize(cNameLen);
        ReadRaw(&clip.name[0], cNameLen);
        ReadRaw(&clip.duration, sizeof(float));
        ReadRaw(&clip.ticksPerSecond, sizeof(float));

        size_t numBoneAnims;
        ReadRaw(&numBoneAnims, sizeof(size_t));
        clip.boneAnimations.resize(numBoneAnims);
        for (auto& ba : clip.boneAnimations) {
            size_t baNameLen;
            ReadRaw(&baNameLen, sizeof(size_t));
            ba.boneName.resize(baNameLen);
            ReadRaw(&ba.boneName[0], baNameLen);
            size_t numKeys;
            ReadRaw(&numKeys, sizeof(size_t));
            ba.keyframes.resize(numKeys);
            ReadRaw(ba.keyframes.data(), numKeys * sizeof(AnimationKey));
        }
    }
    in.close();
}

AABB SubMeshSkinned::ComputeAABB(const std::vector<Vertex>& vertices)
{
    glm::vec3 min(1e30f);
    glm::vec3 max(-1e30f);

    for (auto& v : vertices)
    {
        glm::vec3 pos = glm::vec3(v.position[0], v.position[1], v.position[2]);
        min = glm::min(min, pos);
        max = glm::max(max, pos);
    }

    return AABB(min, max);
}