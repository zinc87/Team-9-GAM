/*!*****************************************************************************
\file ObjectFactory.h
\author Zulfami Ashrafi Bin Wakif
\date 17/2/2025 (MM/DD/YYYY)

\brief Object Factory to create object
*******************************************************************************/
#pragma once
#include "../Header/pch.h"
#include "../Data/SerializationSystem.h"
#include "../Pattern/ICommand.h"

namespace AG {
	namespace System {
		class ObjectManager : public Pattern::ISingleton<ObjectManager> {
		public:
			using ID = IObject::ID;
			using IDList = std::vector<ID>;
			using IObjectPtr = std::shared_ptr<IObject>;
			using IObjectMap = std::unordered_map<ID, std::shared_ptr<IObject>>;
			friend class SerializationSystem;

		public:
			ObjectManager();
			// Temp
			IObjectPtr		CreatePatient(std::string name = "", bool addInList = true, bool raw = false);

			IObjectPtr		CreateObject(std::string name = "", bool addInList = true, bool raw = false);
			IObjectPtr		DuplicateObject			(IObjectPtr source, bool changeName = true);
			IObjectPtr		DuplicateObjectRaw		(IObjectPtr source);
			IObjectPtr		GetObjectByID			(ID id) const;
			IObjectPtr		GetRoot() { return m_root; }
			IObjectPtr CreateObjectWithID(const std::string& name, const ID& id, bool addInList = true, bool raw = false);

			void			RemoveObject			(ID id);
			void			FlushDeletedObjects		();
			void			LinkParentChild			(IObjectPtr parent, IObjectPtr child);
			void			UnlinkParent			(IObjectPtr child);
			void			ReorderChild			(IObjectPtr parent, IObjectPtr child, size_t insertIndex);

			IObjectPtr		GetObjectById			(ID id);

			IObjectMap&    GetAllObjects			();
			void ClearScene();

		private:
			IObjectMap m_objects;
			IObjectPtr m_root;

			IDList m_objectsToDelete;
		};

		class CreateObjectCommand : public Pattern::ICommand {
		public:
			CreateObjectCommand() {}

			void Execute() override {
				if (m_object) {
					m_object = ObjectManager::GetInstance().CreateObject(m_object->GetName());
				}
				else {
					m_object = ObjectManager::GetInstance().CreateObject();
				}
			}

			void Undo() override {
				ObjectManager::GetInstance().RemoveObject(m_object->GetID());
			}

		private:
			std::shared_ptr<IObject> m_object = nullptr;
			
		};

		class DeleteObjectCommand : public Pattern::ICommand {
		public:
			DeleteObjectCommand(std::shared_ptr<IObject> obj)
				: m_toDelete{ obj },
				m_savedObject{ ObjectManager::GetInstance().DuplicateObjectRaw(obj) },
				m_originalParent{ obj->GetParent() } {} 

			void Execute() override {
				ObjectManager::GetInstance().RemoveObject(m_toDelete->GetID());
			}

			void Undo() override {
				// Recreate object from stored copy
				auto restored = ObjectManager::GetInstance().DuplicateObject(m_savedObject);

				// Restore parent link
				if (m_originalParent)
					ObjectManager::GetInstance().LinkParentChild(m_originalParent, restored);
				else
					ObjectManager::GetInstance().LinkParentChild(ObjectManager::GetInstance().GetRoot(), restored);
			}

		private:
			std::shared_ptr<IObject> m_toDelete = nullptr;
			std::shared_ptr<IObject> m_savedObject = nullptr;
			std::shared_ptr<IObject> m_originalParent = nullptr;
		};

	#define OBJECTMANAGER AG::System::ObjectManager::GetInstance()
	}
}
