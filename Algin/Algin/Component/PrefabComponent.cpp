#include "PrefabComponent.h"
#include "pch.h"

#include "../Object Base/Prefab/PrefabEditModeController.h"

namespace AG {
	namespace Component {
		PrefabComponent::PrefabComponent(ID id, ID objId) :
			IComponent(Data::ComponentTypes::Prefab, id, objId) {
			sourcePrefab = Data::GUIDZero();
			sourcePrefabName = "";
			overrides.SetArray();
		}

		void PrefabComponent::setPrefab(System::PrefabID newID, std::string newName)
		{
			sourcePrefab = newID;
			sourcePrefabName = newName;
		}

		void PrefabComponent::setPrefabOverride(const System::jsonDoc& newOverride)
		{
			if (overrides == newOverride) return;
			overrides.CopyFrom(newOverride, overrides.GetAllocator());
		}

		System::jsonDoc& PrefabComponent::getPrefabOverride() {
			return overrides;
		}

		// Inherited via IComponent
		void PrefabComponent::Awake() {}

		void PrefabComponent::Start(){}

		void PrefabComponent::Update(){}

		void PrefabComponent::LateUpdate(){}

		void PrefabComponent::Free(){}

		void PrefabComponent::Inspector(){
			if (PrefabEditModeController::GetInstance().isActive()) {
				if (ImGui::Button("Apply to Current (Default)")) {
					PrefabEditModeController::GetInstance().exitEditMode();
				}
				ImGui::SameLine();
				if (ImGui::Button("Apply to All Instance")) {

				}
				ImGui::SameLine();
				if (ImGui::Button("Discard All Changes")) {

				}
			}
			
		}

		void PrefabComponent::AssignFrom([[maybe_unused]] const std::shared_ptr<IComponent>& fromCmp){}

		void PrefabComponent::PrintData(){}
	}
}

REGISTER_REFLECTED_TYPE(AG::Component::PrefabComponent)