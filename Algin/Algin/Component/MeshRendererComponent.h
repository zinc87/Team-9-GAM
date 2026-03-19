#pragma once
#include "../Header/pch.h"
#include "../AManager/StaticMeshAsset.h"
#include "../AManager/TextureAsset.h"
#include "../Graphics/MaterialAsset.h"

namespace AG {
	namespace Component {
		class MeshRendererComponent : public IComponent {
		public:
			using Self = MeshRendererComponent;

			// --- ADD THIS STATIC FUNCTION ---
			static Type GetStaticType() { return Data::ComponentTypes::MeshRenderer; }

			MeshRendererComponent() :
				IComponent(Data::ComponentTypes::MeshRenderer) 
			{
			}

			MeshRendererComponent(ID id, ID objId) :
				IComponent(Data::ComponentTypes::MeshRenderer, id, objId) 
			{
			}

			// Inherited via IComponent
			void Awake() override;

			void Start() override;

			void Update() override;

			void LateUpdate() override;

			void Free() override;

			void Inspector() override;

			//virtual std::shared_ptr<IComponent> Clone() override;
			void AssignFrom(const std::shared_ptr<IComponent>& fromCmp) override;

			void PrintData();

			void DrawShadows(const StaticMeshAsset& mesh, TransformComponent& trf);
			void DrawLight(const StaticMeshAsset& mesh, TransformComponent& trf);
			void EnsureMaterialDataValid();
			void DrawObjsColor(const StaticMeshAsset& mesh, TransformComponent& trf);
			std::string GetTypeName() const override { return "AG::Component::MeshRendererComponent"; }

			//
			void SetMeshRendererSubmeshMaterial(std::string submeshName, std::string materialName);
			void setMeshfromString(std::string str);
			void setMeshfromHash(size_t hash);

			static std::vector<const char*> GetMeshItems() {
				static std::vector<std::string> names;
				static std::vector<const char*> cptrs;

				// Only rebuild if asset count changes
				names.clear();
				cptrs.clear();

				names.push_back("NO MESH");

				auto& assets = ASSET_MANAGER.GetAssets(ASSET_TYPE::AGSTATICMESH);
				for (auto& [hash, asset] : assets) {
					if (auto lock = asset.lock())
						names.push_back(lock->asset_name);
				}

				// Reserve to avoid reallocating while pushing back
				cptrs.reserve(names.size());
				for (auto& s : names)
					cptrs.push_back(s.c_str());

				return cptrs;
			}


			REFLECT() {
				return {
					REFLECT_FIELD(model_hash, size_t, "Mesh Hash"),
					REFLECT_FIELD_LIST(mesh_name, std::string, "Mesh", "", Self::GetMeshItems),
					REFLECT_FIELD(submesh_name_hashes, std::vector<size_t>, "SubMesh Hashes"),
					REFLECT_FIELD(material_hash_list, std::vector<size_t>, "Material Hash List"),
					REFLECT_FIELD(material_value_list, std::vector<glm::vec3>, "Sub Meshes"),
					REFLECT_FIELD(shininess, float, "Shininess")
				};
			}

			REFLECT_SERIALIZABLE(MeshRendererComponent)

			std::string mesh_name;
			std::vector<size_t> submesh_name_hashes;
			size_t model_hash = 0;
			std::vector<size_t>		material_hash_list;
			std::vector<glm::vec3>	material_value_list; // used cased of no texture
			float shininess = 32.0f;
		};
	}
}
