#include "pch.h"
#include "AssetManager.h"




void AssetManager::LoadAssets(std::filesystem::path path)
{
	for (const auto& entry : std::filesystem::directory_iterator(path))
	{
		if (entry.is_directory())
		{
			LoadAssets(entry.path());
		}
		else /* --- Entry is an Asset --- */
		{
			size_t hashed_id = hasher(entry.path().filename().string());
			if (AssetExist(hashed_id))
			{
				continue; // skip loading if asset already exists
			}

			ASSET_TYPE entry_type = GetFileType(entry);
			std::string file_name = entry.path().filename().string();

			switch (entry_type)
			{
			case ASSET_TYPE::DDS:
			{
				auto shared_tex = std::make_shared<TextureAsset>();
				shared_tex->asset_name = file_name;

				// load mesh into shared_mesh
				shared_tex->LoadFromDDS(entry);

				asset_location_map[hashed_id] = entry.path().generic_string();
				// store shared_mesh into map
				m_assets[hashed_id] = shared_tex;
				m_assets_by_type[ASSET_TYPE::DDS][hashed_id] = shared_tex;

				//AG_CORE_INFO("AssetManager: Loaded DDS Asset: {} - {}", file_name, hashed_id);

				break;
			}
			case ASSET_TYPE::TEXTURE:
			{
				auto shared_tex = std::make_shared<TextureAsset>();
				shared_tex->asset_name = file_name;

				// load mesh into shared_mesh
				shared_tex->Load(entry);

				asset_location_map[hashed_id] = entry.path().generic_string();
				// store shared_mesh into map
				m_assets[hashed_id] = shared_tex;
				m_assets_by_type[ASSET_TYPE::TEXTURE][hashed_id] = shared_tex;

				//AG_CORE_INFO("AssetManager: Loaded GENeric Tex Asset: {} - {}", file_name, hashed_id);

				break;
			}
			case ASSET_TYPE::AGCUBEMAP:
			{
				auto shared_mesh = std::make_shared<CubeMapAsset>();
				shared_mesh->asset_name = file_name;

				// load mesh into shared_mesh
				shared_mesh->LoadFromBinary(entry);
				asset_location_map[hashed_id] = entry.path().generic_string();

				// store shared_mesh into map
				m_assets[hashed_id] = shared_mesh;
				m_assets_by_type[ASSET_TYPE::AGCUBEMAP][hashed_id] = shared_mesh;
				//AG_CORE_INFO("AssetManager: Loaded AGTexture Asset: {} - {}", file_name, hashed_id);
				break;
			}

			case ASSET_TYPE::AGSTATICMESH:
			{
				auto shared_mesh = std::make_shared<StaticMeshAsset>();
				shared_mesh->asset_name = file_name;

				// load mesh into shared_mesh
				shared_mesh->LoadFromBinary(entry);
				shared_mesh->ori_path = entry.path().generic_string();
				asset_location_map[hashed_id] = entry.path().generic_string();
				// store shared_mesh into map
				m_assets[hashed_id] = shared_mesh;
				m_assets_by_type[ASSET_TYPE::AGSTATICMESH][hashed_id] = shared_mesh;
				//AG_CORE_INFO("AssetManager: Loaded AGTexture Asset: {} - {}", file_name, hashed_id);
				break;
			}
			case ASSET_TYPE::AGSKINNEDMESH:
			{
				auto skinned_mesh = std::make_shared<SkinnedMeshAsset>();
				skinned_mesh->asset_name = file_name;

				// load mesh into skinned_mesh
				skinned_mesh->ori_path = entry.path().generic_string();
				skinned_mesh->LoadFromBinary(entry);

				asset_location_map[hashed_id] = entry.path().generic_string();
				// store skinned_mesh into map
				m_assets[hashed_id] = skinned_mesh;
				m_assets_by_type[ASSET_TYPE::AGSKINNEDMESH][hashed_id] = skinned_mesh;
				//AG_CORE_INFO("AssetManager: Loaded SkinnedMesh Asset: {} - {}", file_name, hashed_id);
				break;
			}
			case AUDIO:
			{
				for (const auto& audioEntry : std::filesystem::directory_iterator(path)) {
					std::string audioFileName = audioEntry.path().stem().string();
					if (audioFileName == "Master.strings") {
						auto shared_bank = std::make_shared<AudioBankAsset>();
						shared_bank->asset_name = file_name;
						shared_bank->Load(audioEntry);
					}
				}
				auto shared_bank = std::make_shared<AudioBankAsset>();
				shared_bank->asset_name = file_name;
				shared_bank->Load(entry);

				m_assets[hashed_id] = shared_bank;
				//AG_CORE_INFO("AssetManager: Loaded AGTexture Asset: {} - {}", file_name, hashed_id);
				break;
			}
			case AGFONT:
			{
				auto shared_font = std::make_shared<FontAsset>();
				shared_font->asset_name = file_name;
				shared_font->LoadFromBinary(entry);

				m_assets[hashed_id] = shared_font;
				m_assets_by_type[ASSET_TYPE::AGFONT][hashed_id] = shared_font;
				//AG_CORE_INFO("AssetManager: Loaded Font Asset: {} - {}", file_name, hashed_id);
				break;
			}
			case PREFAB: {
				std::filesystem::path full_path = entry.path();
				auto shared_Prefab = std::make_shared<AG::PrefabAsset>();
				shared_Prefab->asset_name = file_name;
				shared_Prefab->Load(full_path);

				m_assets[hashed_id] = shared_Prefab;
				m_assets_by_type[ASSET_TYPE::PREFAB][hashed_id] = shared_Prefab;
				//AG_CORE_INFO("AssetManager: Loaded Prefab Asset: {} - {}", file_name, hashed_id);
				break;
			}
			case BHT: {
				std::filesystem::path full_path = entry.path();
				auto shared_Tree = std::make_shared<AG::TreeAsset>();
				shared_Tree->asset_name = file_name;
				shared_Tree->Load(full_path);

				m_assets[hashed_id] = shared_Tree;
				m_assets_by_type[ASSET_TYPE::BHT][hashed_id] = shared_Tree;
				//AG_CORE_INFO("AssetManager: Loaded Prefab Asset: {} - {}", file_name, hashed_id);
				break;
			}
			case ASSET_TYPE::AGMATERIAL:
			{
				auto shared_mat = std::make_shared<MaterialTemplate>();
				shared_mat->asset_name = file_name;

				// load mesh into shared_mesh
				shared_mat->LoadFromJson(entry);

				asset_location_map[hashed_id] = entry.path().generic_string();
				m_assets[hashed_id] = shared_mat;
				m_assets_by_type[ASSET_TYPE::AGMATERIAL][hashed_id] = shared_mat;

				//AG_CORE_INFO("AssetManager: Loaded Material Asset: {} - {}", file_name, hashed_id);
				break;
			}
			//case MODEL:
			//{
			//	auto shared_mesh = std::make_shared<StaticMeshAsset>();
			//	shared_mesh->asset_name = file_name;

			//	// load mesh into shared_mesh
			//	shared_mesh->Load(entry);
			//	asset_location_map[hashed_id] = entry.path().generic_string();

			//	// store shared_mesh into map
			//	m_assets[hashed_id] = shared_mesh;
			//	m_assets_by_type[ASSET_TYPE::AGSTATICMESH][hashed_id] = shared_mesh;
			//	AG_CORE_INFO("AssetManager: Loaded Model Asset: {} - {}", file_name, hashed_id);
			//	break;
			//}
			default:
				break;
			}

		}
	}
}

