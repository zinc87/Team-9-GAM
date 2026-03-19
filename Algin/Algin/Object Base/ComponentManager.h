/*!*****************************************************************************
\file ComponentManagers.h
\author Zulfami Ashrafi Bin Wakif
\date 17/2/2025 (MM/DD/YYYY)

\brief Manager all the component
*******************************************************************************/
#pragma once
#include "../Header/pch.h"
#include "../Data/SerializationSystem.h"

namespace AG {
	namespace System {
		class ComponentManager : public Pattern::ISingleton<ComponentManager> {
		public:
			using ID =			Data::GUID;
			using IDList =		std::vector<ID>;
			using CType =		Data::ComponentTypes;

			using CompPtr =		std::shared_ptr<Component::IComponent>;
			using WeakCompPtr = std::weak_ptr<Component::IComponent>;

			using CompVec =		std::vector<CompPtr>;
			using WCompVec =	std::vector<WeakCompPtr>;

			using IDMap =		std::unordered_map<
										ID,
										CompPtr,
										AG::Data::GUIDHasher,
										AG::Data::GUIDEqual
								>;										// Strong ownership
			using ObjMap =		std::unordered_map<
										ID,
										WCompVec,
										AG::Data::GUIDHasher,
										AG::Data::GUIDEqual
								>;										// Weak Ref
			using CompMap =		std::unordered_map<CType, WCompVec>;	// Weak Ref

			using FMap =		std::unordered_map<CType, std::function<CompPtr(ID, ID)>>; // Factory: (componentId, objectId)
		public:
			ComponentManager();
			~ComponentManager();

			void Start();
			void Update();
			void LateUpdate();
			void RenderGraphicsOnly();

			Component::IComponent*  CreateComponent             (IObject* obj, CType type); //Default Construct
			Component::IComponent*	CreateComponent				(IObject* obj, ComponentManager::WeakCompPtr fromPtr); //Copy Construct Component
			CompPtr                 CreateUserOwnedComponent    (CType type);
			CompPtr                 CloneComponent              (ComponentManager::WeakCompPtr fromPtr); //Clone and user own component
			CompPtr					GetComponentByID			(ID id) const;
			void					RemoveComponent				(ID id);
			void					FlushDeletedComponent		();

			/// <summary>
			/// Add the component T into the list of component
			/// </summary>
			/// <typeparam name="T"> The type that is derive from IComponent </typeparam>
			template<typename T>
			void AddToList() {
				static_assert(std::is_base_of<Component::IComponent, T>::value, "T must derive IComponent");

				// --- THIS IS THE FIX ---
				// We now get the type from the static function instead of creating an object.
				m_componentFactory[T::GetStaticType()] = [](ID componentId, ID objId) -> CompPtr {
					return std::make_shared<T>(componentId, objId);
					};
			}

			// === Accessor === //
			const IDMap&	GetComponents		() const;
			const CompMap&	GetComponentsType	() const;
			const ObjMap&	GetComponentsObj	() const;
			const FMap&		GetFactories		() const;

			void ClearAllComponents();

		private:
			// Map with ID to component
			IDMap m_idIndex;

			// Components sorted by type
			CompMap m_componentsByType;

			// Components sorted by obj
			ObjMap m_componentsByObj;

			// Component factory ptr
			FMap m_componentFactory;

			// List of component to delete
			IDList m_ComponentsToDelete;
		};
	}
#define COMPONENTMANAGER AG::System::ComponentManager::GetInstance()
}
