#include "pch.h"

#define REGISTER_SCRIPT_METHOD(methodName, paramNum) if (nameToMethod[#methodName] == nullptr) {\
													nameToMethod[#methodName] = SCRIPTENGINE.getScriptParentClass().getMethod(#methodName, paramNum);\
													}\

namespace AG {
	ScriptClass::ScriptClass(const std::string& newNameSpace, const std::string& newClassName) : nameSpace{newNameSpace}, className{newClassName}, fields{}
	{
		MonoImage* image = newNameSpace == "Script.Library" && (className == "IScript" || className == "INode") ? SCRIPTENGINE.getApiImage() : SCRIPTENGINE.getScriptImage();
		monoClass = mono_class_from_name(image, nameSpace.c_str(), className.c_str());
		reflectMonoFields();
	}
	MonoClass* ScriptClass::getClass()
	{
		return monoClass;
	}
	std::string ScriptClass::getClassName() {
		return className;
	}
	MonoObject* ScriptClass::instantiate()
	{
		MonoObject* instance = mono_object_new(SCRIPTENGINE.getAppDomain(), monoClass);
		mono_runtime_object_init(instance);
		if (!instance)
			AG_CORE_ERROR("Failed to init mono instance");
		return instance;
	}
	MonoMethod* ScriptClass::getMethod(std::string functionName, int paramCount)
	{
		return mono_class_get_method_from_name(monoClass, functionName.c_str(), paramCount);
	}
	void ScriptClass::invokeMethod(MonoMethod* method, uint32_t gchandle, void** param)
	{
		if (!method) {
			AG_CORE_ERROR("Mono: invokeMethod called with null method on {}.{}", nameSpace, className);
			return;
		}
		MonoObject* target = mono_gchandle_get_target(gchandle);
		if (!target) {
			AG_CORE_ERROR("Mono: invokeMethod target is null for {}.{}", nameSpace, className);
			return;
		}
		MonoObject* exception = nullptr;
		mono_runtime_invoke(method, target, param, &exception);
		if (exception) {
			// There really was an exception thrown
			MonoString* msg = mono_object_to_string(exception, nullptr);
			if (!msg) {
				AG_CORE_ERROR("Mono: exception thrown in {}.{} (failed to stringify)", nameSpace, className);
				return;
			}
			char* cstr = mono_string_to_utf8(msg);
			if (!cstr) {
				AG_CORE_ERROR("Mono: exception thrown in {}.{} (utf8 conversion failed)", nameSpace, className);
				return;
			}
			AG_CORE_ERROR("Mono: {}", cstr);
			mono_free(cstr);
		}
	}
	std::unordered_map<std::string, ScriptField>& ScriptClass::getFields()
	{
		return fields;
	}
	void ScriptClass::reflectMonoFields()
	{
		void* iter = nullptr;
		MonoClassField* monofield = mono_class_get_fields(monoClass, &iter);

		while (monofield){
			MonoCustomAttrInfo* attrInfo = mono_custom_attrs_from_field(monoClass, monofield);
			if (!attrInfo) {
				monofield = mono_class_get_fields(monoClass, &iter);
				continue;
			}
			MonoClass* serializeAttrClass = mono_class_from_name(SCRIPTENGINE.getApiImage(), "Script.Library", "SerializeField");
			if (!serializeAttrClass)
			{
				AG_CORE_WARN("SerializeField attribute class not found!");
				monofield = mono_class_get_fields(monoClass, &iter);
				continue;
			}
			if (mono_custom_attrs_has_attr(attrInfo, serializeAttrClass))
			{
				const char* fieldName = mono_field_get_name(monofield);
				MonoType* fieldType = mono_field_get_type(monofield);

				ScriptField sf;
				sf.name = fieldName;
				sf.field = monofield;
				std::string typeName = mono_type_get_name(fieldType);
				sf.type = SCRIPTTYPEREGISTRY.getInfo(typeName).type;
				

				fields[sf.name] = sf;
				//AG_CORE_INFO("loaded {} of type {}({}) from {}", fieldName, typeName, (int)sf.type, className);
			}

			monofield = mono_class_get_fields(monoClass, &iter);
			mono_custom_attrs_free(attrInfo);
		}
	}
	ScriptInstance::ScriptInstance(ScriptClass* newScriptClass, System::IObject::ID entityID, const std::unordered_map<std::string, std::unique_ptr<void, std::function<void(void*)>>>& fieldNameToData) : scriptClass{ newScriptClass }
	{
		instance = scriptClass->instantiate();
		gcHandle = mono_gchandle_new(instance, true);
		nameToMethod["ParamConstructor"] = SCRIPTENGINE.getScriptParentClass().getMethod(".ctor", 1);
		nameToMethod["Awake"] = scriptClass->getMethod("Awake", 0);
		nameToMethod["Start"] = scriptClass->getMethod("Start", 0);
		nameToMethod["Update"] = scriptClass->getMethod("Update", 1);
		nameToMethod["LateUpdate"] = scriptClass->getMethod("LateUpdate", 1);
		nameToMethod["Free"] = scriptClass->getMethod("Free", 0);

		REGISTER_SCRIPT_METHOD(onHover, 0);
		REGISTER_SCRIPT_METHOD(leftOnClick, 0);
		REGISTER_SCRIPT_METHOD(middleOnClick, 0);
		REGISTER_SCRIPT_METHOD(rightOnClick, 0);
		REGISTER_SCRIPT_METHOD(leftOnRelease, 0);
		REGISTER_SCRIPT_METHOD(middleOnRelease, 0);
		REGISTER_SCRIPT_METHOD(rightOnRelease, 0);

		MonoString* monoEntityID = mono_string_new(SCRIPTENGINE.getAppDomain(), entityID.c_str());
		invokeMethod("ParamConstructor", { monoEntityID });

		for (auto field : newScriptClass->getFields()) {
			if (fieldNameToData.find(field.second.name) == fieldNameToData.end()) {
				AG_CORE_WARN("data of field({}) of entity {} cant be found", field.second.name, entityID);
			}
			else if (field.second.type == ScriptFieldType::Object) {
				MonoClass* objClass = mono_class_from_name(SCRIPTENGINE.getApiImage(), "Script.Library", "Object");
				MonoObject* fieldInstance = mono_object_new(SCRIPTENGINE.getAppDomain(), objClass);
				uint32_t fieldInstancegcHandle = mono_gchandle_new(fieldInstance, true);
				MonoMethod* paramCtr = mono_class_get_method_from_name(objClass, ".ctor", 1);
				MonoObject* exception = nullptr;
				System::IObject::ID ID = reinterpret_cast<System::IObject*>(fieldNameToData.at(field.second.name).get())->GetID();
				MonoString* monoFieldObjectID = mono_string_new(SCRIPTENGINE.getAppDomain(), ID.c_str());
				void* args[1];
				args[0] = monoFieldObjectID;
				mono_runtime_invoke(paramCtr, mono_gchandle_get_target(fieldInstancegcHandle), args, &exception);
				if (exception) {
					// There really was an exception thrown
					MonoString* msg = mono_object_to_string(exception, nullptr);
					char* cstr = mono_string_to_utf8(msg);
					AG_CORE_ERROR("Mono: {}", cstr);
					mono_free(cstr);
				}
				mono_field_set_value(instance, field.second.field, fieldInstance);
			}
			else if (field.second.type == ScriptFieldType::String) {
				std::string* stringValue = static_cast<std::string*>(fieldNameToData.at(field.second.name).get());
				if (!stringValue) {
					continue;
				}
				MonoString* monoStringValue = mono_string_new(SCRIPTENGINE.getAppDomain(), stringValue->c_str());
				mono_field_set_value(instance, field.second.field, monoStringValue);
			}
			else if (field.second.type == ScriptFieldType::Image2D) {
				MonoClass* objClass = mono_class_from_name(SCRIPTENGINE.getApiImage(), "Script.Library", "Image2D");
				MonoObject* fieldInstance = mono_object_new(SCRIPTENGINE.getAppDomain(), objClass);
				uint32_t fieldInstancegcHandle = mono_gchandle_new(fieldInstance, true);
				MonoMethod* paramCtr = mono_class_get_method_from_name(objClass, ".ctor", 1);
				MonoObject* exception = nullptr;
				std::size_t ID = *reinterpret_cast<std::size_t*>(fieldNameToData.at(field.second.name).get());
				void* args[1];
				args[0] = &ID;
				mono_runtime_invoke(paramCtr, mono_gchandle_get_target(fieldInstancegcHandle), args, &exception);
				if (exception) {
					// There really was an exception thrown
					MonoString* msg = mono_object_to_string(exception, nullptr);
					char* cstr = mono_string_to_utf8(msg);
					AG_CORE_ERROR("Mono: {}", cstr);
					mono_free(cstr);
				}
				mono_field_set_value(instance, field.second.field, fieldInstance);
			}
			else {
				mono_field_set_value(instance, field.second.field, fieldNameToData.at(field.second.name).get());
			}
		}
	}
	void ScriptInstance::invokeMethod(std::string methodName, std::vector<void*> param)
	{
		void** args = param.empty() ? nullptr : param.data();
		scriptClass->invokeMethod(nameToMethod[methodName], gcHandle, args);
	}
	NodeInstance::NodeInstance(ScriptClass* newNodeClass, const System::IObject::ID& entityID, const AI::AINode::NodeID& nodeID, const std::unordered_map<std::string, std::unique_ptr<void, std::function<void(void*)>>>& fieldNameToData) : nodeClass{ newNodeClass }
	{
		instance = nodeClass->instantiate();
		gcHandle = mono_gchandle_new(instance, true);
		nameToMethod["ParamConstructor"] = SCRIPTENGINE.getNodeParentClass().getMethod(".ctor", 2);
		nameToMethod["onEnter"] = nodeClass->getMethod("onEnter", 0);
		if (nameToMethod["onEnter"] == nullptr) {
			nameToMethod["onEnter"] = SCRIPTENGINE.getNodeParentClass().getMethod("onEnter", 0);
		}
		nameToMethod["onUpdate"] = nodeClass->getMethod("onUpdate", 1);
		if (nameToMethod["onUpdate"] == nullptr) {
			nameToMethod["onUpdate"] = SCRIPTENGINE.getNodeParentClass().getMethod("onUpdate", 1);
		}
		nameToMethod["onExit"] = nodeClass->getMethod("onExit", 0);
		if (nameToMethod["onExit"] == nullptr) {
			nameToMethod["onExit"] = SCRIPTENGINE.getNodeParentClass().getMethod("onExit", 0);
		}

		MonoString* monoEntityID = mono_string_new(SCRIPTENGINE.getAppDomain(), entityID.c_str());
		MonoString* monoNodeID = mono_string_new(SCRIPTENGINE.getAppDomain(), nodeID.c_str());
		invokeMethod("ParamConstructor", { monoEntityID, monoNodeID });

		for (auto field : newNodeClass->getFields()) {
			if (fieldNameToData.find(field.second.name) == fieldNameToData.end()) {
				AG_CORE_WARN("data of field({}) of entity {} cant be found", field.second.name, entityID);
			}
			else if (field.second.type == ScriptFieldType::Object) {
				MonoClass* objClass = mono_class_from_name(SCRIPTENGINE.getApiImage(), "Script.Library", "Object");
				MonoObject* fieldInstance = mono_object_new(SCRIPTENGINE.getAppDomain(), objClass);
				uint32_t fieldInstancegcHandle = mono_gchandle_new(fieldInstance, true);
				MonoMethod* paramCtr = mono_class_get_method_from_name(objClass, ".ctor", 1);
				MonoObject* exception = nullptr;
				System::IObject::ID ID = reinterpret_cast<System::IObject*>(fieldNameToData.at(field.second.name).get())->GetID();
				MonoString* monoFieldObjectID = mono_string_new(SCRIPTENGINE.getAppDomain(), ID.c_str());
				void* args[1];
				args[0] = monoFieldObjectID;
				mono_runtime_invoke(paramCtr, mono_gchandle_get_target(fieldInstancegcHandle), args, &exception);
				if (exception) {
					// There really was an exception thrown
					MonoString* msg = mono_object_to_string(exception, nullptr);
					char* cstr = mono_string_to_utf8(msg);
					AG_CORE_ERROR("Mono: {}", cstr);
					mono_free(cstr);
				}
				mono_field_set_value(instance, field.second.field, fieldInstance);
			}
			else {
				mono_field_set_value(instance, field.second.field, fieldNameToData.at(field.second.name).get());
			}
		}
	}
	void NodeInstance::invokeMethod(std::string methodName, std::vector<void*> param)
	{
		void** args = param.empty() ? nullptr : param.data();
		nodeClass->invokeMethod(nameToMethod[methodName], gcHandle, args);
	}
	void ScriptEngine::init() {
		mono_set_assemblies_path("mono/4.5");
		// Store the root domain pointer
		scriptEngineData.rootDomain = mono_jit_init("ProjectRuntime");
		if (scriptEngineData.rootDomain == nullptr) {
			throw("Failed to initialize Mono");
		}

		if (recompileGameScript())
		{
			AG_CORE_INFO("[SCRIPT] Compiled GameScript");
		}
		else
		{
			AG_CORE_WARN("[SCRIPT] Failed to compile GameScript");
		}

		std::filesystem::path GSPath = std::filesystem::current_path().string() + "\\GameScripts.dll";

		if (std::filesystem::exists(GSPath))
		{
			AG_CORE_INFO("[SCRIPT] Loading GameScript.dll");
			loadScriptAssembly();
		}

		scriptConnector.init();
		registerFileWatch();
	}
	void ScriptEngine::update()
	{
		if (toReload && !SCENESTATEMANAGER.IsSceneRunning()) {
			reloadAssembly();
			AI::TreeBuilder::GetInstance().reload();
			toReload = false;
		}
	}
	void ScriptEngine::terminate() {
		unloadScriptAssembly();

		mono_jit_cleanup(scriptEngineData.rootDomain);
		scriptEngineData.rootDomain = nullptr;
	}
	MonoImage* ScriptEngine::getApiImage()
	{
		return scriptEngineData.apiassemblyImage;
	}
	MonoDomain* ScriptEngine::getAppDomain()
	{
		return scriptEngineData.appDomain;
	}
	MonoImage* ScriptEngine::getScriptImage()
	{
		return scriptEngineData.scriptAssemblyImage;
	}
	ScriptClass ScriptEngine::getScriptParentClass()
	{
		return scriptEngineData.scriptParentClass;
	}
	ScriptClass ScriptEngine::getNodeParentClass() {
		return scriptEngineData.nodeParentClass;
	}
	ScriptConnector& ScriptEngine::getScriptConnector()
	{
		return scriptConnector;
	}
	std::unordered_map<std::string, ScriptField> ScriptEngine::getFields(std::string fullClassName)
	{
		if (scriptEngineData.scriptEntityClasses.find(fullClassName) != scriptEngineData.scriptEntityClasses.end()) {
			return scriptEngineData.scriptEntityClasses[fullClassName]->getFields();
		}
		else {
			AG_CORE_WARN("Cant load field for class[{}]", fullClassName);
			return std::unordered_map<std::string, ScriptField>{};
		}
	}
	void ScriptEngine::reloadAssembly()
	{
		if (recompileGameScript()) {
			unloadScriptAssembly();
			loadScriptAssembly();
			scriptConnector.init();
		}
	}
	void ScriptEngine::registerFileWatch()
	{
		FILEWATCHER.watch("Assets/Script",
			[&](const std::string&, filewatch::Event) {
				toReload = true;
			});
	}
	bool ScriptEngine::scriptExist(const std::string& fullClassName)
	{
		return (scriptEngineData.scriptEntityClasses.find(fullClassName) != scriptEngineData.scriptEntityClasses.end());
	}
	void ScriptEngine::getClassName(std::vector<const char*>& resultVector) const
	{
		resultVector.reserve(scriptEngineData.scriptEntityClasses.size());
		for (auto& className : scriptEngineData.scriptClassNameVector) {
			resultVector.emplace_back(className.c_str());
		}
	}
	void ScriptEngine::createInstance(System::IObject::ID entityID, const std::string& fullClassName, const std::unordered_map<std::string, std::unique_ptr<void, std::function<void(void*)>>>& fieldNameToData)
	{
		//Create script Instance
		if (scriptExist(fullClassName)) {
			scriptEngineData.scriptInstances[entityID] = std::make_unique<ScriptInstance>(scriptEngineData.scriptEntityClasses[fullClassName].get(), entityID, fieldNameToData);
			scriptEngineData.scriptInstances[entityID].get()->invokeMethod("Awake", {});
			return;
		}
		AG_CORE_ERROR("{} doesnt match with the name in mono", fullClassName);
	}
	void ScriptEngine::instanceOnStart(System::IObject::ID entityID)
	{
		if (scriptEngineData.scriptInstances.find(entityID) == scriptEngineData.scriptInstances.end()) return;
		scriptEngineData.scriptInstances[entityID].get()->invokeMethod("Start", { });
	}
	void ScriptEngine::instanceOnUpdate(System::IObject::ID entityID, double dt)
	{
		if (scriptEngineData.scriptInstances.find(entityID) == scriptEngineData.scriptInstances.end()) return;
		scriptEngineData.scriptInstances[entityID].get()->invokeMethod("Update", {&dt});
	}
	void ScriptEngine::instanceOnLateUpdate(System::IObject::ID entityID, double dt)
	{
		if (scriptEngineData.scriptInstances.find(entityID) == scriptEngineData.scriptInstances.end()) return;
		scriptEngineData.scriptInstances[entityID].get()->invokeMethod("LateUpdate", { &dt });
	}
	void ScriptEngine::instanceOnFree(System::IObject::ID entityID)
	{
		if (scriptEngineData.scriptInstances.find(entityID) == scriptEngineData.scriptInstances.end()) return;
		scriptEngineData.scriptInstances[entityID].get()->invokeMethod("Free", { });
	}
	void ScriptEngine::instanceInvokeMethod(System::IObject::ID entityID, std::string methodName)
	{
		if (scriptEngineData.scriptInstances.find(entityID) == scriptEngineData.scriptInstances.end()) return;
		scriptEngineData.scriptInstances[entityID].get()->invokeMethod(methodName, { });
	}
	bool ScriptEngine::nodeExist(const std::string& fullClassName)
	{
		return (scriptEngineData.nodeClasses.find(fullClassName) != scriptEngineData.nodeClasses.end());
	}
	void ScriptEngine::getNodeClassName(std::vector<const char*>& resultVector) const
	{
		resultVector.reserve(scriptEngineData.nodeClasses.size());
		for (auto& className : scriptEngineData.nodeClassNameVector) {
			resultVector.emplace_back(className.c_str());
		}
	}
	void ScriptEngine::createNodeInstance(const System::IObject::ID& entityID, const AI::AINode::NodeID& nodeID, const std::string& fullClassName, const std::unordered_map<std::string, std::unique_ptr<void, std::function<void(void*)>>>& fieldNameToData)
	{
		if (nodeExist(fullClassName)) {
			scriptEngineData.nodeInstances[nodeID] = std::make_unique<NodeInstance>(scriptEngineData.nodeClasses[fullClassName].get(), entityID, nodeID, fieldNameToData);
			return;
		}
		AG_CORE_ERROR("{} doesnt match with the name in mono", fullClassName);
	}
	void ScriptEngine::nodeInstanceOnEnter(const AI::AINode::NodeID& nodeID)
	{
		if (scriptEngineData.nodeInstances.find(nodeID) == scriptEngineData.nodeInstances.end()) return;
		scriptEngineData.nodeInstances[nodeID].get()->invokeMethod("onEnter", { });
	}
	void ScriptEngine::nodeInstanceOnUpdate(const AI::AINode::NodeID& nodeID, double dt)
	{
		if (scriptEngineData.nodeInstances.find(nodeID) == scriptEngineData.nodeInstances.end()) return;
		scriptEngineData.nodeInstances[nodeID].get()->invokeMethod("onUpdate", { &dt });
	}
	void ScriptEngine::nodeInstanceOnExit(const AI::AINode::NodeID& nodeID)
	{
		if (scriptEngineData.nodeInstances.find(nodeID) == scriptEngineData.nodeInstances.end()) return;
		scriptEngineData.nodeInstances[nodeID].get()->invokeMethod("onExit", { });
	}
	char* ScriptEngine::readBytes(const std::string& assemblyPath, uint32_t* outSize)
	{
		std::ifstream stream(assemblyPath, std::ios::binary | std::ios::ate);

		if (!stream)
		{
			// Failed to open the file
			return nullptr;
		}

		std::streampos end = stream.tellg();
		stream.seekg(0, std::ios::beg);
		uint32_t size = static_cast<uint32_t>(end - stream.tellg());

		if (size == 0)
		{
			// File is empty
			return nullptr;
		}

		char* buffer = new char[size];
		stream.read((char*)buffer, size);
		stream.close();

		*outSize = size;
		return buffer;
	}
	MonoAssembly* ScriptEngine::loadCSharpAssembly(const std::string& assemblyPath)
	{
		uint32_t fileSize = 0;
		char* fileData = readBytes(assemblyPath, &fileSize);

		// NOTE: We can't use this image for anything other than loading the assembly because this image doesn't have a reference to the assembly
		MonoImageOpenStatus status;
		MonoImage* image = mono_image_open_from_data_full(fileData, fileSize, 1, &status, 0);

		if (status != MONO_IMAGE_OK)
		{
			//const char* errorMessage = mono_image_strerror(status);
			// Log some error message using the errorMessage data
			return nullptr;
		}

		MonoAssembly* assembly = mono_assembly_load_from_full(image, assemblyPath.c_str(), &status, 0);
		mono_image_close(image);

		// Don't forget to free the file data
		delete[] fileData;
		fileData = nullptr;
		// The pointer that returns metadata, functions etc
		return assembly;
	}
	void ScriptEngine::loadClasses()
	{
		const MonoTableInfo* typeDefinitionsTable = mono_image_get_table_info(scriptEngineData.scriptAssemblyImage, MONO_TABLE_TYPEDEF);
		int32_t numTypes = mono_table_info_get_rows(typeDefinitionsTable);

		scriptEngineData.nodeParentClass = ScriptClass("Script.Library", "INode");

		for (int32_t i = 0; i < numTypes; i++)
		{
			uint32_t cols[MONO_TYPEDEF_SIZE];
			mono_metadata_decode_row(typeDefinitionsTable, i, cols, MONO_TYPEDEF_SIZE);

			const char* nameSpace = mono_metadata_string_heap(scriptEngineData.scriptAssemblyImage, cols[MONO_TYPEDEF_NAMESPACE]);
			const char* name = mono_metadata_string_heap(scriptEngineData.scriptAssemblyImage, cols[MONO_TYPEDEF_NAME]);

			MonoClass* currentMonoClass = mono_class_from_name(scriptEngineData.scriptAssemblyImage, nameSpace, name);
			scriptEngineData.scriptParentClass = ScriptClass("Script.Library", "IScript");

			//AG_CORE_TRACE("{}.{}", nameSpace, name);


			//TODO: Check for nested struct/ class
			if (strlen(nameSpace) == 0) {
				//Debug for nested Type
				//AG_CORE_INFO("Script Nested Type: {}", name);
				continue;
			}
			else if (mono_class_is_subclass_of(currentMonoClass, scriptEngineData.scriptParentClass.getClass(), false)) {
				if (currentMonoClass != scriptEngineData.scriptParentClass.getClass()) {
					std::string nameSpaceString = nameSpace;
					std::string classNameString = name;
					std::string fullPath = nameSpaceString + "." + classNameString;
					//AG_CORE_INFO("Script: {}", fullPath);

					scriptEngineData.scriptEntityClasses[fullPath] = std::make_unique<ScriptClass>(nameSpaceString, classNameString);
					scriptEngineData.scriptClassNameVector.emplace_back(classNameString);
				}
			}
			else if (mono_class_is_subclass_of(currentMonoClass, scriptEngineData.nodeParentClass.getClass(), false)){
				//There will be error if there is nested struct/ class
				if (currentMonoClass != scriptEngineData.nodeParentClass.getClass()) {
					std::string nameSpaceString = nameSpace;
					std::string classNameString = name;
					std::string fullPath = nameSpaceString + "." + classNameString;
					//AG_CORE_INFO("Script: {}", fullPath);

					scriptEngineData.nodeClasses[fullPath] = std::make_unique<ScriptClass>(nameSpaceString, classNameString);
					scriptEngineData.nodeClassNameVector.emplace_back(classNameString);
				}
			}
		}
	}
	bool ScriptEngine::recompileGameScript()
	{
		const char* projectPath = "../../../GameScripts/GameScripts.csproj";
#ifdef _DEBUG
		const char* config = "Debug";
#elif EXPORT
		const char* config = "Export";
#else
		const char* config = "Release";
#endif
		std::string command = "dotnet build ";
		command += projectPath;
		command += " -c ";
		command += config;

		SECURITY_ATTRIBUTES sa{};
		sa.nLength = sizeof(sa);
		sa.bInheritHandle = TRUE;
		sa.lpSecurityDescriptor = NULL;

		HANDLE readPipe = NULL, writePipe = NULL;
		if (!CreatePipe(&readPipe, &writePipe, &sa, 0)) {
			AG_CORE_ERROR("Failed to create pipe");
			return false;
		}

		STARTUPINFOA si{};
		PROCESS_INFORMATION pi{};
		si.cb = sizeof(si);
		si.dwFlags |= STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
		si.wShowWindow = SW_HIDE;
		si.hStdOutput = writePipe;
		si.hStdError = writePipe;

		if (!CreateProcessA(
			nullptr,
			command.data(),
			nullptr, nullptr,
			TRUE, CREATE_NO_WINDOW,
			nullptr, nullptr,
			&si, &pi))
		{
			AG_CORE_ERROR("Failed to start build process");
			CloseHandle(writePipe);
			CloseHandle(readPipe);
			return false;
		}

		CloseHandle(writePipe);

		std::array<char, 256> buffer{};
		std::string output;
		DWORD bytesRead;
		while (ReadFile(readPipe, buffer.data(), (DWORD)buffer.size() - 1, &bytesRead, NULL) && bytesRead > 0) {
			buffer[bytesRead] = '\0';
			output += buffer.data();
		}

		CloseHandle(readPipe);
		WaitForSingleObject(pi.hProcess, INFINITE);
		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);

