#include "pch.h"
#include "SerializationSystem.h"
#include "../Object Base/ObjectManager.h"
#include "../Object Base/ComponentManager.h"
#include "../Data/Reflection.h"
#include <fstream>
#include <iostream>
#include <sstream>

namespace AG {
    namespace System {
        // ------------------------------------------------------------
        // Helpers
        // ------------------------------------------------------------
        std::string SerializationSystem::EscapeString(const std::string& s)
        {
            std::string out;
            for (char c : s)
            {
                switch (c)
                {
                case '\"': out += "\\\""; break;
                case '\\': out += "\\\\"; break;
                case '\n': out += "\\n"; break;
                case '\t': out += "\\t"; break;
                default: out += c; break;
                }
            }
            return out;
        }

        std::string SerializationSystem::Indent(int level)
        {
            return std::string(level * 4, ' ');
        }

        // ------------------------------------------------------------
        // Component serialization via reflection
        // ------------------------------------------------------------
        void SerializationSystem::WriteComponent(std::ostream& out, Component::IComponent* comp, int indent)
        {
            if (!comp) return;

            out << Indent(indent) << "{\n";
            out << Indent(indent + 1) << "\"Type\": " << static_cast<int>(comp->GetType()) << ",\n";
            out << Indent(indent + 1) << "\"ID\": \"" << comp->GetID() << "\"";

            // Reflective field serialization
            AG::TypeInfo* info = AG::TypeRegistry::Instance().GetType(comp->GetTypeName());
            if (info)
            {
                const auto fields = info->getFieldsFunc();
                for (size_t i = 0; i < fields.size(); ++i)
                {
                    const auto& f = fields[i];
                    void* ptr = f.getPtr(comp);
                    if (!ptr) continue;

                    out << ",\n" << Indent(indent + 1) << "\"" << f.name << "\": ";

                    if (auto* serializer = registry.Get(f.type))
                    {
                        serializer->write(out, ptr);
                    }
                    else
                    {
                        std::cerr << "[WriteComponent] No serializer for field: " << f.name << "\n";
                        out << "\"<unsupported>\"";
                    }
                }
            }

            if (comp->GetType() == ComponentManager::CType::Script)
            {
                auto* scriptComp = static_cast<Component::ScriptComponent*>(comp);
                const auto& dynFields = scriptComp->getSerializeFields();

                for (const auto& f : dynFields)
                {
                    void* ptr = f.getPtr(scriptComp);
                    if (!ptr) continue;

                    out << ",\n" << Indent(indent + 1) << "\"" << f.name << "\": ";

                    if (auto* serializer = registry.Get(f.type))
                        serializer->write(out, ptr);
                    else
                        out << "\"<unsupported>\"";
                }
            }

            out << "\n" << Indent(indent) << "}";
        }

        // ------------------------------------------------------------
        // Object serialization (includes components)
        // ------------------------------------------------------------
        void SerializationSystem::WriteObject(std::ostream& out,
            std::shared_ptr<AG::System::IObject> obj,
            ComponentManager& compMgr,
            int indent)
        {
            if (!obj) return;

            out << Indent(indent) << "{\n";
            out << Indent(indent + 1) << "\"ID\": \"" << obj->GetID() << "\",\n";
            out << Indent(indent + 1) << "\"Name\": \"" << EscapeString(obj->GetName()) << "\",\n";

            Data::GUID parentID = obj->GetParent() ? obj->GetParent()->GetID() : Data::GUIDZero();
            out << Indent(indent + 1) << "\"Parent\": \"" << parentID << "\"";

            // Serialize components
            auto compsIt = compMgr.GetComponentsObj().find(obj->GetID());
            if (compsIt != compMgr.GetComponentsObj().end())
            {
                out << ",\n" << Indent(indent + 1) << "\"Components\": [\n";

                size_t count = 0;
                auto& comps = compsIt->second;
                for (auto& wk : comps)
                {
                    if (auto sp = wk.lock())
                    {
                        WriteComponent(out, sp.get(), indent + 2);
                        if (++count < comps.size()) out << ",";
                        out << "\n";
                    }
                }
                out << Indent(1) << "]\n";
                out << "}\n";
            }
        }

        // ------------------------------------------------------------
        // Save Scene
        // ------------------------------------------------------------
        bool SerializationSystem::SaveScene(const std::string& filename,
            ObjectManager& objMgr,
            ComponentManager& compMgr)
        {
            std::ofstream out(filename);
            if (!out.is_open())
            {
                std::cerr << "Cannot open " << filename << " for writing.\n";
                return false;
            }

            out << "{\n";
            out << Indent(1) << "\"Objects\": [\n";

            const auto& objs = objMgr.GetAllObjects();
           /* size_t idx = 0;*/
            size_t count = 0;
            for (auto& [id, obj] : objs)
            {
                if (!obj) continue;
                if (obj->GetName() == "__root__" || Data::IsZero(obj->GetID())) continue;

                WriteObject(out, obj, compMgr, 2);
                if (++count < objs.size()) out << ",";
                out << "\n";
            }


            out << Indent(1) << "]\n";
            out << "}\n";

            std::cout << "[Serialization] Scene saved to " << filename << "\n";
            return true;
        }

