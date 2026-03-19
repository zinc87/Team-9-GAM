#pragma once
#include <string>
#include <sstream>
#include <unordered_map>
#include <vector>


#include "../Header/pch.h"
#include "../Object Base/IObject.h"
#include "../Component/IComponent.h"
#include "../Object Base/Prefab/PrefabManager.h"
#include "SerializerRegistry.h"
#include "../Object Base/Prefab/Override.h"
#include "../BHT/AIManager.h"

namespace AG {
    namespace System {
        class SerializationSystem : public Pattern::ISingleton<SerializationSystem>{
        public:
            bool SaveScene(const std::string& filename,
                ObjectManager& objMgr,
                ComponentManager& compMgr);

            bool LoadScene(const std::string& filename,
                ObjectManager& objMgr,
                ComponentManager& compMgr);
            //Scene
            bool SaveSceneJson(std::filesystem::path fullPath);
            bool loadSceneJson(std::filesystem::path filePath);

            //Component (Normal object or (maybe)prefab Instance)
            void serializeRootCmp(rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator>& allocator, rapidjson::Value& prefabJsonObj, const ComponentManager::WCompVec& obj);
            void deserializeRootCmp(const rapidjson::Value& objIns, System::IObject::ID& objID);

            //Normal Object
            bool serializeObject(rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator>& allocator, rapidjson::Value& prefabInstObj, const IObject::IObjectWPtr& obj);
            bool deserializeObject(const rapidjson::Value& objIns);

            //Prefab Instance
            bool serializePrefabInstance(rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator>& allocator, rapidjson::Value& prefabInstObj, const IObject::IObjectWPtr& obj);
            bool deserializePrefabInstance(const rapidjson::Value& prefabIns);

            //Prefab Template
            bool serializePrefab(System::jsonDoc& prefabDoc, PrefabID guid, const IObject::IObjectWPtr& obj);
            bool deserializePrefabBasicInfo(const System::jsonDoc& prefabDoc, PrefabTemplate& resultPrefab);
            bool deserializePrefabRootCmp(const System::jsonDoc& prefabDoc, ComponentManager::CompVec& comps);

            //BHT related file
            void saveBHT(std::filesystem::path fullPath, const AI::BehaviorTreeTemplate& treeTemplate);
            void loadBHT(std::filesystem::path fullPath, AI::BehaviorTreeTemplate& treeTemplate);
            void saveNodeList(const std::unordered_map<std::string, AI::NodeType>& allNodes);
            void loadNodeList(std::unordered_map<std::string, AI::NodeType>& allNodes);

            //GameConfig
            bool LoadGameConfig(std::string& appName, int& windowWidth, int& windowHeight, std::string& openingScene);

            SerializerRegistry& getRegistry(){ return registry; }
            PrefabOverrideSerializer& getOverrideSerializer() { return overrideSerializer; }

        private:
            std::string EscapeString(const std::string& s);
            std::string Indent(int level);

            void WriteComponent(std::ostream& out,
                Component::IComponent* comp,
                int indent);

            void WriteObject(std::ostream& out,
                std::shared_ptr<AG::System::IObject> obj,
                ComponentManager& compMgr,
                int indent);


            void serializePrefabBasicInfo(rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator>& allocator, rapidjson::Value& prefabJsonObj, PrefabID guid, std::string name);           
            void serializeObjBasicInfo(rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator>& allocator, rapidjson::Value& prefabJsonObj, const IObject::IObjectWPtr& obj);

            void deserializeObjBasicInfo(const rapidjson::Value& objInst, System::IObject::ID& objID);

            void deserializePrefabInstanceComponent(const rapidjson::Value& prefabInst, System::IObject::ID& objID);

            void deserializeHirachy(const std::unordered_map<Data::GUID, Data::GUID>& parentOf);
        private:
            SerializerRegistry registry;
            PrefabOverrideSerializer overrideSerializer;
        };
    }
}

#define SERIALIZATIONSYSTEM AG::System::SerializationSystem::GetInstance()
