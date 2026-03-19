#pragma once
#include "../Header/pch.h"
#include "../Graphics/BatchRenderer.h"

namespace AG {
    namespace Component {


        class RectTransformComponent : public IComponent {
        public:
            using Self = RectTransformComponent;
            static Type GetStaticType() { return Data::ComponentTypes::RectTransform; }
            RectTransformComponent() :
                IComponent(Data::ComponentTypes::RectTransform)
            {}

            RectTransformComponent(ID id, ID objId) :
                IComponent(Data::ComponentTypes::RectTransform, id, objId)
            {
                auto obj = GetObj().lock();
                if (obj)
                {
                    if (obj->hasComponent<Component::TransformComponent>())
                    {
                        COMPONENTMANAGER.RemoveComponent(obj->GetID());
                    }
                }
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

            std::string GetTypeName() const override { return "AG::Component::RectTransformComponent"; }

            REFLECT() {
                return {
                    REFLECT_FIELD(m_trf.position, glm::vec3, "Position"),
                    REFLECT_FIELD(m_trf.scale,    glm::vec2, "Scale"),
                    REFLECT_FIELD(m_trf.rotation, float,     "Rotation"),
                };
            }
            Transform2D m_trf;
            REFLECT_SERIALIZABLE(RectTransformComponent)



			Transform2D& GetTransform() { return m_trf; }
        private:
            /*Transform2D m_trf;*/
        };

    }
} // namespace
