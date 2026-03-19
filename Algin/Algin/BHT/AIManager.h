/*!*****************************************************************************
\file AIManager.h
\author Peh Jun Wei Gabriel
\date 11/7/2025
\brief
    Global manager for Behavior Trees (AI system).
    Provides registration, retrieval, and runtime execution of behavior trees.
*******************************************************************************/
#pragma once
#include "../Header/pch.h"
#include "AINode.h"
#include "BehaviorTree.h"
#include "TreeBuilder.h"

namespace AG {
    namespace AI {

        //Manage the runtime behaviour tree
        class AIManager : public Pattern::ISingleton<AIManager> {
        public:
            void registerTreeInstance(std::shared_ptr <BehaviorTree>& newTree);
            std::weak_ptr<BehaviorTree> getTree(BehaviorTree::TreeID treeID);
            std::weak_ptr<AI::AINode> getNode(BehaviorTree::TreeID treeID, AI::AINode::NodeID nodeID);

            std::optional<BehaviorTreeTemplate> getTemplate(const std::string& treeName);

            void getClassName(std::vector<const char*>& resultVector) const;


        private:
            std::unordered_map<BehaviorTree::TreeID, std::shared_ptr<BehaviorTree>> trees;
        };

    } // namespace AI
} // namespace AG
