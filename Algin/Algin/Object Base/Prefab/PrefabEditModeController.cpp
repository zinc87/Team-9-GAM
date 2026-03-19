#include "pch.h"
#include "PrefabEditModeController.h"

namespace AG {
	void PrefabEditModeController::setNewObjID(const std::shared_ptr<System::IObject>& newObj) {
		//Not active and choose the prefab instance -> enter mode
		//Not active and choose normal object -> do nth
		//Active and choose the same prefab instance -> do nth
		//Active and choose other prefab instance -> ->exit the current one and enter on the other prefab instance
		//Active and choose normal object -> exit mode

		if (!newObj) return;

		if (!modeActive && newObj->hasComponent(Data::ComponentTypes::Prefab)) {
			activeObj = newObj;
			modeActive = true;
			enterEditMode();
		}
		else if (modeActive) {
			if (OBJECTMANAGER.GetObjectByID(activeObj.lock()->GetID()) == nullptr) { //if the active prefab instance is deleted
				activeObj.reset();
				modeActive = false;
				AG_CORE_INFO("Exit Editor mode (deleted object)");
				setNewObjID(newObj);
			}
			else if (!newObj->hasComponent(Data::ComponentTypes::Prefab)) { //choose normal object
				exitEditMode();
			}
			else if (newObj->GetID() != activeObj.lock()->GetID()) { //Choose other prefab instance
				exitEditMode();
				activeObj = newObj;
				enterEditMode();
			}
		}
		
	}
	void PrefabEditModeController::enterEditMode()
	{
		beforeState.SetObject();
		rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator>& allocator = beforeState.GetAllocator();
		rapidjson::Value prefabInst{ rapidjson::kObjectType };
		const System::ComponentManager::WCompVec& compVec = COMPONENTMANAGER.GetComponentsObj().at(activeObj.lock()->GetID());
		SERIALIZATIONSYSTEM.serializeRootCmp(allocator, prefabInst, compVec);
		beforeState.AddMember("Override", prefabInst, allocator);
		AG_CORE_INFO("Enter Editor mode");
	}
	void PrefabEditModeController::exitEditMode(ExitMode exitMode)
	{
		switch (exitMode)
		{
		case ExitMode::ApplyToAll:
			//apply to the template
			//Iterate and apply to object
			break;
		default:
		case ExitMode::ApplyToCurrent:
			afterState.SetObject();
			rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator>& allocator = afterState.GetAllocator();
			rapidjson::Value prefabInst{ rapidjson::kObjectType };
			const System::ComponentManager::WCompVec& compVec = COMPONENTMANAGER.GetComponentsObj().at(activeObj.lock()->GetID());
			SERIALIZATIONSYSTEM.serializeRootCmp(allocator, prefabInst, compVec);
			afterState.AddMember("Override", prefabInst, allocator);

			std::weak_ptr<Component::PrefabComponent> prefabCmp = activeObj.lock()->GetComponent<Component::PrefabComponent>();
			System::jsonDoc& overrideDoc = prefabCmp.lock()->getPrefabOverride();
			System::jsonDoc resultDoc;
			SERIALIZATIONSYSTEM.getOverrideSerializer().CreatePrefabOverride(beforeState, afterState, overrideDoc, resultDoc);
			prefabCmp.lock()->setPrefabOverride(resultDoc);
			break;
		}
		
		activeObj.reset();
		modeActive = false;
		AG_CORE_INFO("Exit Editor mode");
	}
	void PrefabEditModeController::applyBeforeState()
	{

	}
	bool PrefabEditModeController::isActive()
	{
		return modeActive;
	}
}