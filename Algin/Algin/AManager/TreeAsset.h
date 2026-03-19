#pragma once
#include "pch.h"
#include "IAsset.h"
#include "../BHT/AIManager.h"

namespace AG {
	struct TreeAsset :public IAsset {
		ASSET_TYPE GetType() const;

		void Load(std::filesystem::path file) override;
		void Unload() override;
		void WriteToBinary(std::string /*new_file*/) override {}
		void LoadFromBinary([[maybe_unused]] std::filesystem::path path) override {};

		AI::BehaviorTreeTemplate treeTemplate;
	};
}