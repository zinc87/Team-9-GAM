#pragma once
#include "../Header/pch.h"

namespace AG {
	namespace Component {
		class SkyBoxComponent : public IComponent {
		public:
			using Self = SkyBoxComponent;

			// --- ADD THIS STATIC FUNCTION ---
			static Type GetStaticType() { return Data::ComponentTypes::SkyBox; }

			SkyBoxComponent() :
				IComponent(Data::ComponentTypes::SkyBox) {
			}

			SkyBoxComponent(ID id, ID objId) :
				IComponent(Data::ComponentTypes::SkyBox, id, objId) {
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

			std::string GetTypeName() const override { return "AG::Component::SkyBoxComponent"; }

			static std::vector<const char*> GetCMItems() {
				auto& names = CubemapNamesStorage();
				auto& hashes = CubemapHashesStorage();
				auto& cptrs = CubemapNamePtrs();

				names.clear();
				hashes.clear();
				cptrs.clear();

				// index 0 = "NO MESH" / empty
				names.push_back("NO MESH");
				hashes.push_back(0); // 0 means "no cubemap"

				// fill from asset manager
				for (auto& [hash, assetWeak] : ASSET_MANAGER.GetAssets(ASSET_TYPE::AGCUBEMAP)) {
					if (auto asset = assetWeak.lock()) {
						names.push_back(asset->asset_name);
						hashes.push_back(hash); // << store the hash that renderer actually uses
					}
				}

				// build cptrs for ImGui combo
				for (auto& s : names) {
					cptrs.push_back(s.c_str());
				}

				return cptrs;
			}

			REFLECT() {
				return {
					// UI: let user pick a cubemap by dropdown (cm_index)
					REFLECT_FIELD_LIST(cm_index, int, "Cube Map", "", Self::GetCMItems),

					// the actual saved value: hash of the cubemap to render
					REFLECT_FIELD_HIDDEN(cubemap_hash, size_t, "CubeMap Hash"),

					// exposure slider
					REFLECT_FIELD_RANGE(m_exposure, float, "Exposure", "", 0.0f, 10.0f, 0.01f),

					// tint, rotation
					REFLECT_FIELD(m_tint, glm::vec3, "Tint Color"),
					REFLECT_FIELD(m_rotation, glm::vec3, "Rotation")
				};
			}

			REFLECT_SERIALIZABLE(SkyBoxComponent);

			int cm_index = 0;
			float m_exposure = 1.0f;
			glm::vec3 m_tint{ 1.0f, 1.0f, 1.0f };
			glm::vec3 m_rotation{ 0.0f, 0.0f, 0.0f };

		private:
			static std::vector<std::string>& CubemapNamesStorage() {
				static std::vector<std::string> names;
				return names;
			}
			static std::vector<size_t>& CubemapHashesStorage() {
				static std::vector<size_t> hashes;
				return hashes;
			}
			static std::vector<const char*>& CubemapNamePtrs() {
				static std::vector<const char*> ptrs;
				return ptrs;
			}

			size_t cubemap_hash = 0;
			std::string cubemapName;
			/*int cm_index = 0; 
			float m_exposure = 1.0f;
			glm::vec3 m_tint{ 1.0f, 1.0f, 1.0f };
			glm::vec3 m_rotation{ 0.0f, 0.0f, 0.0f };*/

		};
	}
}
