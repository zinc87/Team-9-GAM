#pragma once
#include "../Header/pch.h"

//#include "../Script/SerializeField.h"

namespace AG {
	enum class ScriptFieldType;

	namespace Component {
		class ScriptComponent : public IComponent {
		public:
			using Self = ScriptComponent;

			static Type GetStaticType() { return Data::ComponentTypes::Script; }

			ScriptComponent() :
				IComponent(Data::ComponentTypes::Script) {
			}

			ScriptComponent(ID id, ID objId);
			
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

			std::string GetTypeName() const override { return "AG::Component::ScriptComponent"; }

			static std::vector<const char*> getClassName();
		public:
			REFLECT() {
				return {
					REFLECT_FIELD_LIST(scriptName, std::string, "Script Name", "", getClassName),
				};
			}

			bool Serialize(std::ostream& out) const override;
			bool Deserialize(std::istream& in) override;

			std::string scriptName;
			std::string oldScriptName;
			const std::vector<FieldInfo> getSerializeFields() { return serializeFields; }
			void updateMonoFields();
		private:
			std::unordered_map<std::string, std::unique_ptr<void, std::function<void(void*)>>>& getSerializeFieldData() { return fieldNameToData; };
		private: 
			//bool needsFieldUpdate = false; //Used so that dont need update field everyframe
			std::vector<FieldInfo> serializeFields;
			std::unordered_map<std::string, std::unique_ptr<void, std::function<void(void*)>>> fieldNameToData;
		};
	}
}
