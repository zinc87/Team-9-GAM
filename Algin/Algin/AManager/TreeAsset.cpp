#include "pch.h"
#include "TreeAsset.h"

namespace AG {
	ASSET_TYPE TreeAsset::GetType() const
	{
		return ASSET_TYPE::BHT;
	}
	void TreeAsset::Load(std::filesystem::path file)
	{
		SERIALIZATIONSYSTEM.loadBHT(file, treeTemplate);
	}
	void TreeAsset::Unload()
	{
	}
}