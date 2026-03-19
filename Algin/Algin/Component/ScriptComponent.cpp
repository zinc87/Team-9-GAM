#include "ScriptComponent.h"
#include "pch.h"

namespace AG {
	namespace Component {
		ScriptComponent::ScriptComponent(ID id, ID objId) :
			IComponent(Data::ComponentTypes::Script, id, objId) {
		}

		void ScriptComponent::Awake() {
			if (scriptName == "") {
				AG_CORE_ERROR("Choose a valid Script first");
				return;
			}
			SCRIPTENGINE.createInstance(m_objId, "Script." + scriptName, getSerializeFieldData());
		}

		void ScriptComponent::Start() {
			SCRIPTENGINE.instanceOnStart(m_objId);
		}

		void ScriptComponent::Update() {
			SCRIPTENGINE.instanceOnUpdate(m_objId, Benchmarker::GetInstance().GetDeltaTime());
		}

		void ScriptComponent::LateUpdate() {
			SCRIPTENGINE.instanceOnLateUpdate(m_objId, Benchmarker::GetInstance().GetDeltaTime());
		}

		void ScriptComponent::Free() {
			SCRIPTENGINE.instanceOnFree(m_objId);
		}

		void ScriptComponent::Inspector() {
			RenderFieldsInspector(this, GetFields());

			if (scriptName != "" && (oldScriptName != scriptName)) {
				updateMonoFields();
				//needsFieldUpdate = false;
			}



			RenderFieldsInspector(this, getSerializeFields());
		}

		void ScriptComponent::AssignFrom(const std::shared_ptr<IComponent>& fromCmp) {
			ASSIGNFROM_FN_BODY(
				scriptName = fromCmpPtr->scriptName;
				oldScriptName = fromCmpPtr->oldScriptName;
				updateMonoFields();
				for (auto& [fieldName, fromPtr] : fromCmpPtr->fieldNameToData)
				{
					auto it = fieldNameToData.find(fieldName);
					if (it == fieldNameToData.end())
						continue;

					ScriptFieldType fieldType = SCRIPTENGINE.getFields("Script." + scriptName).at(fieldName).type;
					ScriptFieldTypeInfo typeInfo = SCRIPTTYPEREGISTRY.getInfo(fieldType);

					// Copy data using reflection-aware function
					typeInfo.copyFn(it->second.get(),fromPtr.get());
				}
			)
		}

		void ScriptComponent::PrintData() {

		}

		std::vector<const char*> ScriptComponent::getClassName() {
			std::vector<const char*> result{};
			SCRIPTENGINE.getClassName(result);
			return result;
		}
		void ScriptComponent::updateMonoFields()
		{
			serializeFields.clear();
			fieldNameToData.clear();
			std::unordered_map<std::string, ScriptField> fields = SCRIPTENGINE.getFields("Script." + scriptName);
			if (fields.empty()) return;
			for (auto& field : fields) {
				if (fieldNameToData.find(field.second.name) == fieldNameToData.end()) {
					fieldNameToData[field.second.name] = std::unique_ptr<void, std::function<void(void*)>>(SCRIPTTYPEREGISTRY.getInfo(field.second.type).createFn(), SCRIPTTYPEREGISTRY.getInfo(field.second.type).destroyfn);
					//Set the object as invalid on create
					if (field.second.type == ScriptFieldType::Object) {
						System::IObject* objPtr = static_cast<System::IObject*>(fieldNameToData[field.second.name].get());
						objPtr->SetID(Data::GUIDZero());
					}
				}
				serializeFields.emplace_back(field.second.name,
					SCRIPTTYPEREGISTRY.getInfo(field.second.type).cppType,
					[field](void* obj)->void* { return (static_cast<Self*>(obj)->fieldNameToData[field.second.name].get()); },
					"", nullptr, false, 0.f, 0.f, 0.f, false);
			}
			oldScriptName = scriptName;
		}



		bool ScriptComponent::Serialize(std::ostream& out) const {
			[[maybe_unused]] bool first = true;

			// --- Script Name (normal reflected field) ---
			out << "\"Script Name\": \"" << scriptName << "\"";

			// --- Dynamic Mono Fields ---
			for (const auto& f : serializeFields) {
				void* ptr = f.getPtr(const_cast<ScriptComponent*>(this));
				if (!ptr) continue;

				auto* serializer = AG::System::SerializationSystem::GetInstance().getRegistry().Get(f.type);
				if (!serializer) continue;

				// separator
				out << ",\n\"" << f.name << "\": ";

				serializer->write(out, ptr);
			}

			return out.good();
		}

		bool ScriptComponent::Deserialize(std::istream& in)
		{
			std::string json((std::istreambuf_iterator<char>(in)), {});
			auto find_value = [&](const std::string& key) -> std::string {
				std::string pattern = "\"" + key + "\"";
				size_t k = json.find(pattern);
				if (k == std::string::npos) return {};
				size_t colon = json.find(':', k + pattern.size());
				size_t vstart = json.find_first_not_of(" \t\r\n", colon + 1);
				if (vstart == std::string::npos) return {};
				char c = json[vstart];
				if (c == '"') { size_t end = json.find('"', vstart + 1); return json.substr(vstart + 1, end - vstart - 1); }
				size_t end = json.find_first_of(",}\r\n", vstart);
				return json.substr(vstart, end - vstart);
				};

			std::string val = find_value("Script Name");
			if (!val.empty()) scriptName = val;

			for (auto& f : serializeFields)
			{
				std::string v = find_value(f.name);
				if (v.empty()) continue;
				void* ptr = f.getPtr(this);
				if (!ptr) continue;
				if (auto* s = AG::System::SerializationSystem::GetInstance().getRegistry().Get(f.type))
				{
					std::istringstream fs(v);
					s->read(fs, ptr);
				}
			}
			return true;
		}

	}
}


REGISTER_REFLECTED_TYPE(AG::Component::ScriptComponent)
