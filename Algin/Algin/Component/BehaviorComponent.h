#pragma once
#include "pch.h"
#include "IComponent.h"
#include "../BHT/AIManager.h"

namespace AG {
    namespace Component {

        class BehaviorComponent : public IComponent {
        public:
            using Self = BehaviorComponent;
            BehaviorComponent() : IComponent(Data::ComponentTypes::Behavior) {}
            BehaviorComponent(ID id, ID objId) : IComponent(Data::ComponentTypes::Behavior, id, objId) {}

            static Type GetStaticType() { return Data::ComponentTypes::Behavior; }

            // === Lifecycle === //
            void Awake() override;
            void Start() override;
            void Update() override;
            void LateUpdate() override;
            void Free() override;
            void Inspector() override;
            void AssignFrom(const std::shared_ptr<IComponent>& fromCmp) override;

            std::string GetTypeName() const override { return "AG::Component::BehaviorComponent"; }

            static std::vector<const char*> getClassName();

            REFLECT() {
                return {
                    REFLECT_FIELD_LIST(treeName, std::string, "Behavior Tree Name", "", getClassName),
                };
            }

            const AI::BehaviorTree::TreeID& getTreeID();

            REFLECT_SERIALIZABLE(BehaviorComponent)

        private:
            std::string treeName;
            AI::BehaviorTree::TreeID treeID;
            std::unordered_map <AI::AINode::NodeID, std::vector<FieldInfo>> nodeSerializeFieldInfo; //Unseed for now
        };
    }
}
