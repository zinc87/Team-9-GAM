#pragma once
#include "pch.h"

namespace AG {
	class ScriptClass {
	public:
		ScriptClass() : nameSpace{}, className{}, monoClass{ nullptr }, fields{} {}
		ScriptClass(const std::string& newNameSpace, const std::string& newClassName);
		
		MonoClass* getClass();
		std::string getClassName();
		MonoObject* instantiate();
		MonoMethod* getMethod(std::string functionName, int paramCount);
		void invokeMethod(MonoMethod* method, uint32_t gchandle, void** param);
		std::unordered_map<std::string, ScriptField>& getFields();
	private:
		void reflectMonoFields();
	private:
		std::string nameSpace;
		std::string className;
		std::unordered_map<std::string, ScriptField> fields;
		MonoClass* monoClass;
	};

	class ScriptInstance {
	public:
		ScriptInstance(ScriptClass* newScriptClass, System::IObject::ID entityID, const std::unordered_map<std::string, std::unique_ptr<void, std::function<void(void*)>>>& fieldNameToData);
		void invokeMethod(std::string methodName, std::vector<void*> param);
	private:
		ScriptClass* scriptClass;
		uint32_t gcHandle;
		MonoObject* instance = nullptr;
		std::unordered_map<std::string, MonoMethod*> nameToMethod;
	};

	class NodeInstance {
	public:
		NodeInstance(ScriptClass* newNodeClass, const System::IObject::ID& entityID, const AI::AINode::NodeID& nodeID, const std::unordered_map<std::string, std::unique_ptr<void, std::function<void(void*)>>>& fieldNameToData);
		void invokeMethod(std::string methodName, std::vector<void*> param);
	private:
		ScriptClass* nodeClass;
		uint32_t gcHandle;
		MonoObject* instance = nullptr;
		std::unordered_map<std::string, MonoMethod*> nameToMethod;
	};

	struct ScriptEngineData
	{
		MonoDomain* rootDomain = nullptr;
		MonoDomain* appDomain = nullptr;

		MonoAssembly* apiAssembly = nullptr;
		MonoImage* apiassemblyImage = nullptr;

		MonoAssembly* scriptAssembly = nullptr;
		MonoImage* scriptAssemblyImage = nullptr;

		ScriptClass scriptParentClass;
		ScriptClass nodeParentClass;

		std::unordered_map<std::string, std::unique_ptr<ScriptClass>> scriptEntityClasses;
		std::unordered_map<System::IObject::ID, std::unique_ptr<ScriptInstance>> scriptInstances;
		std::vector<std::string> scriptClassNameVector;

		std::unordered_map<std::string, std::unique_ptr<ScriptClass>> nodeClasses;
		std::unordered_map<AI::AINode::NodeID, std::unique_ptr<NodeInstance>> nodeInstances;
		std::vector<std::string> nodeClassNameVector;
	};

	class ScriptEngine : public Pattern::ISingleton<ScriptEngine>{
	public:
		void init();
		void update();
		void terminate();

		MonoImage* getApiImage();
		MonoDomain* getAppDomain();
		MonoImage* getScriptImage();
		ScriptClass getScriptParentClass();
		ScriptClass getNodeParentClass();
		ScriptConnector& getScriptConnector();
		std::unordered_map<std::string, ScriptField> getFields(std::string fullClassName);
		void reloadAssembly();
		void registerFileWatch();

		bool scriptExist(const std::string& fullClassName);
		void getClassName(std::vector<const char*>& resultVector) const;
		void createInstance(System::IObject::ID entityID, const std::string& fullClassName, const std::unordered_map<std::string, std::unique_ptr<void, std::function<void(void*)>>>& fieldNameToData);
		void instanceOnStart(System::IObject::ID entityID);
		void instanceOnUpdate(System::IObject::ID entityID, double dt);
		void instanceOnLateUpdate(System::IObject::ID entityID, double dt);
		void instanceOnFree(System::IObject::ID entityID);
		void instanceInvokeMethod(System::IObject::ID entityID, std::string methodName);

		bool nodeExist(const std::string& fullClassName);
		void getNodeClassName(std::vector<const char*>& resultVector) const;
		void createNodeInstance(const System::IObject::ID& entityID, const AI::AINode::NodeID& nodeID, const std::string& fullClassName, const std::unordered_map<std::string, std::unique_ptr<void, std::function<void(void*)>>>& fieldNameToData);
		void nodeInstanceOnEnter(const AI::AINode::NodeID& nodeID);
		void nodeInstanceOnUpdate(const AI::AINode::NodeID& nodeID, double dt);
		void nodeInstanceOnExit(const AI::AINode::NodeID& nodeID);
	private:
		char* readBytes(const std::string& assemblyPath, uint32_t* outSize);
		MonoAssembly* loadCSharpAssembly(const std::string& assemblyPath);
		void loadClasses();
		bool recompileGameScript();
		void loadScriptAssembly();
		void unloadScriptAssembly();
	private:
		ScriptEngineData scriptEngineData;
		ScriptConnector scriptConnector;

		std::atomic_bool toReload = false;

		friend class ScriptClass;
	};
}

#define	SCRIPTENGINE AG::ScriptEngine::GetInstance()