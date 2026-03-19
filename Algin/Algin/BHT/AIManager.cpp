/*!*****************************************************************************
\file AIManager.cpp
\author Peh Jun Wei Gabriel
\date 11/7/2025
\brief
    Implementation for the global AIManager singleton.
*******************************************************************************/
#include "pch.h"
#include "AIManager.h"

namespace AG {
    namespace AI {
        void AIManager::registerTreeInstance(std::shared_ptr<BehaviorTree>& newTree)
        {
            trees[newTree->getID()] = newTree;
        }

        std::weak_ptr<BehaviorTree> AIManager::getTree(BehaviorTree::TreeID treeID)
        {
            if (trees.find(treeID) != trees.end()) {
                return trees[treeID];
            }
            AG_CORE_WARN("Tree not stored in AI Manager {}", treeID);
            return std::weak_ptr<BehaviorTree>();
        }

        std::weak_ptr<AI::AINode> AIManager::getNode(BehaviorTree::TreeID treeID, AI::AINode::NodeID nodeID)
        {
            std::weak_ptr<BehaviorTree> tree = getTree(treeID);
            if (tree.expired()) {
                return std::weak_ptr<AI::AINode>();
            }
            return tree.lock()->getNode(nodeID);
        }

        std::optional<BehaviorTreeTemplate> AIManager::getTemplate(const std::string& treeName)
        {
            auto& allAssets = ASSET_MANAGER.GetAssets(ASSET_TYPE::BHT);
            for (auto& asset : allAssets) {
                if (asset.second.lock()->asset_name == treeName) {
                    TreeAsset* treeTemplate = dynamic_cast<TreeAsset*>(asset.second.lock().get());
                    return treeTemplate->treeTemplate;
                }
            }
            return std::nullopt;
        }

        void AIManager::getClassName(std::vector<const char*>& resultVector) const
        {
            auto& allAssets = ASSET_MANAGER.GetAssets(ASSET_TYPE::BHT);
            resultVector.reserve(allAssets.size());
            for (auto& asset : allAssets) {
                resultVector.emplace_back(asset.second.lock()->asset_name.c_str());
            }
        }

    } // namespace AI
} // namespace AG
