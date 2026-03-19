#pragma once
#include "../Header/pch.h"

namespace AG {
	namespace Component {
		class PostProcessVolumeComponent : public IComponent {
		public:

			using Self = PostProcessVolumeComponent;

			// --- ADD THIS STATIC FUNCTION ---
			static Type GetStaticType() { return Data::ComponentTypes::PostProcessVolume; }

			PostProcessVolumeComponent() : m_ppvolume{OBB()},
				IComponent(Data::ComponentTypes::PostProcessVolume) {
			}

			PostProcessVolumeComponent(ID id, ID objId) :
				IComponent(Data::ComponentTypes::PostProcessVolume, id, objId) {
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

			void ToggleEffect() { enable_effect = !enable_effect; }

			std::string GetTypeName() const override { return "AG::Component::PostProcessVolumeComponent"; }

			REFLECT() {
				return {
					REFLECT_FIELD(effect_name, std::string, "effect_name"),
					REFLECT_FIELD(enable_effect, bool, "enable_effect"),
					REFLECT_FIELD(m_isGlobal, bool, "Is Global"),
					REFLECT_FIELD(m_ppvolume.center, glm::vec3, "Center"),
					REFLECT_FIELD(m_ppvolume.halfExtents, glm::vec3, "Half Extents")
				};
			}
			bool m_isGlobal = false;
			OBB m_ppvolume{};
			REFLECT_SERIALIZABLE(PostProcessVolumeComponent)

		private:
			std::string effect_name;
			bool enable_effect = true;
			/*bool m_isGlobal = false;
			OBB m_ppvolume{};*/
		};
	}
}
