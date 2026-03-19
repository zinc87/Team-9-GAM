/*!*****************************************************************************
\file ObjectFactory.cpp
\author Zulfami Ashrafi Bin Wakif
\date 17/2/2025 (MM/DD/YYYY)

\brief Object Factory to create object
*******************************************************************************/
#pragma once
#include "pch.h"

namespace AG {
	namespace System {
		ObjectManager::ObjectManager()
		{
			m_root = std::make_shared<IObject>("__root__");
			m_root->SetID(Data::GUIDZero()); // Reserved ID
		}

		std::shared_ptr<IObject> ObjectManager::CreateObject(std::string name, bool addInList, bool raw)
		{
			// === If no name is provided, find highest "GameObject ##" and increment ===
			if (name.empty())
			{
				int maxIndex = 0;

				for (const auto& [id, obj] : m_objects)
				{
					const std::string& objName = obj->GetName();
					const std::string prefix = "GameObject ";

					if (objName.rfind(prefix, 0) == 0) // starts with "GameObject "
					{
						try {
							int num = std::stoi(objName.substr(prefix.length()));
							if (num > maxIndex) maxIndex = num;
						}
						catch (...) {
							// Skip malformed names like "GameObject ABC"
						}
					}
				}

				name = "GameObject " + std::to_string(maxIndex + 1);
			}

			auto obj = std::make_shared<IObject>(name);
			ID newID = Data::GenerateGUID();
			obj->SetID(newID);

			if (addInList) {
				m_objects[newID] = obj;
				if (!raw)
				{
					COMPONENTMANAGER.CreateComponent(obj.get(), Data::ComponentTypes::Transform);
				}
				// Automatically parent to root
				LinkParentChild(m_root, obj);
			}

			return obj;
		}

		// Temp
		std::shared_ptr<IObject> ObjectManager::CreatePatient(std::string name, bool addInList, bool raw)
		{
			// === If no name is provided, find highest "GameObject ##" and increment ===
			if (name.empty())
			{
				int maxIndex = 0;

				for (const auto& [id, obj] : m_objects)
				{
					const std::string& objName = obj->GetName();
					const std::string prefix = "GameObject ";

					if (objName.rfind(prefix, 0) == 0) // starts with "GameObject "
					{
						try {
							int num = std::stoi(objName.substr(prefix.length()));
							if (num > maxIndex) maxIndex = num;
						}
						catch (...) {
							// Skip malformed names like "GameObject ABC"
						}
					}
				}

				name = "GameObject " + std::to_string(maxIndex + 1);
			}

			auto obj = std::make_shared<IObject>(name);
			ID newID = Data::GenerateGUID();
			obj->SetID(newID);

			if (addInList) {
				m_objects[newID] = obj;
				if (!raw)
				{
					COMPONENTMANAGER.CreateComponent(obj.get(), Data::ComponentTypes::Transform);
					COMPONENTMANAGER.CreateComponent(obj.get(), Data::ComponentTypes::SkinnedMeshRenderer);
				}
				// Automatically parent to root
				LinkParentChild(m_root, obj);
			}

			return obj;
		}

		std::shared_ptr<IObject> ObjectManager::DuplicateObject(IObjectPtr source, bool changeName)
		{
			if (!source) return nullptr;

			// Create new object with incremented name
			std::string newName = source->GetName();
			if (changeName) {
			newName += " Copy";
			}

			std::shared_ptr<IObject> clone = CreateObject(newName);

			// Optionally: preserve type
			clone->SetType(source->GetType());

			// === Duplicate components ===
			const ComponentManager::WCompVec& compVec = COMPONENTMANAGER.GetComponentsObj().at(source->GetID());
			for (const auto& comp : compVec) {
				COMPONENTMANAGER.CreateComponent(clone.get(), comp);
			}

			// === Optionally: deep clone children ===
			for (const auto& child : source->GetChildren())
			{
				auto duplicatedChild = DuplicateObject(child);
				LinkParentChild(clone, duplicatedChild);
			}

			// === Parent it under the same parent (if any) ===
			if (auto parent = source->GetParent())
			{
				LinkParentChild(parent, clone);
			}
			else
			{
				// if root-level, attach to invisible root
				LinkParentChild(GetRoot(), clone);
			}

			return clone;
		}

		ObjectManager::IObjectPtr ObjectManager::DuplicateObjectRaw(IObjectPtr source)
		{
			if (!source) return nullptr;

			std::shared_ptr<IObject> clone = CreateObject(source->Name(), false);

			// Optionally: preserve type
			clone->SetType(source->GetType());

			// === Duplicate components ===
			/*for (const auto& [type, compID] : source->GetComponentsType())
			{
				auto comp = ComponentManager::GetInstance().GetComponentByID(type, compID);
				if (comp)
				{
					auto clonedComp = comp->Clone();
					ComponentManager::GetInstance().AttachComponentToObject(clone, clonedComp);
				}
			}*/

			// === Optionally: deep clone children ===
			//for (const auto& child : source->GetChildren())
			//{
			//	auto duplicatedChild = DuplicateObjectRaw(child);
			//	LinkParentChild(clone, duplicatedChild);
			//}

			//// === Parent it under the same parent (if any) ===
			//if (auto parent = source->GetParent())
			//{
			//	LinkParentChild(parent, clone);
			//}
			//else
			//{
			//	// if root-level, attach to invisible root
			//	LinkParentChild(GetRoot(), clone);
			//}

			return clone;
		}

		std::shared_ptr<IObject> ObjectManager::GetObjectByID(ID id) const
		{
			auto it = m_objects.find(id);
			return it != m_objects.end() ? it->second : nullptr;
		}

