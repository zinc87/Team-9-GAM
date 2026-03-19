#include "pch.h"
#include "AudioBankAsset.h"

void AudioBankAsset::Load(std::filesystem::path path)
{
	FMOD::Studio::Bank* rawBank = nullptr;
	AUDIOSYSTEM.loadBankFile(path, rawBank);
	bank = rawBank;
}

void AudioBankAsset::Unload()
{
	if (bank) {
		bank->unloadSampleData();
		bank->unload();
		bank = nullptr;
	}
}