        // ------------------------------------------------------------
        // Load Scene
        // ------------------------------------------------------------
        bool SerializationSystem::LoadScene(const std::string& filename,
            ObjectManager& objMgr,
            ComponentManager& compMgr)
        {
            std::ifstream in(filename);
            if (!in.is_open())
            {
				AG_CORE_ERROR("Cannot open \"{}\" for reading.", filename);
                return false;
            }

            std::string jsonText((std::istreambuf_iterator<char>(in)),
                std::istreambuf_iterator<char>());
            in.close();

            objMgr.ClearScene();
            compMgr.ClearAllComponents();

            auto trim = [](std::string s)
                {
                    s.erase(std::remove_if(s.begin(), s.end(),
                        [](unsigned char c) { return std::isspace(c); }),
                        s.end());
                    return s;
                };

            auto find_matching = [&](const std::string& s, size_t openPos, char openCh, char closeCh) -> size_t
                {
                    int depth = 0;
                    for (size_t i = openPos; i < s.size(); ++i)
                    {
                        if (s[i] == openCh) ++depth;
                        else if (s[i] == closeCh)
                        {
                            --depth;
                            if (depth == 0) return i;
                        }
                    }
                    return std::string::npos;
                };

            auto find_value_span = [&](const std::string& block, const std::string& key) -> std::pair<size_t, size_t>
                {
                    std::string needle = "\"" + key + "\"";
                    size_t k = block.find(needle);
                    if (k == std::string::npos) return { std::string::npos, std::string::npos };
                    size_t colon = block.find(':', k + needle.size());
                    if (colon == std::string::npos) return { std::string::npos, std::string::npos };

                    size_t vstart = block.find_first_not_of(" \t\r\n", colon + 1);
                    if (vstart == std::string::npos) return { std::string::npos, std::string::npos };

                    char c = block[vstart];
                    if (c == '[') return { vstart, find_matching(block, vstart, '[', ']') + 1 };
                    if (c == '{') return { vstart, find_matching(block, vstart, '{', '}') + 1 };
                    if (c == '"')
                    {
                        size_t q2 = block.find('"', vstart + 1);
                        while (q2 != std::string::npos && block[q2 - 1] == '\\')
                            q2 = block.find('"', q2 + 1);
                        return { vstart, q2 + 1 };
                    }

                    size_t vend = block.find_first_of(",}]\n\r", vstart);
                    return { vstart, (vend == std::string::npos) ? block.size() : vend };
                };

            auto get_string = [&](const std::string& block, const std::string& key) -> std::string
                {
                    auto [s, e] = find_value_span(block, key);
                    if (s == std::string::npos) return {};
                    if (block[s] == '"') return block.substr(s + 1, (e - s - 2));
                    return block.substr(s, e - s);
                };

            auto get_uint = [&](const std::string& block, const std::string& key, uint32_t def = 0) -> uint32_t
                {
                    auto [s, e] = find_value_span(block, key);
                    if (s == std::string::npos) return def;
                    auto t = trim(block.substr(s, e - s));
                    if (t.empty()) return def;
                    try { return static_cast<uint32_t>(std::stoul(t)); }
                    catch (...) { return def; }
                };

            // find object array
            size_t objectsKey = jsonText.find("\"Objects\"");
            if (objectsKey == std::string::npos)
            {
                std::cerr << "Invalid scene JSON: missing \"Objects\".\n";
                return false;
            }

            size_t arrStart = jsonText.find('[', objectsKey);
            size_t arrEnd = find_matching(jsonText, arrStart, '[', ']');
            if (arrStart == std::string::npos || arrEnd == std::string::npos) return false;

            std::string objectsArray = jsonText.substr(arrStart + 1, arrEnd - arrStart - 1);
            std::vector<std::string> objectChunks;
            for (size_t p = 0;;)
            {
                size_t ob = objectsArray.find('{', p);
                if (ob == std::string::npos) break;
                size_t oe = find_matching(objectsArray, ob, '{', '}');
                if (oe == std::string::npos) break;
                objectChunks.emplace_back(objectsArray.substr(ob, oe - ob + 1));
                p = oe + 1;
            }

            std::unordered_map<Data::GUID, Data::GUID> parentOf;

            // Create objects first
            for (const auto& objChunk : objectChunks)
            {
                Data::GUID id = get_string(objChunk, "ID");
                std::string name = get_string(objChunk, "Name");
                Data::GUID parent = get_string(objChunk, "Parent");

                auto obj = objMgr.CreateObjectWithID(name.empty() ? ("Object_" + id) : name, id, true, true);
                parentOf[id] = parent;
            }

            // Components
            for (const auto& objChunk : objectChunks)
            {
                Data::GUID id = get_string(objChunk, "ID");
                auto obj = objMgr.GetObjectByID(id);
                if (!obj) continue;

                auto [compS, compE] = find_value_span(objChunk, "Components");
                if (compS == std::string::npos) continue;
                std::string compArray = objChunk.substr(compS + 1, compE - compS - 2);

                size_t p = 0;
                while (true)
                {
                    size_t cb = compArray.find('{', p);
                    if (cb == std::string::npos) break;
                    size_t ce = find_matching(compArray, cb, '{', '}');
                    if (ce == std::string::npos) break;

                    std::string compChunk = compArray.substr(cb, ce - cb + 1);
                    uint32_t typeVal = get_uint(compChunk, "Type", 0u);
                    auto ctype = static_cast<ComponentManager::CType>(typeVal);
                    auto comp = compMgr.CreateComponent(obj.get(), ctype);
                    if (!comp)
                    {
                        p = ce + 1;
                        continue;
                    }

                    // ============================================================
                    // SPECIAL HANDLING FOR SCRIPTCOMPONENT
                    // ============================================================
                    if (ctype == ComponentManager::CType::Script)
                    {
                        std::cout << "[DEBUG] Processing ScriptComponent\n";
                        auto* scriptComp = static_cast<Component::ScriptComponent*>(comp);

                        // First, deserialize the script name from reflected fields
                        AG::TypeInfo* info = AG::TypeRegistry::Instance().GetType(comp->GetTypeName());
                        if (info)
                        {
                            std::cout << "[DEBUG] Found TypeInfo for ScriptComponent\n";
                            const auto fields = info->getFieldsFunc();
                            std::cout << "[DEBUG] Static fields count: " << fields.size() << "\n";

                            for (const auto& f : fields)
                            {
                                auto [vs, ve] = find_value_span(compChunk, f.name);
                                if (vs == std::string::npos) {
                                    std::cout << "[DEBUG] Field '" << f.name << "' not found in JSON\n";
                                    continue;
                                }

                                std::string fieldData = compChunk.substr(vs, ve - vs);
                                std::cout << "[DEBUG] Deserializing static field '" << f.name << "'\n";

                                std::istringstream fieldStream(fieldData);
                                void* ptr = f.getPtr(comp);
                                if (!ptr) {
                                    std::cout << "[DEBUG] Failed to get pointer for field '" << f.name << "'\n";
                                    continue;
                                }

                                if (auto* serializer = registry.Get(f.type))
                                    serializer->read(fieldStream, ptr);
                            }
                        }
                        else
                        {
                            std::cout << "[ERROR] No TypeInfo found for ScriptComponent!\n";
                        }

                        // Now update the mono fields based on the loaded script name
                        std::cout << "[DEBUG] Calling updateMonoFields()\n";
                        scriptComp->updateMonoFields();
                        scriptComp->oldScriptName = scriptComp->scriptName;

                        // Deserialize the dynamic script fields
                        const auto& dynFields = scriptComp->getSerializeFields();
                        std::cout << "[DEBUG] Dynamic fields count: " << dynFields.size() << "\n";

                        for (const auto& f : dynFields)
                        {
                            std::cout << "[DEBUG] Processing dynamic field: " << f.name << "\n";

                            auto [vs, ve] = find_value_span(compChunk, f.name);
                            if (vs == std::string::npos) {
                                std::cout << "[DEBUG] Dynamic field '" << f.name << "' not found in JSON\n";
                                continue;
                            }

                            std::string fieldData = compChunk.substr(vs, ve - vs);
                            std::cout << "[DEBUG] Raw field data: '" << fieldData << "'\n";

                            // Trim whitespace
                            fieldData.erase(0, fieldData.find_first_not_of(" \t\r\n"));
                            fieldData.erase(fieldData.find_last_not_of(" \t\r\n,") + 1);

                            std::cout << "[DEBUG] After trim: '" << fieldData << "'\n";

                            // Remove quotes if it's a string value
                            if (!fieldData.empty() && fieldData.front() == '"' && fieldData.back() == '"')
                            {
                                fieldData = fieldData.substr(1, fieldData.size() - 2);
                                std::cout << "[DEBUG] After quote removal: '" << fieldData << "'\n";
                            }

                            std::istringstream fieldStream(fieldData);
                            void* ptr = f.getPtr(scriptComp);
                            if (!ptr) {
                                std::cout << "[ERROR] Failed to get pointer for dynamic field '" << f.name << "'\n";
                                continue;
                            }

                            if (auto* serializer = registry.Get(f.type))
                            {
                                std::cout << "[DEBUG] Found serializer, reading data...\n";
                                serializer->read(fieldStream, ptr);
                                std::cout << "[SUCCESS] Deserialized field '" << f.name << "'\n";
                            }
                            else
                            {
                                std::cout << "[ERROR] No serializer found for dynamic field: " << f.name << "\n";
                            }
                        }
                    }
                    // ============================================================
                    // STANDARD COMPONENT HANDLING
                    // ============================================================
                    else
                    {
                        AG::TypeInfo* info = AG::TypeRegistry::Instance().GetType(comp->GetTypeName());
                        if (info)
                        {
                            const auto& fields = info->getFieldsFunc();
                            for (const auto& f : fields)
                            {
                                // Find the field span in JSON text
                                auto [vs, ve] = find_value_span(compChunk, f.name);
                                if (vs == std::string::npos) continue;

                                std::string fieldData = compChunk.substr(vs, ve - vs);
                                std::istringstream fieldStream(fieldData);
                                void* ptr = f.getPtr(comp);
                                if (!ptr) continue;

                                if (auto* serializer = registry.Get(f.type))
                                    serializer->read(fieldStream, ptr);
                                else
                                    std::cerr << "[Deserialize] No serializer for " << f.name << "\n";
                            }
                        }

                        p = ce + 1;
                    }
                }
            }

            // Build hierarchy
            auto root = objMgr.GetRoot();
            if (!root) {
                root = objMgr.CreateObject("__root__", true, true);
                root->SetID(Data::GUIDZero());
            }

            for (const auto& [childID, parentID] : parentOf)
            {
                if (Data::IsZero(childID)) continue;
                auto child = objMgr.GetObjectByID(childID);
                auto parent = (Data::IsZero(parentID)) ? root : objMgr.GetObjectByID(parentID);
                if (child && parent && child != parent)
                    objMgr.LinkParentChild(parent, child);
            }

            std::cout << "[Serialization] Scene loaded from " << filename << "\n";
            SystemTest::SceneManager::setSceneName(filename);

            return true;
        }

