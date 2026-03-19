#include "pch.h"
#include "PrefabManager.h"

namespace AG {
	namespace System {
		PrefabManager::PrefabManager()
		{
			//Do the hot reload
		}

		void PrefabManager::loadPrefab(jsonDoc prefabDoc, const std::filesystem::path& prefabfileFilePath) {
			//Crerate template here
			PrefabTemplate newPrefab;
			if (!newPrefab.loadFromJson(prefabDoc, prefabfileFilePath)) {
				AG_CORE_WARN("Fail to load from JSON : {}", prefabfileFilePath.filename().string());
			}
			//Add to our map
			idToTemplate.emplace(newPrefab.getID(), std::move(newPrefab));
		}

		void PrefabManager::InstantiatePrefabInstance(const std::filesystem::path& prefabFilePath) {
			for (auto& prefab : idToTemplate) {
				if (prefabFilePath == prefab.second.getPath()) {
					ComponentManager::CompVec comps{};
					SERIALIZATIONSYSTEM.deserializePrefabRootCmp(prefab.second.getDoc(), comps);

					std::shared_ptr<IObject> obj = OBJECTMANAGER.CreateObject();
					Component::PrefabComponent* ptr = reinterpret_cast<Component::PrefabComponent*>(COMPONENTMANAGER.CreateComponent(obj.get(), Data::ComponentTypes::Prefab));
					ptr->setPrefab(prefab.second.getID(), prefab.second.getName());
					for (auto& comp : comps) {
						COMPONENTMANAGER.CreateComponent(obj.get(), comp);
					}
					return;
				}
			}

			AG_CORE_WARN("Prefab {} dont exist", prefabFilePath.stem().string());
		}
		void PrefabManager::CreatePrefabTemplate(IObject::IObjectWPtr obj) {
			PrefabTemplate newPrefab;
			if (newPrefab.createFromObject(obj)) {
				//If existing prefab instance want to create another prefab, instance will not use new prefab, will remain with the existing prefab
				if (!obj.lock()->hasComponent(Data::ComponentTypes::Prefab)) {
					Component::PrefabComponent* ptr = reinterpret_cast<Component::PrefabComponent*>(COMPONENTMANAGER.CreateComponent(obj.lock().get(), Data::ComponentTypes::Prefab));
					ptr->setPrefab(newPrefab.getID(), newPrefab.getName());
				}
				PrefabID newPrefabID = newPrefab.getID();
				idToTemplate.emplace(newPrefabID, std::move(newPrefab));
			}
		}
		const std::unordered_map<Data::GUID, PrefabTemplate>& PrefabManager::getIDToTemplate() const
		{
			return idToTemplate;
		}
	}
}

#define PREFABMANAGER AG::System::PrefabManager::GetInstance()