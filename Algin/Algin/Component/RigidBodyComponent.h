#pragma once
#include "../Header/pch.h"
#include <PxPhysicsAPI.h>

namespace AG {
	namespace Component {
		class RigidBodyComponent : public IComponent {
		public:
			struct RigidBodyData {
				glm::vec3 drag{ 0.0f,0.0f, 0.0f };
				glm::vec3 velocity{ 0.0f,0.0f, 0.0f };
				glm::vec3 acceleration{ 0.0f,0.0f, 0.0f };
				glm::vec3 force{ 0.0f,0.0f, 0.0f };
				float mass{ 1.0f };
				glm::vec3 gravity{ 0.0f, -9.81f, 0.0f };
				bool useGravity{ false };
				glm::vec3 finalAcceleration{ 0.0f,0.0f, 0.0f };
			};

			using Self = RigidBodyComponent;

			// --- ADD THIS STATIC FUNCTION ---
			static Type GetStaticType() { return Data::ComponentTypes::RigidBody; }

			RigidBodyComponent() :
				IComponent(Data::ComponentTypes::RigidBody) {
			}

			RigidBodyComponent(ID id, ID objId) :
				IComponent(Data::ComponentTypes::RigidBody, id, objId) {
			}



			// Inherited via IComponent
			void Awake() override;

			void Start() override;

			void Update() override;

			void LateUpdate() override;

			void Free() override;

			void Inspector() override;

			//virtual std::shared_ptr<IComponent> Clone() override;
			void AssignFrom(const std::shared_ptr<IComponent>& fromCmp) override;


			void PrintData();

			std::string GetTypeName() const override { return "AG::Component::RigidBodyComponent"; }

			REFLECT() {
				return {
					REFLECT_FIELD(_data.drag,             glm::vec3, "Drag"),
					REFLECT_FIELD(_data.velocity,         glm::vec3, "Velocity"),
					REFLECT_FIELD(_data.acceleration,     glm::vec3, "Acceleration"),
					REFLECT_FIELD(_data.force,            glm::vec3, "Force"),
					REFLECT_FIELD(_data.mass,             float,     "Mass"),
					REFLECT_FIELD(_data.gravity,          glm::vec3, "Gravity"),
					REFLECT_FIELD(_data.useGravity,       bool,      "Use Gravity")
				};
			}

			REFLECT_SERIALIZABLE(RigidBodyComponent)

			physx::PxRigidDynamic* GetPxActor() const { return pxActor; }

			float GetMass() const { return _data.mass; }
			bool GetUseGravity() const { return _data.useGravity; }

			void SetSimulationEnabled(bool enabled);
			bool IsSimulationEnabled() const { return m_simEnabled; }

			void SetFrozen(bool frozen);
			bool IsFrozen() const { return m_frozen; }

			RigidBodyData _data;

		private:
			
			bool m_simEnabled = true;
			bool m_frozen = false;
						// --- PhysX handles ---
			physx::PxRigidDynamic* pxActor = nullptr;
			physx::PxMaterial* pxMaterial = nullptr;

		};
	}
}
