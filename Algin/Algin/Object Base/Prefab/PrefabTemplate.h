#pragma once
#include "pch.h"

#include "PrefabLink.h"

namespace AG {
	namespace System {

		using PrefabID = Data::GUID;
		using jsonDoc = rapidjson::Document;

		class PrefabTemplate {
		public:
			//Getter
			PrefabID getID() const;
			std::string getName() const;
			std::filesystem::path getPath() const;
			const jsonDoc& getDoc() const;

			bool loadFromJson(const jsonDoc& newDoc, const std::filesystem::path & newFilePath); //Deserilize the basic info into the template
			bool createFromObject(const IObject::IObjectWPtr& obj); //Create a prefab template from an object

		private:
			std::string m_name; //Name for prefab (Mainly for debug or human read, should not use it as any comparison)
			PrefabID m_id;      //GUID representing the prefab template
			jsonDoc prefabDoc;  //Json Doc that store the whole prefab data like component, nested object and nested prefab
			std::filesystem::path prefabFilePath; //Json file for prefab
			Link links; //Container for keeping links representing the Template's nested instances

			bool isDirty = false; //Changse to the template that havent save to file
		private:
			friend class SerializationSystem;
		};
	}
}