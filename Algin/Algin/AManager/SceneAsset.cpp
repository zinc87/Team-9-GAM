#include "pch.h"
#include "SceneAsset.h"

namespace AG {
	ASSET_TYPE SceneAsset::GetType() const {
		return ASSET_TYPE::SCENE;
	}

	void SceneAsset::Load(std::filesystem::path file) {
		// Read the whole file and save to jsonDoc
		if (!std::filesystem::exists(file)) {
			AG_CORE_ERROR("Prefab file not found: {}", file.string());
		}

		std::ifstream ifs(file);
		if (!ifs.is_open()) {
			AG_CORE_ERROR("Failed to open prefab file: {}", file.string());
		}

		rapidjson::IStreamWrapper isw(ifs);
		sceneDoc.ParseStream(isw);

	}
	void SceneAsset::Unload() {

	}
}