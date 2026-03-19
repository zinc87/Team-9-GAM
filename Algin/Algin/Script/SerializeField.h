#pragma once
#include "pch.h"

/*
	Guide to add supported serialize field:
	1. Add the Type for serialize field in the enum class ScriptFieldType
	2. Register the Mono type name, C++ type, and ScriptFieldType enum into the registry
	3. Ensure that reflection system support the newly added type
*/

namespace AG {
	//1. Add the Type for serialize field in the enum class ScriptFieldType
	enum class ScriptFieldType {
		Unknown,
		Float,
		Int,
		UInt,
		Double,
		Object,
		Bool,
		String,
		Image2D
	};

	//Field Type metadata
	struct ScriptFieldTypeInfo {
		ScriptFieldType type{ ScriptFieldType::Unknown };
		std::string monoTypeName{};
		std::type_index cppType{typeid(void)};
		std::function<void* ()> createFn{};
		std::function<void(void*)> destroyfn{};
		std::function<void(void* dst, void* src)> copyFn;
	};
	
	struct ScriptField {
		std::string name;
		ScriptFieldType type{ ScriptFieldType::Unknown};
		MonoClassField* field{nullptr};
	};

	class ScriptTypeRegistry : public Pattern::ISingleton<ScriptTypeRegistry> {
	public:
		ScriptTypeRegistry();

		ScriptFieldTypeInfo getInfo(std::string monoTypeName);
		ScriptFieldTypeInfo getInfo(ScriptFieldType type);
	private:
		template <typename T>
		void Register(ScriptFieldType fieldType, const std::string& monoType) {
			ScriptFieldTypeInfo info;
			info.type = fieldType;
			info.monoTypeName = monoType;
			info.cppType = std::type_index(typeid(T));
			info.createFn = []() -> void* { return new T(); };
			info.destroyfn = [](void* ptr) {delete static_cast<T*>(ptr); };
			info.copyFn = [](void* dst, void* src) {*static_cast<T*>(dst) = *static_cast<T*>(src); };

			monoTypeNameToTypeInfo[monoType] = info;
			TypeEnumToTypeInfo[fieldType] = info;
		}
	private:
		std::unordered_map<std::string, ScriptFieldTypeInfo> monoTypeNameToTypeInfo;
		std::unordered_map<ScriptFieldType, ScriptFieldTypeInfo> TypeEnumToTypeInfo;
	};

	void TestRegisteredScriptTypes();
	
}

#define SCRIPTTYPEREGISTRY ScriptTypeRegistry::GetInstance()