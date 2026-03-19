#pragma once
#include "../Header/pch.h"
#include "../Graphics/Light.h"
#include "../Data/Reflection.h"

namespace AG {
    namespace Component {

        class LightComponent : public IComponent {
        public:
            using Self = LightComponent;

            glm::vec3 r_position{ 0.0f,  0.0f,  0.0f };
            glm::vec3 r_direction{ 0.0f, -1.0f,  0.0f };
            float     r_range = 10.0f;
            float     r_intensity = 1.0f;
            float     r_fov = 100.f;
            float     r_innerCos = 0.f;
            float     r_outerCos = 0.f;
            int       r_type_index = 0; // maps to LightType enum

            // --- ADD THIS STATIC FUNCTION ---
            static Type GetStaticType() { return Data::ComponentTypes::Light; }

			LightComponent() :
				IComponent(Data::ComponentTypes::Light) 
			{
				light = std::make_shared<Light>();
				LightManager::GetInstance().addLight(light);
				SyncToLight();
			}

			LightComponent(ID id, ID objId) :
				IComponent(Data::ComponentTypes::Light, id, objId) 
			{
				light = std::make_shared<Light>();
				LightManager::GetInstance().addLight(light);
				SyncToLight();
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

            static constexpr uint32_t LIGHT_DATA_HASH = 0x3456789A;

            std::string GetTypeName() const override { return "AG::Component::LightComponent"; }

            REFLECT() {
                return {
                    REFLECT_FIELD(r_position,   glm::vec3, "Position"),
                    REFLECT_FIELD(r_direction,  glm::vec3, "Direction"),
                    REFLECT_FIELD_RANGE(r_intensity, float, "Intensity", "", 0.0f, 100.0f, 0.1f),
                    REFLECT_FIELD_RANGE(r_range,     float, "Range",     "", 0.0f, 500.0f, 0.1f),
                    REFLECT_FIELD_RANGE(r_fov,       float, "Cone FOV",  "", 0.0f, 120.f, 0.1f),
                    REFLECT_FIELD_LIST(r_type_index, int,  "LightType",  "", Self::GetTypeItems)
                };
            }


            bool Serialize(std::ostream& out) const override;
            bool Deserialize(std::istream& in) override;

            // provider for the type combo
            static std::vector<const char*> GetTypeItems() {
                static std::vector<const char*> items = { "Point", "Spot" };
                return items;
            }

            void ComputeCos() {

                float outerAngle = glm::radians(r_fov) * 0.5f;
                float innerAngle = outerAngle * 0.85f;

                r_outerCos = std::cos(outerAngle);
                r_innerCos = std::cos(innerAngle);
            }

        
            void SyncToLight();    // push mirrors -> runtime
            void SyncFromLight();  // pull runtime -> mirrors (rarely needed)

            std::shared_ptr<Light> light;

        private:
        };

    }
} // namespace
