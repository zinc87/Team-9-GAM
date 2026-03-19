#include "pch.h"
#include "MaterialTemplate.h"

void MaterialTemplate::SaveToJson(std::filesystem::path path)
{
	std::ofstream ofs(path);
	if (!ofs.is_open()) return;

	rapidjson::OStreamWrapper osw(ofs);
	rapidjson::PrettyWriter<rapidjson::OStreamWrapper> writer(osw);

	writer.StartObject();

	// Name
	writer.Key("material_name");
	writer.String(material_name.c_str());
	AG_CORE_INFO("Saving material: {}", material_name);

	writer.Key("shader_name");
	writer.String(shader_name.c_str());

	// IDs & Hashes
	writer.Key("reference_mat"); writer.Uint64(static_cast<uint64_t>(reference_mat));
	writer.Key("albedo_hash");   writer.Uint64(static_cast<uint64_t>(albedo_hash));
	writer.Key("normal_hash");   writer.Uint64(static_cast<uint64_t>(normal_hash));

	// Vector Data
	writer.Key("baseColor");
	writer.StartArray();
	writer.Double(baseColor.r);
	writer.Double(baseColor.g);
	writer.Double(baseColor.b);
	writer.Double(baseColor.a);
	writer.EndArray();

	// Float Properties
	writer.Key("metallic");  writer.Double(metallic);
	writer.Key("roughness"); writer.Double(roughness);
	writer.Key("uv_scale");  writer.Double(uv_scale);
	writer.Key("tint");      writer.Double(tint);

	// Overrides
	writer.Key("override_shader");    writer.Bool(override_shader);
	writer.Key("override_albedo");    writer.Bool(override_albedo);
	writer.Key("override_normal");    writer.Bool(override_normal);
	writer.Key("override_baseColor"); writer.Bool(override_baseColor);
	writer.Key("override_metallic");  writer.Bool(override_metallic);
	writer.Key("override_roughness"); writer.Bool(override_roughness);
	writer.Key("override_uv_scale");  writer.Bool(override_uv_scale);
	writer.Key("override_tint");      writer.Bool(override_tint);

	writer.EndObject();
}

void MaterialTemplate::LoadFromJson(std::filesystem::path path)
{
	std::ifstream ifs(path);
	if (!ifs.is_open()) return;

	rapidjson::IStreamWrapper isw(ifs);
	rapidjson::Document doc;
	doc.ParseStream(isw);

	if (doc.HasParseError()) return;

	// Name
	if (doc.HasMember("material_name") && doc["material_name"].IsString())
		material_name = doc["material_name"].GetString();

	if (doc.HasMember("shader_name") && doc["shader_name"].IsString())
		shader_name = doc["shader_name"].GetString();

	// IDs & Hashes
	if (doc.HasMember("reference_mat") && doc["reference_mat"].IsUint64())
		reference_mat = static_cast<size_t>(doc["reference_mat"].GetUint64());

	if (doc.HasMember("albedo_hash") && doc["albedo_hash"].IsUint64())
		albedo_hash = static_cast<size_t>(doc["albedo_hash"].GetUint64());

	if (doc.HasMember("normal_hash") && doc["normal_hash"].IsUint64())
		normal_hash = static_cast<size_t>(doc["normal_hash"].GetUint64());

	// Vector Data
	if (doc.HasMember("baseColor") && doc["baseColor"].IsArray())
	{
		const auto& arr = doc["baseColor"];
		if (arr.Size() >= 4)
		{
			baseColor.r = arr[0].GetFloat();
			baseColor.g = arr[1].GetFloat();
			baseColor.b = arr[2].GetFloat();
			baseColor.a = arr[3].GetFloat();
		}
	}

	// Float Properties
	if (doc.HasMember("metallic") && doc["metallic"].IsNumber())
		metallic = doc["metallic"].GetFloat();

	if (doc.HasMember("roughness") && doc["roughness"].IsNumber())
		roughness = doc["roughness"].GetFloat();

	if (doc.HasMember("uv_scale") && doc["uv_scale"].IsNumber())
		uv_scale = doc["uv_scale"].GetFloat();

	if (doc.HasMember("tint") && doc["tint"].IsNumber())
		tint = doc["tint"].GetFloat();

	// Overrides
	if (doc.HasMember("override_shader") && doc["override_shader"].IsBool())
		override_shader = doc["override_shader"].GetBool();

	if (doc.HasMember("override_albedo") && doc["override_albedo"].IsBool())
		override_albedo = doc["override_albedo"].GetBool();

	if (doc.HasMember("override_normal") && doc["override_normal"].IsBool())
		override_normal = doc["override_normal"].GetBool();

	if (doc.HasMember("override_baseColor") && doc["override_baseColor"].IsBool())
		override_baseColor = doc["override_baseColor"].GetBool();

	if (doc.HasMember("override_metallic") && doc["override_metallic"].IsBool())
		override_metallic = doc["override_metallic"].GetBool();

	if (doc.HasMember("override_roughness") && doc["override_roughness"].IsBool())
		override_roughness = doc["override_roughness"].GetBool();

	if (doc.HasMember("override_uv_scale") && doc["override_uv_scale"].IsBool())
		override_uv_scale = doc["override_uv_scale"].GetBool();

	if (doc.HasMember("override_tint") && doc["override_tint"].IsBool())
		override_tint = doc["override_tint"].GetBool();
}