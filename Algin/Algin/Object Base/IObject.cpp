/*!*****************************************************************************
\file IObject.cpp
\author Zulfami Ashrafi Bin Wakif
\date 18/2/2025 (MM/DD/YYYY)

\brief Test Scene
*******************************************************************************/
#pragma once
#include "pch.h"

namespace AG {
    namespace System {
		IObject::IObjectSVector IObject::GetChildren() const {
			IObjectSVector out;
			for (const auto& weak : m_children) {
				if (auto strong = weak.lock()) {
					out.push_back(strong);
				}
			}
			return out;
		}

		std::vector<std::shared_ptr<IObject>> IObject::GetDescendants() const {
			IObjectSVector result;
			for (auto& child : GetChildren()) {
				result.push_back(child);
				auto sub = child->GetDescendants();
				result.insert(result.end(), sub.begin(), sub.end());
			}
			return result;
		}

		IObject::IObjectSPtr IObject::GetParent() const {
			return m_parent.lock();
		}

		void IObject::SetName(const std::string& name) {
			m_name = name;
		}

		void IObject::SetParent(const IObjectSPtr& parent) {
			m_parent = parent;
		}

		void IObject::AddChild(const IObjectSPtr& child) {
			m_children.push_back(child);
			child->SetParent(shared_from_this());
		}

		void IObject::SetActive(bool active)
		{
			m_enable = active;
			
			for (auto& weak : m_children) {
				if (auto child = weak.lock()) {
					child->IsEnable() = active;
				}
			}
		}

		bool IObject::GetActive() const
		{
			return m_enable;
		}

		IObject::WCompVec IObject::GetComponentsType() {
			auto& objMap = COMPONENTMANAGER.GetComponentsObj();
			auto it = objMap.find(m_id);
			if (it != objMap.end()) {
				return it->second;  // copy the vector of weak_ptrs
			}
			return {};              // return empty if no entry
		}

		IObject::WeakCompPtr IObject::GetComponent(Data::ComponentTypes type) {
			const auto& objMap = COMPONENTMANAGER.GetComponentsObj();
			auto it = objMap.find(m_id);
			if (it == objMap.end())
				return {}; // no components for this object

			for (auto& weak : it->second) {
				if (auto comp = weak.lock()) {
					if (comp->GetType() == type)
						return weak;  // return the weak reference itself
				}
			}

			return {}; // none found
		}

		bool IObject::hasComponent(Data::ComponentTypes type) {
			const auto& objMap = COMPONENTMANAGER.GetComponentsObj();
			if (objMap.empty() || objMap.find(m_id) == objMap.end()) return false;
			for (auto& comp : objMap.at(m_id)) {
				if (!comp.expired() && comp.lock()->GetType() == type) {
					return true;
				}
			}
			return false;
		}

		bool& IObject::IsEnable()
		{
			// Reset Start Flag for all component
			if (m_enable == false) {
				const auto& objMap = COMPONENTMANAGER.GetComponentsObj();
				auto it = objMap.find(m_id);
				if (it == objMap.end())
					return m_enable;

				for (auto& weak : it->second) {
					if (auto comp = weak.lock()) {
						comp->IsStart(false);
					}
				}
			}

			for (auto& weak : m_children) {
				if (auto child = weak.lock()) {
					child->IsEnable() = m_enable;
				}
			}

			return m_enable;
		}


		void IObject::SetType(const Type& type)
		{
			m_type = type;
		}
    }
}
