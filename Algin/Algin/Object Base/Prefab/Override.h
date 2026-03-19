#pragma once
#include "pch.h"

namespace AG {
	namespace System {
		struct PrefabOverride {
			enum class OpType
			{
				Edit,
				Add,
				Remove
			};

			OpType op;
			Data::ComponentTypes componentType;
			std::string pathToField;
			rapidjson::Value value;
		};

		class PrefabOverrideSerializer {
		public:
			void CreatePrefabOverride(const jsonDoc& beforeState, const jsonDoc& afterState, jsonDoc& overrideResult, jsonDoc& finalResult);

			void applyOverride(System::IObject::ID objID, const rapidjson::Value& overrides);
		private:
			void pushOpObject(const char* opName, const std::string& path, const rapidjson::Value* valuePtr, Data::ComponentTypes compType, jsonDoc& resultArray);
			void diffObject(const rapidjson::Value& beforeState, const rapidjson::Value& afterState, const std::string& path, Data::ComponentTypes compTypes, jsonDoc& resultArray);
			void mergeAndCleanUp(const jsonDoc& existingOverrides, const jsonDoc& diffResult, jsonDoc& result);

			bool Equal(const rapidjson::Value& a, const rapidjson::Value& b);
			std::vector<std::string> SplitPath(const std::string& path);
			rapidjson::Value* GetJsonValueByPath(rapidjson::Value& root, const std::string& path);

			void scriptApplyOverride(const System::IObject::ID& objID, const std::vector<System::jsonDoc>& compOverrides);
			void normalApplyOverride(const System::IObject::ID& objID, Data::ComponentTypes compType, std::vector<System::jsonDoc>& compOverrides);
		};
	}
}