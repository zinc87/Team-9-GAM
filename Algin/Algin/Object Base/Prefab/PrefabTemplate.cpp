#include "pch.h"

#include "PrefabTemplate.h"

namespace AG {
	namespace System {
		PrefabID PrefabTemplate::getID() const {
			return m_id;
		}

		std::string PrefabTemplate::getName() const {
			return m_name;
		}

		std::filesystem::path PrefabTemplate::getPath() const {
			return prefabFilePath;
		}

		const jsonDoc& PrefabTemplate::getDoc() const {
			return prefabDoc;
		}

		bool PrefabTemplate::loadFromJson(const jsonDoc& newDoc, const std::filesystem::path& newFilePath) {
			prefabDoc.CopyFrom(newDoc, prefabDoc.GetAllocator());
			prefabFilePath = newFilePath;
			return SERIALIZATIONSYSTEM.deserializePrefabBasicInfo(prefabDoc, *this);
		}

		bool PrefabTemplate::createFromObject(const IObject::IObjectWPtr& obj) {
			m_id = Data::GenerateGUID();
			m_name = obj.lock()->GetName();
			
			rapidjson::Document document;
			document.SetObject();
			if (SERIALIZATIONSYSTEM.serializePrefab(document, m_id, obj)) {
				prefabFilePath = PREFAB_DIR / (m_name + ".prefab");
				//Check whether file already exist
				int i{};
				while (std::filesystem::exists(prefabFilePath)) {
					prefabFilePath = PREFAB_DIR / (m_name + std::to_string(i++) + ".prefab");
				}

				prefabDoc.CopyFrom(document, prefabDoc.GetAllocator());
				
				std::ofstream ofs(prefabFilePath);
				rapidjson::OStreamWrapper osw(ofs);
				rapidjson::PrettyWriter<rapidjson::OStreamWrapper> writer(osw);
				prefabDoc.Accept(writer);
				

				return true;
			}
			return false;
		}
	}
}