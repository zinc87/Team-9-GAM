#pragma once
#include "../Header/pch.h"
#include "../Graphics/DebugRenderer.h"
#include "../Graphics/Cube.h"

namespace AG {
	namespace Component {
		class CubeComponent : public IComponent {
		public:
			static Type GetStaticType() { return Data::ComponentTypes::Cube; }

			using Self = CubeComponent;

			CubeComponent() : IComponent(Data::ComponentTypes::Cube) 
			{
				m_cube = std::make_shared<MaterialObj>();
				CubeRenderer::GetInstance().Setup();
				m_cube->params = MaterialSetup::GetInstance().matParams.params;
				m_cube->albedoHashID = 0;
				m_cube->normalMapHashID = 0;
			}

			CubeComponent(ID id, ID objId) : IComponent(Data::ComponentTypes::Cube, id, objId)
			{
				m_cube = std::make_shared<MaterialObj>();
				CubeRenderer::GetInstance().Setup();
				m_cube->params = MaterialSetup::GetInstance().matParams.params;
				m_cube->albedoHashID = 0;
				m_cube->normalMapHashID = 0;
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

			void SyncToCube();

			std::string GetTypeName() const override { return "AG::Component::CubeComponent"; }

			REFLECT() {
				return {
				};
			}

			REFLECT_SERIALIZABLE(CubeComponent)

		private:
			std::shared_ptr<MaterialObj> m_cube;

			glm::vec3 r_position{ 0.0f,  0.0f,  0.0f };
			glm::vec3 r_scale{ 1.f, 1.f, 1.f };
			glm::vec3 r_rotation = { 1.f, 1.f, 1.f };
			//MaterialParams r_params = CubeRenderer::GetInstance().matParams;
			matAsset r_m_asset;
		};
	}
}