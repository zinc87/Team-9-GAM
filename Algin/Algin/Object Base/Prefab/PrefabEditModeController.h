#pragma once

#include "pch.h"

namespace AG {
	class PrefabEditModeController : public Pattern::ISingleton<PrefabEditModeController> {
	public:
		enum class ExitMode {
			ApplyToCurrent,
			ApplyToAll
		};

		void setNewObjID(const std::shared_ptr<System::IObject>& newObj); //this check whenever there is new selected object and determine whether to on or off the prefab edit mode
		void enterEditMode();
		void exitEditMode(ExitMode exitMode = ExitMode::ApplyToCurrent);
		void applyBeforeState();
		bool isActive();
	private:
		std::weak_ptr<System::IObject> activeObj;
		bool modeActive = false;

		System::jsonDoc beforeState;
		System::jsonDoc afterState;
	};
}