		bool success = output.find("Build succeeded.") != std::string::npos;
		bool haveWarning = output.find("0 Warning(s)") == std::string::npos;

		if (!success) {
			AG_CORE_ERROR(output);
		}
		else if (haveWarning) {
			AG_CORE_WARN(output);
		}
		else {
			AG_CORE_INFO(output);
		}
		return success;
	}
	void ScriptEngine::loadScriptAssembly()
	{
		// Create a new app domain
		// @param domainName: Name of the domain
		// @param configFile: Config file to use
		char domainName[] = "ProjectRuntime";
		scriptEngineData.appDomain = mono_domain_create_appdomain(domainName, nullptr);

		// Set the app domain as the active domain
		// @param domain: The domain to set as active
		// @param force: If true, the domain is set as the active domain even if it is not the root domain
		mono_domain_set(scriptEngineData.appDomain, true);
		scriptEngineData.apiAssembly = loadCSharpAssembly("ScriptCore.dll");
		scriptEngineData.apiassemblyImage = mono_assembly_get_image(scriptEngineData.apiAssembly);

		scriptEngineData.scriptAssembly = loadCSharpAssembly("GameScripts.dll");
		scriptEngineData.scriptAssemblyImage = mono_assembly_get_image(scriptEngineData.scriptAssembly);

		loadClasses();
	}
	void ScriptEngine::unloadScriptAssembly()
	{
		scriptEngineData.scriptInstances.clear();
		scriptEngineData.scriptEntityClasses.clear();
		scriptEngineData.scriptClassNameVector.clear();

		scriptEngineData.nodeInstances.clear();
		scriptEngineData.nodeClasses.clear();
		scriptEngineData.nodeClassNameVector.clear();

		mono_domain_set(mono_get_root_domain(), false);
		if (scriptEngineData.appDomain) {
			mono_domain_unload(scriptEngineData.appDomain);
			scriptEngineData.appDomain = nullptr;
		}
	}
	
}
