#pragma once
#include "../Header/pch.h"
#include "../Object Base/Prefab/PrefabManager.h"

namespace AG {
	namespace Component {
		class PrefabComponent : public IComponent {
		public:
			using Self = PrefabComponent;

			static Type GetStaticType() { return Data::ComponentTypes::Prefab; }

			PrefabComponent() :
				IComponent(Data::ComponentTypes::Prefab) {
				sourcePrefab = Data::GUIDZero();
				sourcePrefabName = "";
				overrides.SetArray();
			}

			PrefabComponent(ID id, ID objId);

			void setPrefab(System::PrefabID newID, std::string newName);
			void setPrefabOverride(const System::jsonDoc& newOverride);
			System::jsonDoc& getPrefabOverride();

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

			std::string GetTypeName() const override { return "AG::Component::PrefabComponent"; }

			static std::vector<const char*> getClassName();
		public:
			REFLECT() {
				return {
					REFLECT_FIELD_HIDDEN(sourcePrefab, System::PrefabID, "Prefab ID"),
					REFLECT_FIELD_HIDDEN(sourcePrefabName, std::string, "Prefab Name"),
					REFLECT_FIELD_HIDDEN(overrides, System::jsonDoc, "Overrides") //Dont need to be supported in the inspector
				};
			}
			REFLECT_SERIALIZABLE(PrefabComponent)
		private:
			System::PrefabID sourcePrefab;
			std::string sourcePrefabName; //Only for debug purposes
			System::jsonDoc overrides;
		};
	}
}