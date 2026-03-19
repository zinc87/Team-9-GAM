/*!*****************************************************************************
\file IObject.inl
\author Zulfami Ashrafi Bin Wakif
\date 17/2/2025 (MM/DD/YYYY)

\brief Interface class for IObject
*******************************************************************************/
#pragma once
#include "ComponentManager.h" // safe to include here
#include "IObject.h"

namespace AG {
	namespace System {
		template<typename T>
		inline std::weak_ptr<T> IObject::GetComponent()
		{
			const auto& objMap = COMPONENTMANAGER.GetComponentsObj();
			auto it = objMap.find(m_id);
			if (it == objMap.end())
				return {}; // no components for this object

			for (auto& wk : it->second) {
				if (auto comp = wk.lock()) {
					// Try cast to desired type
					if (auto casted = std::dynamic_pointer_cast<T>(comp)) {
						return casted; // weak_ptr<T>
					}
				}
			}

			return {}; // not found
		}
		template<typename T>
		inline bool IObject::hasComponent()
		{
			const ComponentManager::ObjMap& objMap = COMPONENTMANAGER.GetComponentsObj();
			if (objMap.empty()) return false;
			auto it = objMap.find(m_id);
			if (it == objMap.end()) return false;
			for (auto& weakCompPtr : it->second) {
				if (auto compPtr = weakCompPtr.lock()) {
					if (auto casted = std::dynamic_pointer_cast<T>(compPtr)) {
						return true;
					}
				}
			}
			return false;
		}
	}
}