        bool SerializationSystem::SaveSceneJson(std::filesystem::path fullPath) {
            rapidjson::Document document;
            rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator>& allocator = document.GetAllocator();
            document.SetObject();

            rapidjson::Value objList{ rapidjson::kArrayType };

            const auto& objs = OBJECTMANAGER.GetAllObjects();
            for (auto& obj : objs) {
                if (obj.second->GetID() == Data::GUIDZero()) continue;
                rapidjson::Value object{ rapidjson::kObjectType };
                if (obj.second->hasComponent(Data::ComponentTypes::Prefab)) {
                    serializePrefabInstance(allocator, object, obj.second);
                }
                else {
                    serializeObject(allocator, object, obj.second);
                }
                objList.PushBack(std::move(object), allocator);
            }
            document.AddMember("Object List", objList, allocator);

            std::ofstream ofs(fullPath);
            rapidjson::OStreamWrapper osw(ofs);
            rapidjson::PrettyWriter<rapidjson::OStreamWrapper> writer(osw);
            document.Accept(writer);

            return true;
        }

        bool SerializationSystem::loadSceneJson(std::filesystem::path filePath)
        {

            OBJECTMANAGER.ClearScene();
            COMPONENTMANAGER.ClearAllComponents();
            //Check File
            if (!std::filesystem::exists(filePath)) {
                AG_CORE_WARN("Prefab file not found: {}", filePath.string());
                return false;
            }
            std::ifstream ifs(filePath);
            if (!ifs.is_open()) {
                AG_CORE_WARN("Failed to open prefab file: {}", filePath.string());
                return false;
            }
            rapidjson::IStreamWrapper isw(ifs);
            rapidjson::Document sceneDoc;
            sceneDoc.ParseStream(isw);
            if (sceneDoc.HasParseError()) {
                AG_CORE_WARN("Failed to deserialize prefab doc");
                return false;
            }

            //Check Root object list
            if (!sceneDoc.HasMember("Object List") || !sceneDoc["Object List"].IsArray()) {
                AG_CORE_WARN("Cant deserialize Root object {}", filePath.string());
                return false;
            }
            rapidjson::Value& objList = sceneDoc["Object List"];

            //itertate the whole object List
            /*std::unordered_map<Data::GUID, Data::GUID> parentOf;
            for (rapidjson::SizeType i{}; i < objList.Size(); ++i) {
                rapidjson::Value& curObject = objList[i];

                if (!curObject.HasMember("Parent ID") || !curObject["Parent ID"].IsString() ||
                    !curObject.HasMember("ID") || !curObject["ID"].IsString()) {
                    AG_CORE_WARN("Cannot deserialize object basic and parent info");
                    continue;
                }
                parentOf[(curObject["ID"].GetString())] = curObject["Parent ID"].GetString();
                
                if (curObject.HasMember("Prefab ID")) {
                    deserializePrefabInstance(curObject);
                }
                else {
                    deserializeObject(curObject);
                }
            }*/

            std::unordered_map<Data::GUID, Data::GUID> parentOf;
            for (rapidjson::SizeType i{}; i < objList.Size(); ++i) {
                rapidjson::Value& curObject = objList[i];

                if (!curObject.HasMember("Parent ID") || !curObject["Parent ID"].IsString() ||
                    !curObject.HasMember("ID") || !curObject["ID"].IsString()) {
                    AG_CORE_WARN("Cannot deserialize object basic and parent info");
                    continue;
                }
                parentOf[(curObject["ID"].GetString())] = curObject["Parent ID"].GetString();

                System::IObject::ID objID{};
                deserializeObjBasicInfo(curObject, objID);
            }

            for (rapidjson::SizeType i{}; i < objList.Size(); ++i) {
                rapidjson::Value& curObject = objList[i];

                if (!curObject.HasMember("ID") || !curObject["ID"].IsString()) {
                    continue;
                }

                System::IObject::ID objID = curObject["ID"].GetString();
                deserializeRootCmp(curObject, objID);
            }

            deserializeHirachy(parentOf);
            SystemTest::SceneManager::setSceneName(filePath.string());
            return true;
        }

