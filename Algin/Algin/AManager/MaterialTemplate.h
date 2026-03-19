#pragma once
#include "pch.h"
#include "AssetManager.h"

struct MaterialTemplate : public IAsset
{
	std::string material_name;
	std::string shader_name;

	size_t reference_mat = 0;
	size_t albedo_hash = 0;
	size_t normal_hash = 0;

	glm::vec4 baseColor = glm::vec4(1.f);
	float metallic = 0.f;
	float roughness = 0.f;
	float uv_scale = 1.f;
	float tint = 1.f;

	bool override_shader = false;

	bool override_albedo = false;
	bool override_normal = false;

	bool override_baseColor = false;
	bool override_metallic = false;
	bool override_roughness = false;
	bool override_uv_scale = false;
	bool override_tint = false;

	MaterialTemplate() = default;

	void Load(std::filesystem::path path) override { (void)path; }
	void Unload() override {}
	void WriteToBinary(std::string new_file) override { (void)new_file; }
	void LoadFromBinary(std::filesystem::path path) override { (void)path; }

	void LoadFromJson(std::filesystem::path path);
	void SaveToJson(std::filesystem::path path);

	void SetShaderName(const std::string& name)
	{
		shader_name = name;
	}

	// use this for getter, for future proof

	std::string& GetShaderName()
	{
		if (reference_mat == 0 || override_shader) // no refernce or overrided, use own
		{
			if (shader_name.empty())
			{
				shader_name = "Algin_3D";
			}

			return shader_name;
		}
		else // has reference
		{
			auto ref_mat = AssetManager::GetInstance().GetAsset(reference_mat);
			auto ref_mat_sp = std::dynamic_pointer_cast<MaterialTemplate>(ref_mat.lock());
			if (ref_mat_sp)
			{
				return ref_mat_sp->GetShaderName();
			}
			else
			{
				if (shader_name.empty())
				{
					shader_name = "Algin_3D";
				}
				return shader_name;
			}
		}
	}

	size_t& GetAlbedoHash() 
	{
		if (reference_mat == 0 || override_albedo) // no refernce or overrided, use own
		{
			return albedo_hash;
		}
		else // has reference
		{
			auto ref_mat = AssetManager::GetInstance().GetAsset(reference_mat);
			auto ref_mat_sp = std::dynamic_pointer_cast<MaterialTemplate>(ref_mat.lock());
			if (ref_mat_sp)
			{
				return ref_mat_sp->GetAlbedoHash();
			}
			else
			{
				return albedo_hash;  
			}
		}
	}
	size_t& GetNormalHash() 
	{ 
		if (reference_mat == 0 || override_normal) // no refernce or overrided, use own
		{
			return normal_hash;
		}
		else // has reference
		{
			auto ref_mat = AssetManager::GetInstance().GetAsset(reference_mat);
			auto ref_mat_sp = std::dynamic_pointer_cast<MaterialTemplate>(ref_mat.lock());
			if (ref_mat_sp)
			{
				return ref_mat_sp->GetNormalHash();
			}
			else
			{
				return normal_hash;
			}
		}
	}

	glm::vec4& GetBaseColor()
	{
		if (reference_mat == 0 || override_baseColor) // no refernce or overrided, use own
		{
			return baseColor;
		}
		else // has reference
		{
			auto ref_mat = AssetManager::GetInstance().GetAsset(reference_mat);
			auto ref_mat_sp = std::dynamic_pointer_cast<MaterialTemplate>(ref_mat.lock());
			if (ref_mat_sp)
			{
				return ref_mat_sp->GetBaseColor();
			}
			else
			{
				return baseColor;
			}
		}
	}

	float& GetMetallic() 
	{
		if (reference_mat == 0 || override_metallic) // no refernce or overrided, use own
		{
			return metallic;
		}
		else // has reference
		{
			auto ref_mat = AssetManager::GetInstance().GetAsset(reference_mat);
			auto ref_mat_sp = std::dynamic_pointer_cast<MaterialTemplate>(ref_mat.lock());
			if (ref_mat_sp)
			{
				return ref_mat_sp->GetMetallic();
			}
			else
			{
				return metallic;
			}
		}
	}

	float& GetRoughness()
	{
		if (reference_mat == 0 || override_roughness) // no refernce or overrided, use own
		{
			return roughness;
		}
		else // has reference
		{
			auto ref_mat = AssetManager::GetInstance().GetAsset(reference_mat);
			auto ref_mat_sp = std::dynamic_pointer_cast<MaterialTemplate>(ref_mat.lock());
			if (ref_mat_sp)
			{
				return ref_mat_sp->GetRoughness();
			}
			else
			{
				return roughness;
			}
		}
	}

	float& GetUV_scale()
	{
		if (reference_mat == 0 || override_uv_scale) // no refernce or overrided, use own
		{
			return uv_scale;
		}
		else // has reference
		{
			auto ref_mat = AssetManager::GetInstance().GetAsset(reference_mat);
			auto ref_mat_sp = std::dynamic_pointer_cast<MaterialTemplate>(ref_mat.lock());
			if (ref_mat_sp)
			{
				return ref_mat_sp->GetUV_scale();
			}
			else
			{
				return uv_scale;
			}
		}
	}
	float& GetTint()
	{
		if (reference_mat == 0 || override_tint) // no refernce or overrided, use own
		{
			return tint;
		}
		else // has reference
		{
			auto ref_mat = AssetManager::GetInstance().GetAsset(reference_mat);
			auto ref_mat_sp = std::dynamic_pointer_cast<MaterialTemplate>(ref_mat.lock());
			if (ref_mat_sp)
			{
				return ref_mat_sp->GetTint();
			}
			else
			{
				return tint;
			}
		}
	}

	ASSET_TYPE GetType() const override { return ASSET_TYPE::AGMATERIAL; }
};
