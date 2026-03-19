/*!*****************************************************************************
\file TransformComponent.cpp
\author Zulfami Ashrafi Bin Wakif
\date 17/2/2025 (MM/DD/YYYY)

\brief Transform Component for the objects
*******************************************************************************/
#pragma once
#include "pch.h"

namespace AG {
	namespace Component {
		void TransformComponent::Awake()
		{
		}
		void TransformComponent::Start()
		{
		}
		void TransformComponent::Update()
		{
		}
		void TransformComponent::LateUpdate()
		{
		}
		void TransformComponent::Free()
		{
		}

		void TransformComponent::Inspector()
		{
			// 3. Use the utility function to render the fields
			AG::RenderFieldsInspector(this, Self::GetFields());
		}

		void TransformComponent::AssignFrom(const std::shared_ptr<IComponent>& fromCmp) {
			ASSIGNFROM_FN_BODY(
				m_trf = fromCmpPtr->m_trf;
			)
		}

		void TransformComponent::PrintData() {
			AG_CORE_INFO("Component ID: {}", GetID());
			AG_CORE_INFO("Attached Object ID: {}", m_objId);
			AG_CORE_INFO("Attached Object ID: {}", m_objId);
			AG_CORE_INFO("Position: {}, {}, {}", m_trf.position.x, m_trf.position.y, m_trf.position.z);
			AG_CORE_INFO("Scale: {}, {}, {}", m_trf.scale.x, m_trf.scale.y, m_trf.scale.z);
			AG_CORE_INFO("Rotation: {}, {}, {}", m_trf.rotation.x, m_trf.rotation.y, m_trf.rotation.z);
		}

		
	}
}

// 4. Register the type in the registry (source file only)
REGISTER_REFLECTED_TYPE(AG::Component::TransformComponent)