void AssetManager::UnloadAssets()
{
	for (auto& tex : GetAssets(TEXTURE))
	{
		if (tex.second.lock())
		{
			auto tex_shared = std::dynamic_pointer_cast<TextureAsset>(tex.second.lock());
			glDeleteTextures(1, &tex_shared->textureID);
		}
	}

	m_assets.clear();
}

void AssetManager::UnloadAsset(size_t id)
{
	auto it = m_assets.find(id);
	if (it != m_assets.end())
	{
		m_assets.erase(it);
	}
}

ASSET_TYPE AssetManager::GetFileType(std::filesystem::path file_path)
{
	// extension is the filetype, example :
	// path("durian.png").extension -> ".png"
	if (!file_path.has_extension())
		return ASSET_TYPE::UNKNOWN;

	std::string ext = file_path.extension().string();

	std::transform(ext.begin(), ext.end(), ext.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });

	auto it = extensionMap.find(ext);
	if (it != extensionMap.end())
	{
		return it->second;
	}

	return ASSET_TYPE::UNKNOWN;
}

std::weak_ptr<IAsset> AssetManager::GetAsset(size_t id)
{
	auto it = m_assets.find(id);
	if (it != m_assets.end())
	{
		return m_assets[id];
	}

	return std::shared_ptr<IAsset>();
}

std::weak_ptr<IAsset> AssetManager::UseAsset(size_t id)
{
	auto it = m_assets.find(id);
	if (it != m_assets.end())
	{
		asset_used_map[id] = true;
		return m_assets[id];
	}

	return std::shared_ptr<IAsset>();
}

bool AssetManager::AssetExist(size_t id)
{
	return m_assets.find(id) != m_assets.end();
}

void AssetManager::LoadQueue()
{
	if (!load_queue.empty())
	{
		auto path = load_queue.front();
		load_queue.pop();
		//LoadAsset(path);
	}
}

void AssetManager::AddQueue(std::string path)
{
	load_queue.push(path);
}

void AssetManager::RegisterAsset(ASSET_TYPE type, size_t id, std::shared_ptr<IAsset> new_asset)
{
	m_assets[id] = new_asset;
	m_assets_by_type[type][id] = new_asset;
}




