#pragma once
#include "pch.h"

namespace AG {
	namespace AI {
        struct NodeRep;

        enum class NodeStatus {
            READY, // node is enterable
            RUNNING, // node is currently running
            EXITING, // node has succeeded or failed
            SUSPENDED // node won't exceute anything
        };

        enum class NodeResult {
            IN_PROGRESS, // still being run 
            SUCCESS, // node succeeded
            FAILURE // node failed
        };

        // ---------------------------------------------------------------------
        // Base AI Node
        // ---------------------------------------------------------------------
        class AINode {
        public:
            AINode();
            AINode(NodeRep nodeRep, System::IObject::ID);

            using NodeID = Data::GUID;

            void tick(float dt);

            void setStatus(NodeStatus newStatus);
            NodeStatus getStatus();
            void setResult(NodeResult newResult);
            NodeResult getResult();
            void getChildren(std::vector<NodeID>& output);
        private:
            void OnEnter();
            void OnUpdate(float dt);
            void OnExit();
        private:
            NodeID nodeID;
            NodeStatus status = NodeStatus::READY;
            NodeResult result = NodeResult::IN_PROGRESS;
            std::weak_ptr<AINode> parent;
            std::vector<std::shared_ptr<AINode>> childrens;
            std::string nodeName;

            std::unordered_map<std::string, std::unique_ptr<void, std::function<void(void*)>>> fieldNameToData;

            friend class BehaviorTree;
        };
	}
}