		void ObjectManager::RemoveObject(ID id)
		{
			auto obj = GetObjectByID(id);
			if (!obj) return;

			// Avoid duplicates
			if (std::find(m_objectsToDelete.begin(), m_objectsToDelete.end(), id) != m_objectsToDelete.end())
				return;
			 
			m_objectsToDelete.push_back(id);

			// Queue all descendants too
			for (auto& child : obj->GetDescendants())
			{
				if (child)
				{
					ID childID = child->GetID();
					if (std::find(m_objectsToDelete.begin(), m_objectsToDelete.end(), childID) == m_objectsToDelete.end())
					{
						m_objectsToDelete.push_back(childID);
					}
				}
			}

		}

		void ObjectManager::FlushDeletedObjects()
		{
			for (auto id : m_objectsToDelete)
			{
				auto obj = GetObjectByID(id);
				if (!obj) continue;

				// === Remove attached components first ===
				auto& objMap = COMPONENTMANAGER.GetComponentsObj();
				auto it = objMap.find(id);
				if (it != objMap.end()) {
					for (auto& weak : it->second) {
						if (auto comp = weak.lock()) {
							COMPONENTMANAGER.RemoveComponent(comp->GetID());
						}
					}
				}

				// === Unlink from parent ===
				if (auto parent = obj->GetParent())
				{
					auto& siblings = parent->m_children;
					siblings.erase(std::remove_if(siblings.begin(), siblings.end(),
						[&](const std::weak_ptr<IObject>& weak)
						{
							auto child = weak.lock();
							return child && child->GetID() == id;
						}), siblings.end());
				}

				// === Finally remove object itself ===
				m_objects.erase(id);
			}

			m_objectsToDelete.clear();

			// Important: flush component deletions immediately
			COMPONENTMANAGER.FlushDeletedComponent();
		}

		void ObjectManager::LinkParentChild(IObjectPtr parent, IObjectPtr child)
		{
			if (!parent || !child) return;

			// Don't parent to self
			if (parent == child) return;

			// Prevent cycles (no parenting to descendant)
			auto descendants = child->GetDescendants();
			if (std::find(descendants.begin(), descendants.end(), parent) != descendants.end())
				return;

			// If already parented to this object, do nothing
			if (child->GetParent() == parent)
				return;

			// Remove from previous parent's child list
			if (auto oldParent = child->GetParent()) {
				auto& siblings = oldParent->m_children;
				siblings.erase(std::remove_if(siblings.begin(), siblings.end(),
					[&](const std::weak_ptr<IObject>& weak) {
						auto shared = weak.lock();
						return shared == child;
					}), siblings.end());
			}

			// Add to new parent
			parent->m_children.push_back(child);
			child->SetParent(parent);
		}

		void ObjectManager::UnlinkParent(IObjectPtr child) {
			if (!child) return;

			auto oldParent = child->GetParent();
			if (oldParent) {
				auto& siblings = oldParent->m_children;
				siblings.erase(std::remove_if(siblings.begin(), siblings.end(),
					[&](const std::weak_ptr<IObject>& weak) {
						return weak.lock() == child;
					}), siblings.end());
			}

			child->SetParent(m_root);            //  Reparent to invisible root
			m_root->m_children.push_back(child); //  Add to root's children
		}

		void ObjectManager::ReorderChild(IObjectPtr parent, IObjectPtr child, size_t insertIndex)
		{
			if (!parent || !child) return;

			// Get vector of children as raw shared_ptrs
			auto& allChildren = parent->m_children;

			// Remove from current position
			auto it = std::remove_if(allChildren.begin(), allChildren.end(), [&](const std::weak_ptr<IObject>& wp) {
				return wp.lock() == child;
				});
			allChildren.erase(it, allChildren.end());

			// Re-insert at new position
			if (insertIndex > allChildren.size())
				insertIndex = allChildren.size();

			allChildren.insert(allChildren.begin() + insertIndex, child);
			child->SetParent(parent);
		}
		ObjectManager::IObjectPtr ObjectManager::GetObjectById(ID id)
		{
			auto it = m_objects.find(id);
			if (it != m_objects.end()) {
				return it->second;
			}
			return {};
		}

		ObjectManager::IObjectMap& ObjectManager::GetAllObjects(){
			return m_objects;
		}

		void ObjectManager::ClearScene()
		{
			// 1. Queue EVERY object except the root for deletion
			for (auto& [id, obj] : m_objects)
			{
				if (id == Data::GUIDZero())
					continue;   // do NOT delete root

				RemoveObject(id); // queue it
			}

			// 2. Recreate a clean root object
			m_root = std::make_shared<IObject>("__root__");
			m_root->SetID(Data::GUIDZero());

			// 3. Reset object map to contain only the root
			m_objects.clear();
			m_objects[Data::GUIDZero()] = m_root;
		}

		std::shared_ptr<IObject> ObjectManager::CreateObjectWithID(const std::string& name, const ID& id, bool addInList, bool raw)
		{
			std::string finalName = name.empty() ? "GameObject" : name;

			auto obj = std::make_shared<IObject>(finalName);
			ID newID = AG::Data::IsZero(id) ? AG::Data::GenerateGUID() : id;
			obj->SetID(newID);

			if (addInList) {
				m_objects[newID] = obj;
				if (!raw)
					COMPONENTMANAGER.CreateComponent(obj.get(), AG::Data::ComponentTypes::Transform);
				LinkParentChild(m_root, obj);
			}

			return obj;
		}
	}
}
