#include "pch.h"
#include "MaterialAsset.h"

namespace fs = std::filesystem;
MaterialAsset MaterialAsset::save_mat_ass{};


//change default material here
matAsset::matAsset() : params{ {1.f,1.f,1.f,1.f},{0.f,0.f,1.f,1.f} },
albedoHashID{ 0 }, normalMapHashID{ 0 } {}

void MaterialSetup::init() {
	
	std::string folder = "Assets/Materials";

	try {
		for (const auto& entry : fs::directory_iterator(folder)) {
			MaterialAsset matAsset;
			parseMaterial(entry.path().string(), matAsset);
			materialPaths.push_back(entry.path().string());
			materialAssets.push_back(matAsset);
		}
	}
	catch (const fs::filesystem_error& e) {
		std::cerr << "Filesystem error: " << e.what() << "\n";
	}

}


bool MaterialSetup::parseMaterial(const std::string& filePath, MaterialAsset& outMat) {
	
	std::ifstream ifs(filePath);
	if (!ifs.is_open()) {
		AG_CORE_ERROR("file path error");
		return false;
	}

	rapidjson::IStreamWrapper isw(ifs);

	rapidjson::Document doc;
	doc.ParseStream(isw);

	if (doc.HasParseError()) {
		AG_CORE_ERROR("doc.HasParseError()");
		return false;
	}
		

	if (!doc.IsObject()) {
		AG_CORE_ERROR("!doc.IsObject()");
		return false;
	}

	if (!doc.HasMember("baseColor") || !doc["baseColor"].IsArray())
		return false;

	//read name
	if (!doc.HasMember("name") || !doc["name"].IsString())
		return false;
	outMat.name = static_cast<std::string>(doc["name"].GetString());

	//read baseColor
	const rapidjson::Value& baseColorVal = doc["baseColor"];
	if (baseColorVal.Size() != 4)
		return false;

	outMat.baseColor.r = static_cast<float>(baseColorVal[0].GetFloat());
	outMat.baseColor.g = static_cast<float>(baseColorVal[1].GetFloat());
	outMat.baseColor.b = static_cast<float>(baseColorVal[2].GetFloat());
	outMat.baseColor.a = static_cast<float>(baseColorVal[3].GetFloat());


	// read metallic
	if (!doc.HasMember("metallic") || !doc["metallic"].IsNumber())
		return false;
	outMat.metallic = static_cast<float>(doc["metallic"].GetFloat());

	// read smoothness
	if (!doc.HasMember("roughness") || !doc["roughness"].IsNumber())
		return false;
	outMat.roughness = static_cast<float>(doc["roughness"].GetFloat());

	// read uvScale
	if (!doc.HasMember("uvScale") || !doc["uvScale"].IsNumber())
		return false;
	outMat.UvScale = static_cast<float>(doc["uvScale"].GetFloat());

	//read tint
	if (!doc.HasMember("tint") || !doc["tint"].IsNumber())
		return false;
	outMat.tint = static_cast<float>(doc["tint"].GetFloat());

	//read albedo hash ID
	if (!doc.HasMember("albedoHASHid") || !doc["albedoHASHid"].IsNumber())
		return false;
	outMat.albedoHASHid = static_cast<size_t>(doc["albedoHASHid"].GetUint64());

	//read normal map hash ID
	if (!doc.HasMember("normalMapHashID") || !doc["normalMapHashID"].IsNumber())
		return false;
	outMat.normalMapHashID = static_cast<size_t>(doc["normalMapHashID"].GetUint64());
	return true;
}

bool MaterialSetup::saveMatToJS(const MaterialAsset& mat, std::string& filePath) {

	rapidjson::Document doc;
	doc.SetObject();
	rapidjson::Document::AllocatorType& alloc = doc.GetAllocator();

	//add name
	doc.AddMember("name", rapidjson::Value(mat.name.c_str(), alloc), alloc);

	// "baseColor": [r, g, b, a]
	rapidjson::Value baseColor(rapidjson::kArrayType);
	baseColor.PushBack(mat.baseColor.r, alloc);
	baseColor.PushBack(mat.baseColor.g, alloc);
	baseColor.PushBack(mat.baseColor.b, alloc);
	baseColor.PushBack(mat.baseColor.a, alloc);

	doc.AddMember("baseColor", baseColor, alloc);
	doc.AddMember("metallic", mat.metallic, alloc);
	doc.AddMember("roughness", mat.roughness, alloc);
	doc.AddMember("uvScale", mat.UvScale, alloc);
	doc.AddMember("tint", mat.tint, alloc);
	doc.AddMember("albedoHASHid", mat.albedoHASHid, alloc);
	doc.AddMember("normalMapHashID", mat.normalMapHashID, alloc);

	fs::path folder = "Assets/Materials";
	fs::path path = folder / (mat.name + ".mat");
	filePath = path.string();

	std::ofstream ofs(filePath);
	if (!ofs.is_open()) {
		// handle error
		return false;
	}
	rapidjson::OStreamWrapper osw(ofs);
	rapidjson::PrettyWriter<rapidjson::OStreamWrapper> writer(osw);
	doc.Accept(writer);

	return true;
}