#pragma once
#include "pch.h"



static const std::unordered_map<std::string, ASSET_TYPE> extensionMap = {
	{".png",  ASSET_TYPE::TEXTURE},
	{".jpg",  ASSET_TYPE::TEXTURE},
	{".jpeg", ASSET_TYPE::TEXTURE},
	{".bmp",  ASSET_TYPE::TEXTURE},
	{".tga",  ASSET_TYPE::DDS},

	{".agtex",  ASSET_TYPE::AGTEXTURE},
	{".agcubemap",  ASSET_TYPE::AGCUBEMAP},
	{".agfont",  ASSET_TYPE::AGFONT},
	{".ttf",  ASSET_TYPE::FONT},
	{".dds",  ASSET_TYPE::DDS},
	
	{".obj",  ASSET_TYPE::MODEL},
	{".fbx",  ASSET_TYPE::MODEL},
	{".gltf", ASSET_TYPE::MODEL},
	{".agstaticmesh", ASSET_TYPE::AGSTATICMESH},
	{".agskinnedmesh", ASSET_TYPE::AGSKINNEDMESH},
	
	{".wav",  ASSET_TYPE::AUDIO},
	{".mp3",  ASSET_TYPE::AUDIO},
	{".bank", ASSET_TYPE::AUDIO},
	
	{".cs",   ASSET_TYPE::SCRIPT},
	{".bht",   ASSET_TYPE::BHT},

	{".prefab", ASSET_TYPE::PREFAB},
	{".agmat", ASSET_TYPE::AGMATERIAL},

	{".json", ASSET_TYPE::SCENE},
	{".scene", ASSET_TYPE::SCENE},
};


class AssetManager : public AG::Pattern::ISingleton<AssetManager>
{
public:
	AssetManager()
	{
		FT_Init_FreeType(&FT);
	}
	/* --- Load from Asset Path --- */
	void LoadAssets(std::filesystem::path path);

	///* --- Load (1) Asset--- */
	//template <typename T>
	//void LoadAsset(std::filesystem::path path);

	///* --- Load (1) Asset Binary --- */
	//template <typename T>
	//void LoadAssetBinary(std::filesystem::path path);

	//void LoadAsset(std::filesystem::path path);

	/* --- Unload All Assets --- */
	void UnloadAssets();

	/* --- Unload (1) Asset --- */
	void UnloadAsset(size_t id);

	/* --- Retrieve the ASSET_TYPE --- */
	ASSET_TYPE	GetFileType(std::filesystem::path file_path);

	/* --- Retrieve reference to asset map of ASSET_TYPE --- */
	std::map<size_t, std::weak_ptr<IAsset>>& GetAssets(ASSET_TYPE type) { return m_assets_by_type[type]; }

	/* --- Retrieve the weak_ptr to existing IAsset --- */
	std::weak_ptr<IAsset> GetAsset(size_t id);
	std::weak_ptr<IAsset> UseAsset(size_t id);

	/* --- Check if Assets exist --- */
	bool AssetExist(size_t id);

	/* --- Load from Queue */
	void LoadQueue();

	void AddQueue(std::string path);

	void RegisterAsset(ASSET_TYPE type, size_t id, std::shared_ptr<IAsset> new_asset);

	size_t hasher(std::string name) {
		return std::hash<std::string>{}(name);
	}

	constexpr uint64_t fnv1a_64(const char* s) {
		uint64_t hash = 14695981039346656037ull;
		while (*s) {
			hash ^= static_cast<unsigned char>(*s++);
			hash *= 1099511628211ull;
		}
		return hash;
	}

	void CleanUsedMap() { asset_used_map.clear(); }
	FT_Library& GetFreetypeInit() { return FT; }
private:
	FT_Library FT;

	std::unordered_map<size_t, std::shared_ptr<IAsset>> m_assets;
	std::unordered_map<ASSET_TYPE, std::map<size_t, std::weak_ptr<IAsset>>> m_assets_by_type;

	/* --- UNDER MAINTAINENCE --- */
	std::queue<std::string> load_queue;
	std::unordered_map <size_t, bool> asset_used_map;
	std::unordered_map <size_t, std::filesystem::path> asset_location_map;
};

#define ASSET_MANAGER AssetManager::GetInstance()
#define HASH(name) AssetManager::GetInstance().hasher(name)
#define HASH2(name) AssetManager::GetInstance().fnv1a_64(name)

//template<typename T>
//inline void AssetManager::LoadAsset(std::filesystem::path path)
//{
//	auto shared_type = std::make_shared<T>();
//	shared_type->Load(path); 
//	m_assets[GetFileType(path)][path.filename().string()] = shared_type;
//}
//
//template<typename T>
//inline void AssetManager::LoadAssetBinary(std::filesystem::path path)
//{
//	auto shared_type = std::make_shared<T>();
//	shared_type->LoadFromBinary(path);
//	m_assets[GetFileType(path)][path.filename().string()] = shared_type;
//	m_assets_by_type[GetFileType(path)][hasher(path.filename().string())] = shared_type;
//}