        bool SerializationSystem::serializeObject(rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator>& allocator, rapidjson::Value& objInst, const IObject::IObjectWPtr& obj) {
            serializeObjBasicInfo(allocator, objInst, obj);
            const ComponentManager::WCompVec& compVec = COMPONENTMANAGER.GetComponentsObj().at(obj.lock()->GetID());
            serializeRootCmp(allocator, objInst, compVec); 

            return true;
        }

        bool SerializationSystem::deserializeObject(const rapidjson::Value& objInst)
        {
            System::IObject::ID objID{};
            deserializeObjBasicInfo(objInst, objID); //Created object in object manager here

            deserializeRootCmp(objInst, objID);

            return true;
        }

        bool SerializationSystem::serializePrefabInstance(rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator>& allocator, rapidjson::Value& prefabInstObj, const IObject::IObjectWPtr& obj)
        {
            //I alreday know that obj have prefab component(is prefab instance)
            //Get the prefab Id from the preab component
            serializeObjBasicInfo(allocator, prefabInstObj, obj);

            //Serialize Prefab component
            const std::weak_ptr<Component::PrefabComponent>& prefabComp = obj.lock()->GetComponent<Component::PrefabComponent>();
            AG::TypeInfo* info = AG::TypeRegistry::Instance().GetType(prefabComp.lock()->GetTypeName());
            if (!info)
            {
                AG_CORE_WARN("Unknown component type name: {}", prefabComp.lock()->GetTypeName());
                return false;
            }
            else {
                const auto fields = info->getFieldsFunc();
                for (auto& field : fields) {
                    void* ptr = field.getPtr(prefabComp.lock().get());
                    if (ptr && registry.GetJsonReg(field.type)) {
                        registry.GetJsonReg(field.type)->write(allocator, prefabInstObj, ptr, field.name);
                    }
                    else if (field.type == std::type_index(typeid(jsonDoc))) {
                        const jsonDoc& overrideDoc = *static_cast<const jsonDoc*>(ptr);

                        rapidjson::Value overrideCopy(rapidjson::kObjectType);
                        overrideCopy.CopyFrom(overrideDoc, allocator);

                        rapidjson::Value key(field.name.c_str(), (rapidjson::SizeType)field.name.size(), allocator);
                        prefabInstObj.AddMember(key, overrideCopy, allocator);
                    }
                    else{
                        AG_CORE_WARN("Cant serialize prefab component field: {}", field.name);
                    }
                }
            }

            return false;
        }

