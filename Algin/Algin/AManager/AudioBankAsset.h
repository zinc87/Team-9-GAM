#pragma once
#include "pch.h"

struct AudioBankAsset : public IAsset
{
	FMOD::Studio::Bank* bank = nullptr;

	void Load(std::filesystem::path path) override;
	void Unload() override;


	void WriteToBinary(std::string new_file) override { (void)new_file; }
	void LoadFromBinary(std::filesystem::path path) override { (void)path; }
	ASSET_TYPE GetType() const override { return ASSET_TYPE::AUDIO; }
};