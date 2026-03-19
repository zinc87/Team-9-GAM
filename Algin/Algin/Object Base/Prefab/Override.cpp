#include "pch.h"
#include "Override.h"

namespace AG {
	namespace System {
		void PrefabOverrideSerializer::CreatePrefabOverride(const jsonDoc& beforeState, const jsonDoc& afterState, jsonDoc& existingOverride, jsonDoc& finalResult)
		{
			jsonDoc diffResult;
			diffResult.SetArray();

			//Produce the diff purely base on before and after state
			if (beforeState.IsObject() && afterState.IsObject()) {
				diffObject(beforeState, afterState, "", Data::ComponentTypes::None, diffResult);
			}
			else {
				// Top-level change: replace entire doc
				pushOpObject("Edit", "", &afterState, Data::ComponentTypes::None, diffResult);
			}

			//Need to compare with the overrideResult for the clean up
			finalResult.SetArray();
			mergeAndCleanUp(existingOverride, diffResult, finalResult);
		}
		void PrefabOverrideSerializer::applyOverride(System::IObject::ID objID, const rapidjson::Value& overrides)
		{
			std::unordered_map<Data::ComponentTypes, std::vector<System::jsonDoc>> grouped;

			//1️ Group overrides by component type
			for (rapidjson::SizeType i{}; i < overrides.Size(); ++i) {
				const std::vector<std::string>& pathVector = SplitPath(overrides[i]["path"].GetString());
				const ComponentManager::WCompVec& compVec = COMPONENTMANAGER.GetComponentsObj().at(objID);
				for (auto& comp : compVec) {
					//!!!!!!!Have to be super careful when changing format of json. This part rely on that part of the path to be type name in string
					if (comp.expired()) {
						AG_CORE_WARN("Override: Invalid comp");
						continue;
					}
					if (pathVector.size() < 3) {
						AG_CORE_WARN("Prefab Override: Invalid path format '{}'", overrides[i]["path"].GetString());
						continue;
					}
					if (comp.lock()->GetTypeName() == pathVector[2]) { 
						System::jsonDoc copyOverride;
						copyOverride.CopyFrom(overrides[i], copyOverride.GetAllocator());
						grouped[comp.lock()->GetType()].emplace_back(std::move(copyOverride));
					}
				}
				
			}

			//2️ Apply per-component
			for (auto& [compType, compOverrides] : grouped) {
				//For script, apply script name first then reload then apply field
				if (compType != Data::ComponentTypes::Script) {
					normalApplyOverride(objID, compType, compOverrides);
				}
				else {
					scriptApplyOverride(objID, compOverrides);
				}

			}

			//3 Set back new override
			//Set back the override
			const ComponentManager::WCompVec& compVec = COMPONENTMANAGER.GetComponentsObj().at(objID);
			for (auto& comp : compVec) {
				if (comp.lock()->GetType() == Data::ComponentTypes::Prefab) {
					//Component::PrefabComponent* ptr = reinterpret_cast<Component::PrefabComponent*>(comp.lock().get());

				}
			}
		}
		void PrefabOverrideSerializer::pushOpObject(const char* opName, const std::string& path, const rapidjson::Value* valuePtr, Data::ComponentTypes, jsonDoc& resultArray)
		{
			rapidjson::Value opObject{ rapidjson::kObjectType };
			auto& allocator = resultArray.GetAllocator();

			opObject.AddMember("op", rapidjson::Value(opName, allocator), allocator);
			opObject.AddMember("path", rapidjson::Value(path.c_str(), static_cast<rapidjson::SizeType>(path.size()), allocator), allocator);
			if (valuePtr) {
				rapidjson::Value copy(*valuePtr, allocator);
				opObject.AddMember("Value", std::move(copy), allocator);
			}
			//opObject.AddMember("Component Type")

			resultArray.PushBack(std::move(opObject), allocator);
		}
		void PrefabOverrideSerializer::diffObject(const rapidjson::Value& beforeState, const rapidjson::Value& afterState, const std::string& path, Data::ComponentTypes compTypes, jsonDoc& resultArray)
		{
			for (auto it = afterState.MemberBegin(); it != afterState.MemberEnd(); ++it) {
				std::string newPath = path + "/" + it->name.GetString();
				const rapidjson::Value& afterCmpValue = it->value;

				Data::ComponentTypes curCompType{ compTypes };
				if (curCompType == Data::ComponentTypes::None && afterCmpValue.HasMember("ComponentType") && afterCmpValue["ComponentType"].IsInt()) {
					curCompType = static_cast<Data::ComponentTypes>(afterCmpValue["ComponentType"].GetInt());
				}


				//Check for add(after have but before dont have)
				if (!beforeState.HasMember(it->name)) {
					pushOpObject("Add", newPath, &it->value, curCompType, resultArray);
				}
				else {
					//Check for edit
					const rapidjson::Value& beforeCmpValue = beforeState[it->name];

					if (Equal(afterCmpValue, beforeCmpValue)) continue;
					else if (beforeCmpValue.IsObject() && afterCmpValue.IsObject()) {
						diffObject(beforeCmpValue, afterCmpValue, newPath, curCompType, resultArray);
					}
					else {
						pushOpObject("Edit", newPath, &afterCmpValue, curCompType, resultArray);
					}
						
				}
			}

			//Check for remove (after dont have but before have
			for (auto it = beforeState.MemberBegin(); it != beforeState.MemberEnd(); ++it) {
				if (!afterState.HasMember(it->name)) {
					std::string newPath = path + "/" + it->name.GetString();
					const rapidjson::Value& beforeCmpValue = it->value;
					Data::ComponentTypes curCompType{ compTypes };
					if (beforeCmpValue.HasMember("ComponentType") && beforeCmpValue["ComponentType"].IsInt()) {
						curCompType = static_cast<Data::ComponentTypes>(beforeCmpValue["ComponentType"].GetInt());
					}
					pushOpObject("Remove", newPath, nullptr, curCompType, resultArray);
				}
			}
		}
		void PrefabOverrideSerializer::mergeAndCleanUp(const jsonDoc& existingOverrides, const jsonDoc& diffResult, jsonDoc& result)
		{
			//if add then remove -> skip the add from the existing, skip the remove from diff
			//if edit then remove -> check for all the path and its child in edit, skip remove path and its child in existing, push remove
			//if add then edit -> find the parent path of the edit in the existing add, edit the data
			//if remove then add -> skip the remove from the existing, change to edit of the individual field if any (check againest the serialize field)
			//rest just take the diffResult

			result.SetArray();
			if (!existingOverrides.IsArray() || !diffResult.IsArray()) return;
			auto& allocator = result.GetAllocator();

			// Maps for quick lookup
			std::unordered_map<std::string, const rapidjson::Value*> existingMap;
			std::unordered_map<std::string, PrefabOverride> workingAddMap;
			std::unordered_set<std::string> consumedPaths; // For removes we skip later

			// Build maps from existing overrides
			for (auto& ov : existingOverrides.GetArray()) {
				std::string path = ov["path"].GetString();
				std::string opStr = ov["op"].GetString();
				existingMap[path] = &ov;

				if (opStr == "Add") {
					PrefabOverride po;
					po.op = PrefabOverride::OpType::Add;
					po.pathToField = path;
					po.value.CopyFrom(ov["Value"], allocator);
					workingAddMap[path] = std::move(po);
				}
			}

			// Process diff overrides
			for (auto& diffOv : diffResult.GetArray()) {
				std::string path = diffOv["path"].GetString();
				std::string opStr = diffOv["op"].GetString();
				const rapidjson::Value* valuePtr = diffOv.HasMember("Value") ? &diffOv["Value"] : nullptr;

				// --- Case 1: Add → Remove ---
				if (opStr == "Remove") {
					auto itAdd = workingAddMap.find(path);
					if (itAdd != workingAddMap.end()) {
						// cancel out both
						workingAddMap.erase(itAdd);
						consumedPaths.insert(path);
						continue;
					}

					// --- Case 2: Edit → Remove ---
					// remove overrides all edits of the same path or children
					for (auto it = existingMap.begin(); it != existingMap.end();) {
						const std::string& existingPath = it->first;
						if (existingPath == path || existingPath.rfind(path + "/", 0) == 0) {
							// if existing is edit or add under this path, skip it
							if (strcmp((*it->second)["op"].GetString(), "Edit") == 0) {
								consumedPaths.insert(existingPath);
							}
							it = existingMap.erase(it);
						}
						else {
							++it;
						}
					}

					// push remove itself
					//pushOpObject("Remove", path, nullptr, result); !!To be fixed
					continue;
				}

				// --- Case 3: Add → Edit ---
				if (opStr == "Edit") {
					bool merged = false;
					for (auto& [addPath, po] : workingAddMap) {
						if (path.rfind(addPath, 0) == 0) { // edit belongs to an added object
							std::string relativePath = path.substr(addPath.size());
							if (!relativePath.empty() && relativePath[0] == '/') relativePath.erase(0, 1);

							rapidjson::Value* target = GetJsonValueByPath(po.value, relativePath);
							if (target) {
								target->CopyFrom(*valuePtr, allocator);
								merged = true;
								break;
							}
						}
					}

					if (merged) continue;

					// --- Case 4: Remove → Add (handled as Edit) ---
					auto itExisting = existingMap.find(path);
					if (itExisting != existingMap.end() &&
						strcmp((*itExisting->second)["op"].GetString(), "Remove") == 0) {
						consumedPaths.insert(path);
						//pushOpObject("Edit", path, valuePtr, result);!!To be fixed
						continue;
					}

					// --- Case 5: Normal Edit ---
					//pushOpObject("Edit", path, valuePtr, result);!!To be fixed
					continue;
				}

				// --- Case 4: Remove → Add (explicit Add form) ---
				if (opStr == "Add") {
					auto itExisting = existingMap.find(path);
					if (itExisting != existingMap.end() && strcmp((*itExisting->second)["op"].GetString(), "Remove") == 0){
						consumedPaths.insert(path);
						//pushOpObject("Edit", path, valuePtr, result);!!To be fixed
						continue;
					}

					// Normal Add
					//pushOpObject("Add", path, valuePtr, result);!!To be fixed
					continue;
				}
			}

			// Push remaining Add operations (not cancelled)
			for (auto& [path, po] : workingAddMap) {
				//pushOpObject("Add", path, &po.value, result);!!To be fixed
			}

			// Push remaining edits/removes from existing that weren’t consumed
			for (auto& ov : existingOverrides.GetArray()) {
				std::string path = ov["path"].GetString();
				std::string opStr = ov["op"].GetString();

				if (consumedPaths.find(path) != consumedPaths.end()) continue;
				if (opStr == "Add" && workingAddMap.count(path)) continue; // handled already

				//const rapidjson::Value* valuePtr = ov.HasMember("Value") ? &ov["Value"] : nullptr;
				//pushOpObject(opStr.c_str(), path, valuePtr, result);!!To be fixed
			}
		}
		bool PrefabOverrideSerializer::Equal(const rapidjson::Value& a, const rapidjson::Value& b)
		{
			// Both are numbers
			if (a.IsDouble() && b.IsDouble()) {
				return std::abs(a.GetDouble() - b.GetDouble()) < 1e-5;
			}
			else if (a.IsFloat() && b.IsFloat()) {
				return std::abs(a.GetFloat() - b.GetFloat()) < 1e-5;
			}
			// Both are arrays
			else if (a.IsArray() && b.IsArray() && a.Size() == b.Size()) {
				for (rapidjson::SizeType i = 0; i < a.Size(); ++i) {
					if (!Equal(a[i], b[i])) return false;
				}
				return true;
			}
			// Both are objects
			else if (a.IsObject() && b.IsObject()) {
				if (a.MemberCount() != b.MemberCount()) return false;

				for (auto it = a.MemberBegin(); it != a.MemberEnd(); ++it) {
					const char* name = it->name.GetString();
					if (!b.HasMember(name)) return false;
					if (!Equal(it->value, b[name])) return false;
				}
				return true;
			}
			else {
				return a == b;
			}
		}
		std::vector<std::string> PrefabOverrideSerializer::SplitPath(const std::string& path)
		{
			std::vector<std::string> tokens;
			size_t start = 0;
			while (start < path.size()) {
				size_t end = path.find('/', start);
				if (end == std::string::npos) end = path.size();
				if (end > start) {
					tokens.push_back(path.substr(start, end - start));
				}
				start = end + 1;
			}
			return tokens;
		}
		rapidjson::Value* PrefabOverrideSerializer::GetJsonValueByPath(rapidjson::Value& root, const std::string& path)
		{
			auto tokens = SplitPath(path);
			rapidjson::Value* current = &root;
			for (auto& token : tokens) {
				if (token.empty()) continue; // skip empty tokens (leading '/')
				if (!current->IsObject()) return nullptr;
				if (!current->HasMember(token.c_str())) return nullptr;
				current = &((*current)[token.c_str()]);
			}
			return current;
		}
		void PrefabOverrideSerializer::normalApplyOverride(const System::IObject::ID& objID, Data::ComponentTypes compType, std::vector<System::jsonDoc>& compOverrides)
		{
			for (rapidjson::Value& compOverride : compOverrides) {
				if (!compOverride.HasMember("op") || !compOverride["op"].IsString()) {
					AG_CORE_WARN("Override: override no op");
					continue;
				}
				const std::string& operation = (compOverride)["op"].GetString();

				if (operation == "Add") {
					ComponentManager::CompPtr newComp = COMPONENTMANAGER.CreateUserOwnedComponent(compType);
					if (!newComp) {
						AG_CORE_WARN("Apply Override: Failed to create component during prefab load: {}", (int)compType);
						continue;
					}
					AG::TypeInfo* info = AG::TypeRegistry::Instance().GetType(newComp->GetTypeName());
					if (!info) {
						AG_CORE_WARN("Apply Override: Unknown component type during prefab load: {}", newComp->GetTypeName());
						continue;
					}
					if (!compOverride.HasMember("Value") || !compOverride["Value"].IsObject()) {
						AG_CORE_WARN("Override: override no Value");
						continue;
					}
					const auto fields = info->getFieldsFunc();
					for (auto& field : fields) {
						if (!compOverride["Value"].HasMember(field.name.c_str())) continue;

						void* ptr = field.getPtr(newComp.get());
						if (ptr && SERIALIZATIONSYSTEM.getRegistry().GetJsonReg(field.type)) {
							SERIALIZATIONSYSTEM.getRegistry().GetJsonReg(field.type)->read(compOverride["Value"], ptr, field.name);
						}
						else {
							AG_CORE_WARN("Cannot deserialize field {} in component {}", field.name, newComp->GetTypeName());
						}
					}
				}
				else if (operation == "Remove") {
					const ComponentManager::WCompVec& objMap = COMPONENTMANAGER.GetComponentsObj().at(objID);
					for (const auto& comp : objMap) {
						if (comp.lock()->GetType() == compType) {
							COMPONENTMANAGER.RemoveComponent(comp.lock()->GetID());
						}
					}
				}
				else if (operation == "Edit") {
					//Check againest the current data, if the same remove the edit field(same as the prefab template and no override)
					const ComponentManager::WCompVec& objMap = COMPONENTMANAGER.GetComponentsObj().at(objID);
					ComponentManager::CompPtr targetComp = nullptr;
					for (const auto& comp : objMap) {
						if (comp.lock()->GetType() == compType) {
							targetComp = comp.lock();
							break;
						}
					}

					if (!targetComp) {
						AG_CORE_WARN("Apply Override: No component found for Edit override of type {}", (int)compType);
						continue;
					}

					// Check against the prefab template or current component data
					AG::TypeInfo* info = AG::TypeRegistry::Instance().GetType(targetComp->GetTypeName());
					if (!info) {
						AG_CORE_WARN("Apply Override: Unknown component type {}", targetComp->GetTypeName());
						continue;
					}

					if (!compOverride.HasMember("Value") || !compOverride["Value"].IsObject()) {
						AG_CORE_WARN("Override Edit: no Value object for component {}", targetComp->GetTypeName());
						continue;
					}

					const auto& fields = info->getFieldsFunc();
					for (auto& field : fields) {
						if (!compOverride["Value"].HasMember(field.name.c_str())) continue;

						void* fieldPtr = field.getPtr(targetComp.get());
						if (!fieldPtr) continue;

						// Read value from JSON into temporary storage to compare
						const rapidjson::Value& jsonField = compOverride["Value"][field.name.c_str()];
						if (SERIALIZATIONSYSTEM.getRegistry().GetJsonReg(field.type)) {
							// Compare with current component field value
							rapidjson::Document tempDoc;
							rapidjson::Value tempValue;
							tempValue.SetObject();
							SERIALIZATIONSYSTEM.getRegistry().GetJsonReg(field.type)->write(tempDoc.GetAllocator(), tempValue, fieldPtr, field.name);
							if (jsonField == tempValue) {
								// Field is identical, remove from override
								compOverride["Value"].RemoveMember(field.name.c_str());
								continue;
							}

							// Apply the field override
							SERIALIZATIONSYSTEM.getRegistry().GetJsonReg(field.type)->read(jsonField, fieldPtr, field.name);
						}
						else {
							AG_CORE_WARN("Cannot deserialize field {} in component {}", field.name, targetComp->GetTypeName());
						}
					}

					// If after filtering, the Value object is empty, the override is redundant
					if (compOverride["Value"].ObjectEmpty()) {
						// Skip this override entirely
						continue;
					}
				}
			}
		}
		void PrefabOverrideSerializer::scriptApplyOverride(const System::IObject::ID& , const std::vector<System::jsonDoc>& )
		{

		}
	}
}