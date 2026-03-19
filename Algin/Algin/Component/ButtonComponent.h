#pragma once
#include "pch.h"

constexpr int STATE_HOVER = 0;
constexpr int STATE_PRESSED = 1;
constexpr int STATE_HELD = 2;
constexpr int STATE_RELEASED = 3;

constexpr int MOUSE_BUTTON_LEFT = 0;
constexpr int MOUSE_BUTTON_RIGHT = 1;
constexpr int MOUSE_BUTTON_MIDDLE = 2;

namespace AG {
    namespace Component {


        class ButtonComponent : public IComponent {
        public:
            using Self = ButtonComponent;
            static Type GetStaticType() { return Data::ComponentTypes::Button; }
            ButtonComponent() :
                IComponent(Data::ComponentTypes::Button)
            {
            }

            ButtonComponent(ID id, ID objId) :
                IComponent(Data::ComponentTypes::Button, id, objId)
            {
                auto obj = GetObj().lock();
                if (obj)
                {
                    if (obj->hasComponent<Component::Image2DComponent>())
                    {
                        COMPONENTMANAGER.CreateComponent(obj.get(), Data::ComponentTypes::Image2DComponent);
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

            std::string GetTypeName() const override { return "AG::Component::ButtonComponent"; }

            const std::array<bool, 4>& GetLeftStates() const { return L_buttonStates; }
            const std::array<bool, 4>& GetRightStates() const { return R_buttonStates; }
            const std::array<bool, 4>& GetMiddleStates() const { return M_buttonStates; }

            void SetLeftStates(const std::array<bool, 4>& v) { L_buttonStates = v; }
            void SetRightStates(const std::array<bool, 4>& v) { R_buttonStates = v; }
            void SetMiddleStates(const std::array<bool, 4>& v) { M_buttonStates = v; }

            REFLECT() {
                return 
                {

                };
            }

            REFLECT_SERIALIZABLE(ButtonComponent)

        private:
            // hover, pressed, held, released
            std::array<bool, 4> L_buttonStates = { false, false, false, false }; 
            std::array<bool, 4> R_buttonStates = { false, false, false, false };
            std::array<bool, 4> M_buttonStates = { false, false, false, false };
        };

    }
} // namespace