        bool SerializationSystem::deserializePrefabInstance(const rapidjson::Value& prefabIns)
        {
            System::IObject::ID objID{};
            deserializeObjBasicInfo(prefabIns, objID); //Created object in object manager here

            deserializePrefabInstanceComponent(prefabIns, objID);

            return true;
        }

        bool SerializationSystem::serializePrefab(System::jsonDoc& prefabDoc, PrefabID guid, const IObject::IObjectWPtr& obj)
        {   
            rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator>& allocator = prefabDoc.GetAllocator();
            rapidjson::Value rapidjsonPrefab(rapidjson::kObjectType);

            //Prerfab Basic Info
            serializePrefabBasicInfo(allocator, rapidjsonPrefab, guid, obj.lock()->GetName());
            const ComponentManager::WCompVec& compVec = COMPONENTMANAGER.GetComponentsObj().at(obj.lock()->GetID());
            serializeRootCmp(allocator, rapidjsonPrefab, compVec);

            prefabDoc.AddMember("Prefab", rapidjsonPrefab, allocator);

            return true;
        }
        bool SerializationSystem::deserializePrefabBasicInfo(const System::jsonDoc& prefabDoc, PrefabTemplate& resultPrefab)
        {
            if (prefabDoc.HasParseError()) {
                AG_CORE_WARN("Failed to deserialize prefab doc");
                return false;
            }

            // Root object check
            if (!prefabDoc.HasMember("Prefab") || !prefabDoc["Prefab"].IsObject()) {
                AG_CORE_WARN("Invalid prefab format: missing 'Prefab' object");
                return false;
            }

            const auto& prefabObj = prefabDoc["Prefab"];

            //Basic info
            registry.GetJsonReg(typeid(std::string))->read(prefabObj, &resultPrefab.m_name, "Prefab Name");
            registry.GetJsonReg(typeid(System::IObject::ID))->read(prefabObj, &resultPrefab.m_id, "Id");


            //AG_CORE_INFO("Loaded Prefab: {} ({} components)", resultPrefab.m_name, resultPrefab.baseComponentData.size());
            return true;
        }
        bool SerializationSystem::deserializePrefabRootCmp(const System::jsonDoc& prefabDoc, ComponentManager::CompVec& comps) {
            if (!prefabDoc.HasMember("Prefab") || !prefabDoc["Prefab"].IsObject())
                return false;
            const rapidjson::Value& prefabObj = prefabDoc["Prefab"];
            
            if (!prefabObj.HasMember("Components") || !prefabObj["Components"].IsObject())
                return false;

            const rapidjson::Value& compList = prefabObj["Components"];

            for (auto it = compList.MemberBegin(); it != compList.MemberEnd(); ++it)
            {
                const std::string compName = it->name.GetString();
                const rapidjson::Value& compJson = it->value;

                if (!compJson.HasMember("ComponentType") || !compJson["ComponentType"].IsInt()) {
                    AG_CORE_WARN("Prefab deserialization: ComponentType missing for {}", compName);
                    continue;
                }

                int compTypeInt = compJson["ComponentType"].GetInt();
                Data::ComponentTypes compType = static_cast<Data::ComponentTypes>(compTypeInt);

                // Allocate new component instance
                ComponentManager::CompPtr newComp = COMPONENTMANAGER.CreateUserOwnedComponent(compType);
                if (!newComp) {
                    AG_CORE_WARN("Failed to create component during prefab load: {}", compName);
                    continue;
                }

                // Resolve reflection info
                AG::TypeInfo* info = AG::TypeRegistry::Instance().GetType(newComp->GetTypeName());
                if (!info) {
                    AG_CORE_WARN("Unknown component type during prefab load: {}", compName);
                    continue;
                }

                const auto fields = info->getFieldsFunc();
                for (auto& field : fields) {
                    if (!compJson.HasMember(field.name.c_str())) continue;

                    void* ptr = field.getPtr(newComp.get());
                    if (ptr && registry.GetJsonReg(field.type)) {
                        registry.GetJsonReg(field.type)->read(compJson, ptr, field.name);
                    }
                    else {
                        AG_CORE_WARN("Cannot deserialize field {} in component {}", field.name, compName);
                    }
                }

                // Special Script Component Fields
                if (compType == Data::ComponentTypes::Script) {
                    Component::ScriptComponent* scriptComp = reinterpret_cast<Component::ScriptComponent*>(newComp.get());
                    const std::vector<FieldInfo>& serializeFields = scriptComp->getSerializeFields();

                    for (auto& serializeField : serializeFields) {
                        if (!compJson.HasMember(serializeField.name.c_str())) continue;

                        void* ptr = serializeField.getPtr(scriptComp);
                        if (ptr && registry.GetJsonReg(serializeField.type)) {
                            registry.GetJsonReg(serializeField.type)->read(compJson, ptr, serializeField.name);
                        }
                        else {
                            AG_CORE_WARN("Cannot deserialize script serialize field: {}", serializeField.name);
                        }
                    }
                }

                // Append to the output list
                comps.push_back(std::shared_ptr<Component::IComponent>(newComp));
            }

            return true;
        }

