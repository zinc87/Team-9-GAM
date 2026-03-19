#include "pch.h"
#include "PrefabAsset.h"

namespace AG {
	ASSET_TYPE PrefabAsset::GetType() const {
		return ASSET_TYPE::PREFAB;
	}

	void PrefabAsset::Load(std::filesystem::path file) {
		// Read the whole file and save to jsonDoc
		if (!std::filesystem::exists(file)) {
			AG_CORE_ERROR("Prefab file not found: {}", file.string());
		}

		std::ifstream ifs(file);
		if (!ifs.is_open()) {
			AG_CORE_ERROR("Failed to open prefab file: {}", file.string());
		}

		rapidjson::IStreamWrapper isw(ifs);
		rapidjson::Document doc;
		doc.ParseStream(isw);
		// Load the prefab manager with the doc
		PREFABMANAGER.loadPrefab(std::move(doc), file);

	}
	void PrefabAsset::Unload() {

	}
}