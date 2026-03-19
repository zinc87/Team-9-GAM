/*!*****************************************************************************
\file IObject.h
\author Zulfami Ashrafi Bin Wakif
\date 17/2/2025 (MM/DD/YYYY)

\brief Interface class for Object
*******************************************************************************/
#pragma once
#include "../Header/pch.h"

namespace AG {
	// Forward Declared
	namespace Component {
		class IComponent;
	}

	namespace System {
		class IObject : public std::enable_shared_from_this<IObject> {
		public:
			friend class ObjectManager;
			using Type = Data::ObjectTypes;
			using ID = ::AG::Data::GUID;

			using IObjectSPtr = std::shared_ptr<IObject>;
			using IObjectWPtr = std::weak_ptr<IObject>;

			using IObjectSVector = std::vector<IObjectSPtr>;
			using IObjectWVector = std::vector<IObjectWPtr>;

			using CVector = std::vector<Component::IComponent>;

			using WeakCompPtr = std::weak_ptr<Component::IComponent>;
			using WCompVec = std::vector<WeakCompPtr>;
			using ObjCompMap = std::unordered_map<ID, WCompVec>;

		public:
			IObject(std::string name = "")
				: m_name{ std::move(name) }, m_type{ Data::ObjectTypes::None }, m_id{ Data::GenerateGUID() } {}

			// === Hierarchy === //
			IObjectSVector	GetChildren() const;
			IObjectSVector	GetDescendants() const;
			IObjectSPtr		GetParent() const;

			void SetName	(const std::string& name);
			void SetParent	(const IObjectSPtr& parent);
			void SetType	(const Type& type);
			void AddChild	(const IObjectSPtr& child);

			void SetActive	(bool active);
			bool GetActive	() const;

			// === Component === //
			WCompVec GetComponentsType();
			WeakCompPtr GetComponent(Data::ComponentTypes type);

			template<typename T>
			std::weak_ptr<T> GetComponent();

			template <typename T>
			bool hasComponent();
			bool hasComponent(Data::ComponentTypes type);

			// === Accessors === //
			IObjectSPtr GetSelf()		{ return shared_from_this(); }

			std::string GetName()	const { return m_name; }
			Type		GetType()	const { return m_type; }
			ID			GetID()		const { return m_id; }

			void SetID(ID id) { m_id = id; }

			std::string& Name() { return m_name; }
			bool& IsEnable();

		private:
			std::string m_name;
			Type m_type;
			ID m_id;

			bool m_enable = true;

			IObjectWPtr		m_parent;
			IObjectWVector	m_children;
		};
	}
}

#include "IObject.inl"