        void SerializationSystem::saveBHT(std::filesystem::path fullPath, const AI::BehaviorTreeTemplate& treeTemplate)
        {
            rapidjson::Document document;
            rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator>& allocator = document.GetAllocator();
            document.SetObject();

            AI::BehaviorTreeTemplate::TreeTemplateID id = treeTemplate.getID();
            registry.GetJsonReg(typeid(std::string))->write(allocator, document, &id, "Tree Template ID");

            rapidjson::Value objList{ rapidjson::kArrayType };
            const std::vector<AI::NodeRep>& nodeVec = treeTemplate.getNodesVector();
            for (const AI::NodeRep& node : nodeVec) {
                rapidjson::Value nodeObj{ rapidjson::kObjectType };
                int nodeTypeNum = static_cast<int>(node.nodeType);
                registry.GetJsonReg(typeid(int))->write(allocator, nodeObj, &nodeTypeNum, "Node Type");
                registry.GetJsonReg(typeid(int))->write(allocator, nodeObj, &node.depth, "Node Depth");
                registry.GetJsonReg(typeid(std::string))->write(allocator, nodeObj, &node.name, "Node Name");
                objList.PushBack(nodeObj, allocator);
            }

            document.AddMember("Node List", objList, allocator);

            std::ofstream ofs(fullPath);
            rapidjson::OStreamWrapper osw(ofs);
            rapidjson::PrettyWriter<rapidjson::OStreamWrapper> writer(osw);
            document.Accept(writer);
        }

        void SerializationSystem::loadBHT(std::filesystem::path fullPath, AI::BehaviorTreeTemplate& treeTemplate)
        {
            //Check File
            if (!std::filesystem::exists(fullPath)) {
                AG_CORE_WARN("BHT file not found: {}", fullPath.string());
                return;
            }
            std::ifstream ifs(fullPath);
            if (!ifs.is_open()) {
                AG_CORE_WARN("Failed to open BHT file: {}", fullPath.string());
                return;
            }
            rapidjson::IStreamWrapper isw(ifs);
            rapidjson::Document Doc;
            Doc.ParseStream(isw);
            if (Doc.HasParseError()) {
                AG_CORE_WARN("Failed to deserialize BHT doc");
                return;
            }


            std::string id{};
            registry.GetJsonReg(typeid(std::string))->read(Doc, &id, "Tree Template ID");
            treeTemplate.setTemplateID(std::move(id));

            // --- Load Node List ---
            if (Doc.HasMember("Node List") && Doc["Node List"].IsArray()) {
                const rapidjson::Value& nodeArray = Doc["Node List"];
                std::vector<AI::NodeRep> nodes;

                for (auto& nodeVal : nodeArray.GetArray()) {
                    AI::NodeRep nodeRep;

                    int typeEnum{};
                    registry.GetJsonReg(typeid(int))->read(nodeVal, &typeEnum, "Node Type");
                    nodeRep.nodeType = static_cast<AI::NodeType>(typeEnum);
                    registry.GetJsonReg(typeid(int))->read(nodeVal, &nodeRep.depth, "Node Depth");
                    registry.GetJsonReg(typeid(std::string))->read(nodeVal, &nodeRep.name, "Node Name");

                    nodes.emplace_back(std::move(nodeRep));
                }

                treeTemplate.setNodesVector(std::move(nodes));
            }
            else {
                AG_CORE_WARN("BHT file missing Node List: {}", fullPath.string());
            }
        }

        void SerializationSystem::saveNodeList(const std::unordered_map<std::string, AI::NodeType>& allNodes)
        {
            rapidjson::Document document;
            rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator>& allocator = document.GetAllocator();
            document.SetObject();
            rapidjson::Value nodeList{ rapidjson::kArrayType };

            for (const auto& node : allNodes) {
                rapidjson::Value nodeObj{ rapidjson::kObjectType };
                int enumType = static_cast<int>(node.second);
                registry.GetJsonReg(typeid(int))->write(allocator, nodeObj, &enumType, "Node Type");
                registry.GetJsonReg(typeid(std::string))->write(allocator, nodeObj, &node.first, "Node Name");
                nodeList.PushBack(nodeObj, allocator);
            }

            document.AddMember("Node List", nodeList, allocator);

            std::filesystem::path filePath = "EditorAssets/NodeList.json";
            std::ofstream ofs(filePath);
            rapidjson::OStreamWrapper osw(ofs);
            rapidjson::PrettyWriter<rapidjson::OStreamWrapper> writer(osw);
            document.Accept(writer);
        }

        void SerializationSystem::loadNodeList(std::unordered_map<std::string, AI::NodeType>& allNodes)
        {
            allNodes.clear();

            std::filesystem::path filePath = "EditorAssets/NodeList.json";

            // Check file
            if (!std::filesystem::exists(filePath)) {
                AG_CORE_WARN("NodeList.json not found: {}", filePath.string());
                return;
            }

            std::ifstream ifs(filePath);
            if (!ifs.is_open()) {
                AG_CORE_WARN("Failed to open NodeList.json: {}", filePath.string());
                return;
            }

            rapidjson::IStreamWrapper isw(ifs);
            rapidjson::Document document;
            document.ParseStream(isw);

            if (document.HasParseError()) {
                AG_CORE_WARN("NodeList.json parse error");
                return;
            }

            if (!document.HasMember("Node List") || !document["Node List"].IsArray()) {
                AG_CORE_WARN("NodeList.json missing 'Node List' array");
                return;
            }

            const rapidjson::Value& nodeList = document["Node List"];
            for (const auto& nodeObj : nodeList.GetArray()) {

                int typeEnum{};
                registry.GetJsonReg(typeid(int))->read(nodeObj, &typeEnum, "Node Type");
                AI::NodeType type = static_cast<AI::NodeType>(typeEnum);

                std::string name{};
                registry.GetJsonReg(typeid(std::string))->read(nodeObj, &name, "Node Name");

                // Insert into map
                allNodes[name] = type;
            }
        }

