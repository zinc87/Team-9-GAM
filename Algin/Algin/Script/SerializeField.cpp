#include "pch.h"

namespace AG {
	
	//2. Register the Mono type name, C++ type, and ScriptFieldType enum into the registry
	ScriptTypeRegistry::ScriptTypeRegistry() {
		Register<float>(ScriptFieldType::Float, "System.Single");
		Register<int>(ScriptFieldType::Int, "System.Int32");
		Register<unsigned int>(ScriptFieldType::UInt, "System.UInt32");
		Register<double>(ScriptFieldType::Double, "System.Double");
		Register<System::IObject>(ScriptFieldType::Object, "Script.Library.Object");
		Register<bool>(ScriptFieldType::Bool, "System.Boolean");
		Register<std::string>(ScriptFieldType::String, "System.String");
		Register<std::size_t>(ScriptFieldType::Image2D, "Script.Library.Image2D");
	}
	ScriptFieldTypeInfo ScriptTypeRegistry::getInfo(std::string monoTypeName)
	{
		if (monoTypeNameToTypeInfo.find(monoTypeName) != monoTypeNameToTypeInfo.end()) {
			return monoTypeNameToTypeInfo.at(monoTypeName);
		}
		AG_CORE_WARN("Unsupported Type({})", monoTypeName);
		return ScriptFieldTypeInfo();
	}
	ScriptFieldTypeInfo ScriptTypeRegistry::getInfo(ScriptFieldType type)
	{
		if (TypeEnumToTypeInfo.find(type) != TypeEnumToTypeInfo.end()) {
			return TypeEnumToTypeInfo.at(type);
		}
		AG_CORE_WARN("Unsupported Type({})", (int)type);
		return ScriptFieldTypeInfo();
	}

	void TestRegisteredScriptTypes()
	{
		/*std::vector<std::string> typeNames = {
		"System.Single",
		"System.Int32",
		"System.UInt32",
		"System.Double",
		"Script.Library.Object",
		"System.Boolean"
		};

		for (auto& typeName : typeNames)
		{
			auto info = SCRIPTTYPEREGISTRY.getInfo(typeName);
			if (info.type == ScriptFieldType::Unknown)
				AG_CORE_WARN("Type '{}' not found in registry!", typeName);
			else
				AG_CORE_INFO("Type '{}' registered as {}", typeName, (int)info.type);
		}*/
	}
}