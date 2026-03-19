#pragma once
#include "../../Header/pch.h"
#include "PrefabTemplate.h"
#include "PrefabLink.h"

#define PREFAB_DIR std::filesystem::path("Assets") / "Prefab" 

namespace AG {
	namespace System {
		class PrefabManager : public  Pattern::ISingleton<PrefabManager> {
		public:
			PrefabManager();

			void loadPrefab(jsonDoc prefabDoc, const std::filesystem::path& prefabfileFilePath); //Load the file from assets manager

			void InstantiatePrefabInstance(const std::filesystem::path& prefabFilePath);
			void CreatePrefabTemplate(IObject::IObjectWPtr obj);

			const std::unordered_map<Data::GUID, PrefabTemplate>& getIDToTemplate() const;
		private:
			std::unordered_map<Data::GUID, PrefabTemplate> idToTemplate;
		};
	}
}
#define PREFABMANAGER AG::System::PrefabManager::GetInstance()