        bool SerializationSystem::LoadGameConfig(std::string& appName, int& windowWidth, int& windowHeight, std::string& openingScene)
        {
            const char* configFilePath = "GameConfig.json";
            std::ifstream ifs(configFilePath);

            if (!ifs.is_open())
            {
                AG_CORE_ERROR("ERROR: Could not open config file {}", configFilePath);
                return false;
            }

            std::stringstream buffer;
            buffer << ifs.rdbuf();
            std::string jsonContent = buffer.str();
            ifs.close();

            rapidjson::Document document;
            document.Parse(jsonContent.c_str());



            if (document.HasMember("AppName") && document["AppName"].IsString())
            {
                appName = document["AppName"].GetString();
            }
            else
            {
                std::cerr << "Config Error: Missing or invalid 'AppName'." << std::endl;
                return false;
            }
            if (document.HasMember("WindowWidth") && document["WindowWidth"].IsInt())
            {
                windowWidth = document["WindowWidth"].GetInt();
            }
            else
            {
                std::cerr << "Config Error: Missing or invalid 'WindowWidth'." << std::endl;
                return false;
            }

            if (document.HasMember("WindowHeight") && document["WindowHeight"].IsInt())
            {
                windowHeight = document["WindowHeight"].GetInt();
            }
            else
            {
                std::cerr << "Config Error: Missing or invalid 'WindowHeight'." << std::endl;
                return false;
            }

            if (document.HasMember("OpeningScene") && document["OpeningScene"].IsString())
            {
                openingScene = document["OpeningScene"].GetString();
            }
            else
            {
                std::cerr << "Config Error: Missing or invalid 'OpeningScene'." << std::endl;
                return false;
            }

            AG_CORE_INFO("Configuration loaded successfully.");
            return true;
        }


        void SerializationSystem::serializePrefabBasicInfo(rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator>& allocator, rapidjson::Value& prefabJsonObj, PrefabID guid, std::string name) {
            registry.GetJsonReg(typeid(std::string))->write(allocator, prefabJsonObj, &name, "Prefab Name");
            registry.GetJsonReg(typeid(System::IObject::ID))->write(allocator, prefabJsonObj, &guid, "Id");
        }
        void SerializationSystem::serializeRootCmp(rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator>& allocator, rapidjson::Value& prefabJsonObj, const ComponentManager::WCompVec& compVec) {
            

            rapidjson::Value rapidjsonPrefabCompList(rapidjson::kObjectType);
            for (auto& comp : compVec) {
                rapidjson::Value rapidjsonPrefabComp(rapidjson::kObjectType);
                AG::TypeInfo* info = AG::TypeRegistry::Instance().GetType(comp.lock()->GetTypeName());
                const auto fields = info->getFieldsFunc();
                for (auto& field : fields) {
                    void* ptr = field.getPtr(comp.lock().get());
                    if (ptr && registry.GetJsonReg(field.type)) {
                        registry.GetJsonReg(field.type)->write(allocator, rapidjsonPrefabComp, ptr, field.name);
                    }
                    else {
                        AG_CORE_WARN("Cant serialize prefab component field: {}", field.name);
                    }
                }
                //Special Case for Script Component
                if (comp.lock()->GetType() == Data::ComponentTypes::Script) {
                    Component::ScriptComponent* scriptComp = reinterpret_cast<Component::ScriptComponent*>(comp.lock().get());
                    const std::vector<FieldInfo>& serializeFields = scriptComp->getSerializeFields();
                    for (auto& serializeField : serializeFields) {
                        void* ptr = serializeField.getPtr(scriptComp);
                        if (ptr && registry.GetJsonReg(serializeField.type)) {
                            registry.GetJsonReg(serializeField.type)->write(allocator, rapidjsonPrefabComp, ptr, serializeField.name);
                        }
                        else {
                            AG_CORE_WARN("Cant serialize script component serialize field: {}", serializeField.name);
                        }
                    }
                }

                rapidjson::Value compType(static_cast<int>(comp.lock()->GetType()));
                rapidjsonPrefabComp.AddMember("ComponentType", compType, allocator);

                rapidjson::Value compName(comp.lock()->GetTypeName().c_str(), static_cast<rapidjson::SizeType>(comp.lock()->GetTypeName().size()), allocator);
                rapidjsonPrefabCompList.AddMember(compName, std::move(rapidjsonPrefabComp), allocator);
            }
            prefabJsonObj.AddMember("Components", rapidjsonPrefabCompList, allocator);
        }

        void SerializationSystem::deserializeRootCmp(const rapidjson::Value& objIns, System::IObject::ID& objID)
        {
            if (!objIns.HasMember("Components") || !objIns["Components"].IsObject())
                return;

            const rapidjson::Value& compList = objIns["Components"];

            for (auto it = compList.MemberBegin(); it != compList.MemberEnd(); ++it)
            {
                const std::string compName = it->name.GetString();
                const rapidjson::Value& compJson = it->value;

                if (!compJson.HasMember("ComponentType") || !compJson["ComponentType"].IsInt()) {
                    AG_CORE_WARN("Prefab deserialization: ComponentType missing for {}", compName);
                    continue;
                }

                int compTypeInt = compJson["ComponentType"].GetInt();
                Data::ComponentTypes compType = static_cast<Data::ComponentTypes>(compTypeInt);

                // Allocate new component instance
                ComponentManager::CompPtr newComp = COMPONENTMANAGER.CreateUserOwnedComponent(compType);
                if (!newComp) {
                    AG_CORE_WARN("Failed to create component during prefab load: {}", compName);
                    continue;
                }

                // Resolve reflection info
                AG::TypeInfo* info = AG::TypeRegistry::Instance().GetType(newComp->GetTypeName());
                if (!info) {
                    AG_CORE_WARN("Unknown component type during prefab load: {}", compName);
                    continue;
                }

                const auto fields = info->getFieldsFunc();
                for (auto& field : fields) {
                    if (!compJson.HasMember(field.name.c_str())) continue;

                    void* ptr = field.getPtr(newComp.get());
                    if (ptr && registry.GetJsonReg(field.type)) {
                        registry.GetJsonReg(field.type)->read(compJson, ptr, field.name);
                    }
                    else {
                        AG_CORE_WARN("Cannot deserialize field {} in component {}", field.name, compName);
                    }
                }

                // Special Script Component Fields
                if (compType == Data::ComponentTypes::Script) {
                    Component::ScriptComponent* scriptComp = reinterpret_cast<Component::ScriptComponent*>(newComp.get());
                    scriptComp->updateMonoFields();
                    const std::vector<FieldInfo>& serializeFields = scriptComp->getSerializeFields();

                    for (auto& serializeField : serializeFields) {
                        if (!compJson.HasMember(serializeField.name.c_str())) continue;

                        void* ptr = serializeField.getPtr(scriptComp);
                        if (ptr && registry.GetJsonReg(serializeField.type)) {
                            registry.GetJsonReg(serializeField.type)->read(compJson, ptr, serializeField.name);
                        }
                        else {
                            AG_CORE_WARN("Cannot deserialize script serialize field: {}", serializeField.name);
                        }
                    }
                }

                //Create componeent with the newComp
                std::weak_ptr<IObject> obj = OBJECTMANAGER.GetObjectByID(objID);
                if (!obj.expired()) {
                    COMPONENTMANAGER.CreateComponent(obj.lock().get(), newComp);
                }
            }
        }

