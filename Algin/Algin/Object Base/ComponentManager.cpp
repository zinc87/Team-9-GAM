/*!*****************************************************************************
\file ComponentManagers.cpp
\author Zulfami Ashrafi Bin Wakif
\date 17/2/2025 (MM/DD/YYYY)

\brief Manager all the component
*******************************************************************************/
#pragma once
#include "pch.h"
namespace AG {
	namespace System {
		ComponentManager::ComponentManager() {
			AddToList<Component::TransformComponent>();
			AddToList<Component::LightComponent>();
			AddToList<Component::CameraComponent>();
			AddToList<Component::AudioComponent>();
			AddToList<Component::MeshRendererComponent>();
			AddToList<Component::SkyBoxComponent>();
			AddToList<Component::BoxColliderComponent>();
			AddToList<Component::CapsuleColliderComponent>();
			AddToList<Component::PostProcessVolumeComponent>();
			AddToList<Component::RigidBodyComponent>();
			AddToList<Component::RectTransformComponent>();
			AddToList<Component::Image2DComponent>();
			AddToList<Component::ScriptComponent>();
			AddToList<Component::TextMeshUIComponent>();
			AddToList<Component::ParticleComponent>();
			AddToList<Component::SkinnedMeshRenderer>();
			AddToList<Component::BehaviorComponent>();
			//AddToList<Component::PrefabComponent>();
			AddToList<Component::CubeComponent>();
			AddToList<Component::ButtonComponent>();
			AddToList<Component::VideoPlayerComponent>();
		}

		ComponentManager::~ComponentManager() {
			// Clear all the created components
			for (auto& [type, list] : m_componentsByType) {
				list.clear();
			}

			m_componentsByType.clear();
			m_componentsByObj.clear();
			m_componentFactory.clear();
		}

		void ComponentManager::Start() {
			// Check if engine is running
			if (!SCENESTATEMANAGER.IsSceneRunning()) return;

			// Awake (only once)
			for (auto& [type, list] : m_componentsByType) {
				for (auto& weak : list) {
					if (auto comp = weak.lock()) {   // safely promote to shared_ptr
						if (!comp->IsInit()) {
							comp->Awake();

							AG_CORE_INFO(comp->GetObj().lock()->Name() + " " +
								Data::ComponentTypeToString(type) +
								" Awake");

							SCENESTATEMANAGER.AddFuncOnEnd(
								[comp]() {
									comp->IsInit(false);
								});

							comp->IsInit(true);
						}
					}
				}
			}

			// Start (only once) and mark initialized
			for (auto& [type, list] : m_componentsByType) {
				for (auto& weak : list) {
					if (auto comp = weak.lock()) {
						auto owner = comp->GetObj().lock();
						if (owner && !comp->IsStart() && comp->IsEnable() && owner->IsEnable()) {
							comp->Start();

							AG_CORE_INFO(comp->GetObj().lock()->Name() + " " +
								Data::ComponentTypeToString(type) +
								" Started");

							SCENESTATEMANAGER.AddFuncOnEnd(
								[comp]() {
									comp->IsStart(false);
								});

							comp->IsStart(true);
						}
					}
				}
			}
		}

		void ComponentManager::Update() {
			// temp start
			Start();

			// Check if engine is running
			if (!SCENESTATEMANAGER.IsSceneRunning()) {
				// Things to render when engine is not running
				for (auto& [type, list] : m_componentsByType)
				{
					if (type == Data::ComponentTypes::Transform ||
						type == Data::ComponentTypes::RectTransform ||
						type == Data::ComponentTypes::Image2DComponent ||
						type == Data::ComponentTypes::Camera ||
						type == Data::ComponentTypes::Light ||
						type == Data::ComponentTypes::MeshRenderer ||
						type == Data::ComponentTypes::SkyBox ||
						type == Data::ComponentTypes::SkinnedMeshRenderer||
						type == Data::ComponentTypes::Behavior ||
						type == Data::ComponentTypes::BoxCollider)  {
						for (auto& wk : list) {
							if (auto sp = wk.lock()) {
								auto owner = sp->GetObj().lock();
								if (!owner || !owner->GetActive() || !sp->IsEnable()) continue;
								sp->Update();
							}
						}
					}
					else {
						continue;
					}

					
				}
			}
			else {
				// Update all components
				for (auto& [type, list] : m_componentsByType) {
					for (auto& wk : list) {
						if (auto sp = wk.lock()) {
							auto owner = sp->GetObj().lock();
							if (!owner || !owner->GetActive() || !sp->IsEnable()) continue;
							sp->Update();
						}
					}
				}
			}

			FlushDeletedComponent();
		}

