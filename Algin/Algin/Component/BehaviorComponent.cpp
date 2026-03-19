#include "pch.h"
#include "BehaviorComponent.h"
#include "../BHT/AIManager.h"

namespace AG {
    namespace Component {

        void BehaviorComponent::Awake() {
            //Create Tree instance at AI manager
            std::optional<AI::BehaviorTreeTemplate> treeTemplate = AI::AIManager::GetInstance().getTemplate(treeName);
            if (!treeTemplate) {
                AG_CORE_WARN("No Template: {}", treeName);
                return;
            }
            std::shared_ptr<AI::BehaviorTree> tree = std::make_shared<AI::BehaviorTree>(treeTemplate.value(), m_objId);
            AI::AIManager::GetInstance().registerTreeInstance(tree);
            treeID = tree->getID();
        }

        void BehaviorComponent::Start() {
        }

        void BehaviorComponent::Update() {
            std::weak_ptr<AI::BehaviorTree> treeInstance = AI::AIManager::GetInstance().getTree(treeID);
            if (treeInstance.expired()) {
                return;
            }
            treeInstance.lock()->Run(static_cast<float>(BENCHMARKER.GetDeltaTime()));
            
        }

        void BehaviorComponent::LateUpdate() {
            // Debugging, visualization, etc.
        }

        void BehaviorComponent::Free() {
        }

        void BehaviorComponent::Inspector() {
            //Test code
            if (ImGui::Button("Test Build Tree")) {
                std::vector<AI::NodeRep> nodeVec{};
                nodeVec.emplace_back(AI::NodeType::CONTROL_FLOW, 0, "Selector");
                nodeVec.emplace_back(AI::NodeType::DECORATOR, 1, "PreDelay");
                nodeVec.emplace_back(AI::NodeType::LEAF, 2, "Leaf1");
                /*nodeVec.emplace_back(AI::NodeType::DECORATOR, 1, "Delay");
                nodeVec.emplace_back(AI::NodeType::CONTROL_FLOW, 2, "Sequencer");
                nodeVec.emplace_back(AI::NodeType::CONTROL_FLOW, 3, "Selector");
                nodeVec.emplace_back(AI::NodeType::DECORATOR, 4, "Delay");
                nodeVec.emplace_back(AI::NodeType::LEAF, 5, "Leaf2");
                nodeVec.emplace_back(AI::NodeType::DECORATOR, 4, "Delay");
                nodeVec.emplace_back(AI::NodeType::LEAF, 5, "Leaf3");
                nodeVec.emplace_back(AI::NodeType::LEAF, 1, "Idle");*/
                

                AI::TreeBuilder::GetInstance().buildTree(std::move(nodeVec), "Assets/Tree/Test.bht");
            }
            ImGui::SameLine();
            if (ImGui::Button("Test Retrieve Tree")) {
                std::vector<AI::NodeRep> nodeVec = AI::TreeBuilder::GetInstance().getTree("Assets/Tree/Test.bht");
                for (auto& node : nodeVec) {
                    AG_CORE_TRACE("Node Name: {}. Type: {}. Depth: {}", node.name, (int)node.nodeType, node.depth);
                }
            }
            RenderFieldsInspector(this, GetFields());

        }

        void BehaviorComponent::AssignFrom(const std::shared_ptr<IComponent>& fromCmp) {
            ASSIGNFROM_FN_BODY(
                this->treeName = fromCmpPtr->treeName;
                )
        }

        std::vector<const char*> BehaviorComponent::getClassName()
        {
            std::vector<const char*> result{};
            AI::AIManager::GetInstance().getClassName(result);
            return result;
        }

        const AI::BehaviorTree::TreeID& BehaviorComponent::getTreeID()
        {
            return treeID;
        }

    }
}

REGISTER_REFLECTED_TYPE(AG::Component::BehaviorComponent)