        void SerializationSystem::serializeObjBasicInfo(rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator>& allocator, rapidjson::Value& prefabJsonObj, const IObject::IObjectWPtr& obj)
        {
            //Serilzie Id, Name and prarent
            std::string objName = obj.lock()->GetName();
            System::IObject::ID objID = obj.lock()->GetID();
            System::IObject::ID parrentID = obj.lock()->GetParent() ? obj.lock()->GetParent()->GetID() : Data::GUIDZero();

            registry.GetJsonReg(typeid(std::string))->write(allocator, prefabJsonObj, &(objName), "Object Name");
            registry.GetJsonReg(typeid(System::IObject::ID))->write(allocator, prefabJsonObj, &(objID), "ID");
            registry.GetJsonReg(typeid(System::IObject::ID))->write(allocator, prefabJsonObj, &(parrentID), "Parent ID");
        }

        void SerializationSystem::deserializeObjBasicInfo(const rapidjson::Value& objInst, System::IObject::ID& objID)
        {
            if (!objInst.HasMember("Object Name") || !objInst["Object Name"].IsString() ||
                !objInst.HasMember("ID")          || !objInst["ID"].IsString()) {
                AG_CORE_WARN("Cannot deserialize object basic info");
                return;
            }
            std::string objName = objInst["Object Name"].GetString();
            objID = objInst["ID"].GetString();

            OBJECTMANAGER.CreateObjectWithID(objName, objID);

            //Parent Child do seperately at the end
        }

        void SerializationSystem::deserializePrefabInstanceComponent(const rapidjson::Value& prefabInst, System::IObject::ID& objID)
        {
            std::weak_ptr<IObject> obj = OBJECTMANAGER.GetObjectByID(objID);
            if (obj.expired()) {
                AG_CORE_WARN("Prefab Deserialization: No such object");
                return;
            }
            if (!prefabInst.HasMember("Prefab ID") || !prefabInst["Prefab ID"].IsString() ||
                !prefabInst.HasMember("Prefab Name") || !prefabInst["Prefab Name"].IsString() ||
                !prefabInst.HasMember("Overrides") || !prefabInst["Overrides"].IsArray()) {
                AG_CORE_WARN("Prefab Deserialization: prefab Component failed");
                return;
            }
            System::PrefabID prefabID = prefabInst["Prefab ID"].GetString();
            std::string prefabName = prefabInst["Prefab Name"].GetString();
            const auto& overrides = prefabInst["Overrides"];
            Component::PrefabComponent* ptr = reinterpret_cast<Component::PrefabComponent*>(COMPONENTMANAGER.CreateComponent(obj.lock().get(), Data::ComponentTypes::Prefab));
            ptr->setPrefab(prefabID, prefabName);
            System::jsonDoc newDoc;
            newDoc.CopyFrom(overrides, newDoc.GetAllocator());
            ptr->setPrefabOverride(newDoc);
            
            //Fetch Template
            const std::unordered_map<PrefabID, PrefabTemplate>& idToTemplate = PREFABMANAGER.getIDToTemplate();
            if (idToTemplate.find(prefabID) == idToTemplate.end()) {
                AG_CORE_WARN("Prefab Deserialization: No such template");
                return;
            }

            //Apply template to the object
            ComponentManager::CompVec comps{};
            SERIALIZATIONSYSTEM.deserializePrefabRootCmp(idToTemplate.at(prefabID).getDoc(), comps);
            for (auto& comp : comps) {
                COMPONENTMANAGER.CreateComponent(obj.lock().get(), comp);
            }

            //Apply override to the object
            overrideSerializer.applyOverride(objID, overrides);
        }

        void SerializationSystem::deserializeHirachy(const std::unordered_map<Data::GUID, Data::GUID>& parentOf)
        {
            auto root = OBJECTMANAGER.GetRoot();
            if (!root) {
                root = OBJECTMANAGER.CreateObject("__root__", true, true);
                root->SetID(Data::GUIDZero());
            }

            for (const auto& [childID, parentID] : parentOf)
            {
                if (Data::IsZero(childID)) continue;
                auto child = OBJECTMANAGER.GetObjectByID(childID);
                auto parent = (Data::IsZero(parentID)) ? root : OBJECTMANAGER.GetObjectByID(parentID);
                if (child && parent && child != parent)
                    OBJECTMANAGER.LinkParentChild(parent, child);
            }
        }

    } // namespace System
} // namespace AG