		void ComponentManager::LateUpdate() {
			// temp start
			Start();

			// Check if engine is running
			if (!SCENESTATEMANAGER.IsSceneRunning()) {
				// Things to render when engine is not running
				for (auto& [type, list] : m_componentsByType)
				{
					if (type == Data::ComponentTypes::Transform ||
						type == Data::ComponentTypes::RectTransform ||
						type == Data::ComponentTypes::Image2DComponent ||
						type == Data::ComponentTypes::Camera ||
						type == Data::ComponentTypes::Light ||
						type == Data::ComponentTypes::MeshRenderer ||
						type == Data::ComponentTypes::SkyBox ||
						type == Data::ComponentTypes::TextMeshUI ||
						type == Data::ComponentTypes::TextMesh ||
						type == Data::ComponentTypes::SkinnedMeshRenderer ||
						type == Data::ComponentTypes::Behavior ||
						type == Data::ComponentTypes::BoxCollider)  {
						for (auto& wk : list) {
							if (auto sp = wk.lock()) {
								auto owner = sp->GetObj().lock();
								if (!owner || !owner->GetActive() || !sp->IsEnable()) continue;
								sp->LateUpdate();
							}
						}
					}
					else {
						continue;
					}

					
				}
			}
			else {
				// LateUpdate all components
				for (auto& [type, list] : m_componentsByType) {
					for (auto& wk : list) {
						if (auto sp = wk.lock()) {
							auto owner = sp->GetObj().lock();
							if (!owner || !owner->GetActive() || !sp->IsEnable()) continue;
							sp->LateUpdate();
						}
					}
				}
			}

			FlushDeletedComponent();
		}

		void ComponentManager::RenderGraphicsOnly()
		{
			// Ensure components are initialized
			Start();

			// Only render graphics-related components (skip scripts to avoid double updates)
			const CType renderTypes[] = {
				Data::ComponentTypes::SkyBox,
				Data::ComponentTypes::MeshRenderer,
				Data::ComponentTypes::SkinnedMeshRenderer,
				Data::ComponentTypes::Particles
			};

			for (CType type : renderTypes)
			{
				auto it = m_componentsByType.find(type);
				if (it == m_componentsByType.end())
					continue;

				for (auto& wk : it->second)
				{
					if (auto sp = wk.lock())
					{
						auto owner = sp->GetObj().lock();
						if (!owner || !owner->GetActive() || !sp->IsEnable()) continue;
						sp->LateUpdate();
					}
				}
			}

			FlushDeletedComponent();
		}

		Component::IComponent* ComponentManager::CreateComponent(IObject* obj, CType type) {
			// Find the factory
			auto it = m_componentFactory.find(type);
			if (it == m_componentFactory.end()) {
				return nullptr;
			}

			//If alr have that component, function wont do anything
			if (obj->hasComponent(type)) { 
				return nullptr; 
			}

			// Generate new ID
			ID newID = Data::GenerateGUID();

			// Construct the component
			CompPtr comp = it->second(newID, obj->GetID());

			// track it by component by ID
			m_idIndex.emplace(newID, comp);

			// Store it in the type registry
			m_componentsByType[type].push_back(comp);

			// Also track it by object if you use m_componentsByObj
			m_componentsByObj[obj->GetID()].push_back(comp);

			// Return raw pointer for immediate use (caller should NOT own it)
			return comp.get();
		}

		Component::IComponent* ComponentManager::CreateComponent(IObject* obj, ComponentManager::WeakCompPtr fromPtr) {
			if (fromPtr.expired()) { return nullptr; }
			
			// Find the factory
			auto it = m_componentFactory.find(fromPtr.lock()->GetType());
			if (it == m_componentFactory.end()) {
				return nullptr;
			}

			//If alr have that component, function will assign the value to the current ptr instead
			if (obj->hasComponent(fromPtr.lock()->GetType())) { 
				WeakCompPtr existComp = obj->GetComponent(fromPtr.lock()->GetType());
				existComp.lock()->AssignFrom(fromPtr.lock());
				return existComp.lock().get();
			}

			// Generate new ID
			ID newID = Data::GenerateGUID();

			// Construct the component
			CompPtr comp = it->second(newID, obj->GetID());
			comp->AssignFrom(fromPtr.lock());

			// track it by component by ID
			m_idIndex.emplace(newID, comp);

			// Store it in the type registry
			m_componentsByType[fromPtr.lock()->GetType()].push_back(comp);

			// Also track it by object if you use m_componentsByObj
			m_componentsByObj[obj->GetID()].push_back(comp);

			// Return raw pointer for immediate use (caller should NOT own it)
			return comp.get();
		}

