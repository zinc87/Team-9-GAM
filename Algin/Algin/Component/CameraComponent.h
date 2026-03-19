#pragma once
#include "../Header/pch.h"
#include "../Graphics/Camera.h"

namespace AG {
	namespace Component {
		struct CameraRay
		{
			glm::vec3 origin;
			glm::vec3 direction;
		};
		class CameraComponent : public IComponent {
		public:

			// --- ADD THIS STATIC FUNCTION ---
			static Type GetStaticType() { return Data::ComponentTypes::Camera; }


			using Self = CameraComponent;

			CameraComponent() :
				IComponent(Data::ComponentTypes::Camera) 
			{
				m_camera = std::make_shared<Camera>();

				// initialize reflected values from camera
				SyncToCamera();
			}

			CameraComponent(ID id, ID objId) :
				IComponent(Data::ComponentTypes::Camera, id, objId) 
			{
				m_camera = std::make_shared<Camera>();

				// initialize reflected values from camera
				SyncToCamera();
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

			static constexpr uint32_t CAMERA_TYPE_HASH = 0x11110001;
			static constexpr uint32_t CAMERA_NEAR_HASH = 0x11110002;
			static constexpr uint32_t CAMERA_FAR_HASH = 0x11110003;
			static constexpr uint32_t CAMERA_FOV_HASH = 0x11110004;
			static constexpr uint32_t CAMERA_VERTICAL_HASH = 0x11110005;
			static constexpr uint32_t CAMERA_GAME_HASH = 0x11110006;

			// Serialization
			bool Serialize(std::ostream& out) const override;

			bool Deserialize(std::istream& in) override;

			void PrintData();

			std::string GetTypeName() const override { return "AG::Component::CameraComponent"; }

			// reflected GUI-only copies (synced to/from m_camera)
			float reflected_near = 0.1f;
			float reflected_far = 50.0f;
			float reflected_fov = 60.0f;
			float reflected_vertical = 10.0f;
			int reflected_proj = (int)PROJECTION_TYPE::PERSPECTIVE;
			bool reflected_isGameCam = false;

			CameraRay GetForwardRay();
			bool drawDebugRay = false;
			float debugRayLength = 50.0f;


			REFLECT() {
				return {
					REFLECT_FIELD(reflected_proj, int, "Projection"),
					REFLECT_FIELD(reflected_near, float, "Near Plane"),
					REFLECT_FIELD(reflected_far, float, "Far Plane"),
					REFLECT_FIELD(reflected_fov, float, "FOV"),
					REFLECT_FIELD(reflected_vertical, float, "Vertical Size"),
					REFLECT_FIELD(reflected_isGameCam, bool, "Game Camera")
				};
			}

			static std::vector<const char*> GetProjectionItems() {
				static std::vector<std::string> names = { "Orthographic", "Perspective" };
				static std::vector<const char*> ptrs;
				ptrs.clear();
				for (auto& s : names) ptrs.push_back(s.c_str());
				return ptrs;
			}

			// helper sync methods 
			void SyncFromCamera();
			void SyncToCamera();
			std::shared_ptr<Camera> GetCameraPtr() const { return m_camera; }

		private:
			std::shared_ptr<Camera> m_camera;
			
		};
	}
}
