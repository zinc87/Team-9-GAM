/*!*****************************************************************************
\file TransformComponent.h
\author Zulfami Ashrafi Bin Wakif
\date 17/2/2025 (MM/DD/YYYY)

\brief Transform Component for the objects
*******************************************************************************/
#pragma once
#include "../Header/pch.h"

struct Transformation
{
	glm::vec3 position;
	glm::vec3 scale;
	glm::vec3 rotation; // x,y,z

	Transformation() : position{ 0.f,0.f,0.f }, scale{ 1.f,1.f,1.f }, rotation{ 0.f,0.f,0.f } {}
	glm::mat4 getM2W()
	{
		glm::mat4 M(1.0f);

		M = glm::translate(M, position);

		M = glm::rotate(M, glm::radians(rotation.z), glm::vec3(0.f, 0.f, 1.f));
		M = glm::rotate(M, glm::radians(-rotation.y), glm::vec3(0.f, 1.f, 0.f));
		M = glm::rotate(M, glm::radians(rotation.x), glm::vec3(1.f, 0.f, 0.f));

		M = glm::scale(M, scale);

		return M;
	}
};


namespace AG {
	namespace Component {
		class TransformComponent : public IComponent {
		public:
			// 1. Declare this class type for reflection
			using Self = TransformComponent;

			// --- ADD THIS STATIC FUNCTION ---
			static Type GetStaticType() { return Data::ComponentTypes::Transform; }

			TransformComponent() : 
				IComponent(Data::ComponentTypes::Transform) {}

			TransformComponent(ID id, ID objId) :
				IComponent(Data::ComponentTypes::Transform, id, objId) {}

			// Inherited via IComponent
			void Awake() override;

			void Start() override;

			void Update() override;

			void LateUpdate() override;

			void Free() override;

			void Inspector() override;

			//virtual std::shared_ptr<IComponent> Clone() override;
			void AssignFrom(const std::shared_ptr<IComponent>& fromCmp) override;

			// === Debug === //
			void PrintData();

			Transformation& GetTransformation() { return m_trf; }

			glm::mat4 getM2W() {
				// check if there is a parent
				auto obj = GetObj().lock();
				if (!obj) return m_trf.getM2W();

				auto parentObj = obj->GetParent();
				if (!parentObj) return m_trf.getM2W();

				auto parentTrf = parentObj->GetComponent<Component::TransformComponent>().lock();
				if (!parentTrf) return m_trf.getM2W();

				if (parentObj && parentTrf) {
					return parentTrf->getM2W()* m_trf.getM2W();
				}

				return m_trf.getM2W();
			}

			glm::mat3 getNormalMat() {
				glm::mat3 normalMat = glm::transpose(glm::inverse(getM2W()));
				return normalMat;
			}

			glm::vec3 GetPosiion() {
				auto obj = GetObj().lock();
				if (!obj) return m_trf.position;

				auto parentObj = obj->GetParent();
				if (!parentObj) return m_trf.position;

				auto parentTrf = parentObj->GetComponent<Component::TransformComponent>().lock();
				if (!parentTrf) return m_trf.position;

				if (parentObj && parentTrf) {
					return parentTrf->m_trf.position + m_trf.position;
				}

				return m_trf.position;
			}

			glm::vec3 GetRot() {
				auto obj = GetObj().lock();
				if (!obj) return m_trf.rotation;

				auto parentObj = obj->GetParent();
				if (!parentObj) return m_trf.rotation;

				auto parentTrf = parentObj->GetComponent<Component::TransformComponent>().lock();
				if (!parentTrf) return m_trf.rotation;

				if (parentObj && parentTrf) {
					return parentTrf->m_trf.rotation + m_trf.rotation;
				}

				return m_trf.position;
			}

			glm::vec3 GetScale() {
				auto obj = GetObj().lock();
				if (!obj) return m_trf.scale;

				auto parentObj = obj->GetParent();
				if (!parentObj) return m_trf.scale;

				auto parentTrf = parentObj->GetComponent<Component::TransformComponent>().lock();
				if (!parentTrf) return m_trf.scale;

				if (parentObj && parentTrf) {
					return parentTrf->m_trf.scale + m_trf.scale;
				}

				return m_trf.scale;
			}

			// ---------------------------------------------------------------------
			// Direction vectors (global/world space)
			// ---------------------------------------------------------------------

			// Forward (+Z in your engine)
			glm::vec3 GetForward() {
				glm::mat4 M = getM2W();
				// In GLM: forward is +Z (third column)
				glm::vec3 forward = glm::normalize(glm::vec3(M[2]));
				return forward;
			}

			// Backward (-Z)
			glm::vec3 GetBackward() {
				return -GetForward();
			}

			// Right (+X)
			glm::vec3 GetRight() {
				glm::mat4 M = getM2W();
				glm::vec3 right = glm::normalize(glm::vec3(M[0]));
				return right;
			}

			// Left (-X)
			glm::vec3 GetLeft() {
				return -GetRight();
			}

			// Up (+Y)
			glm::vec3 GetUp() {
				glm::mat4 M = getM2W();
				glm::vec3 up = glm::normalize(glm::vec3(M[1]));
				return up;
			}

			// Down (-Y)
			glm::vec3 GetDown() {
				return -GetUp();
			}


			std::string GetTypeName() const override { return "AG::Component::TransformComponent"; }

			// 2. Define the fields to be reflected
			REFLECT() {
				return {
					REFLECT_FIELD(m_trf.position, glm::vec3, "Position"),
					REFLECT_FIELD(m_trf.rotation, glm::vec3, "Rotation"),
					REFLECT_FIELD(m_trf.scale,    glm::vec3, "Scale")
				};
			}

			REFLECT_SERIALIZABLE(TransformComponent)

		private:
			Transformation m_trf;
		};
	}
}