		ComponentManager::CompPtr ComponentManager::CreateUserOwnedComponent(ComponentManager::CType type)
		{
			// Find the factory
			auto it = m_componentFactory.find(type);
			if (it == m_componentFactory.end()) {
				return nullptr;
			}

			ID newID = Data::GenerateGUID();
			CompPtr comp = it->second(newID, Data::GUIDZero());

			return comp;
		}

		ComponentManager::CompPtr ComponentManager::CloneComponent(ComponentManager::WeakCompPtr fromPtr) {
			//Clone all the data to a component pointer(owned by user not the component manager), alter only the ID
			//This function at first used for prefab component where i need to copy all the data as prefab
			if (fromPtr.expired()) { return nullptr; }
			auto it = m_componentFactory.find(fromPtr.lock()->GetType());
			if (it == m_componentFactory.end()) {
				return nullptr;
			}
			if (fromPtr.expired()) {
				return nullptr;
			}

			ID newID = Data::GenerateGUID();
			CompPtr comp = it->second(newID, Data::GUIDZero());
			comp->AssignFrom(fromPtr.lock());
			return comp;
		}
		
		ComponentManager::CompPtr ComponentManager::GetComponentByID(ID id) const {
			if (auto it = m_idIndex.find(id); it != m_idIndex.end())
				return it->second;
			return {};
		}

		void ComponentManager::RemoveComponent(ID id) {
			auto comp = GetComponentByID(id);
			if (!comp) return;

			// Avoid duplicates
			if (std::find(m_ComponentsToDelete.begin(), m_ComponentsToDelete.end(), id) != m_ComponentsToDelete.end())
				return;

			m_ComponentsToDelete.push_back(id);

		}

		void ComponentManager::FlushDeletedComponent() {
			if (m_ComponentsToDelete.empty())
				return;

			// Dedupe the deletion queue
			std::sort(m_ComponentsToDelete.begin(), m_ComponentsToDelete.end());
			m_ComponentsToDelete.erase(
				std::unique(m_ComponentsToDelete.begin(), m_ComponentsToDelete.end()),
				m_ComponentsToDelete.end()
			);

			// Helper: erase-from vector<weak_ptr<...>> by component ID (or expired)
			auto erase_by_id = [](WCompVec& vec, ID dead) {
				vec.erase(std::remove_if(vec.begin(), vec.end(),
					[dead](const WeakCompPtr& wk) {
						auto sp = wk.lock();
						return !sp || sp->GetID() == dead;
					}), vec.end());
				};

			for (ID id : m_ComponentsToDelete) {
				// Pull strong owner; if not present, nothing to do
				auto it = m_idIndex.find(id);
				if (it == m_idIndex.end())
					continue;

				CompPtr comp = std::move(it->second);
				m_idIndex.erase(it);

				if (!comp)
					continue;

				comp->Free();

				const CType type = comp->GetType();
				const ID   ownerId = comp->GetObjID(); // ensure IComponent exposes this

				// Prune from per-type weak view
				if (auto byType = m_componentsByType.find(type); byType != m_componentsByType.end())
					erase_by_id(byType->second, id);

				// Prune from per-object weak view
				if (auto byObj = m_componentsByObj.find(ownerId); byObj != m_componentsByObj.end())
					erase_by_id(byObj->second, id);
			}

			m_ComponentsToDelete.clear();
		}
		  
		const ComponentManager::IDMap& ComponentManager::GetComponents() const
		{
			return m_idIndex;
		}

		const ComponentManager::CompMap& ComponentManager::GetComponentsType() const
		{
			return m_componentsByType;
		}

		const ComponentManager::ObjMap& ComponentManager::GetComponentsObj() const
		{
			return m_componentsByObj;
		}

		const ComponentManager::FMap& ComponentManager::GetFactories() const
		{
			return m_componentFactory;
		}

		void ComponentManager::ClearAllComponents()
		{
			// Queue every component for deletion
			for (auto& [id, comp] : m_idIndex)
			{
				if (comp)
					RemoveComponent(id);   // <--- USE QUEUE SYSTEM
			}
		}
	}
}
