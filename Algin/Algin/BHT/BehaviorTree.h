#pragma once
#include "pch.h"

namespace AG {
	namespace AI {
        struct NodeRep;
        class AINode;

        class BehaviorTreeTemplate {
        public:
            using TreeTemplateID = Data::GUID;

            const std::vector<NodeRep>& getNodesVector() const;
            const std::string& getName() const;
            const TreeTemplateID& getID() const;

            void setNodesVector(std::vector<NodeRep>&& newVector);
            void setTemplateID(TreeTemplateID&& newID);
            void setTemplatePath(const std::filesystem::path& newFilePath);
        private:
            std::vector<NodeRep> nodesVector;
            std::filesystem::path filePath;
            std::string name;
            TreeTemplateID TemplateID;
        };

        // ---------------------------------------------------------------------
        // Behavior Tree Instance
        // ---------------------------------------------------------------------
        class BehaviorTree {
        public:
            using TreeID = Data::GUID;

            BehaviorTree(BehaviorTreeTemplate newTreeTemplate, System::IObject::ID newObjID);

            const TreeID& getID();
            std::weak_ptr<AINode> getNode(AINode::NodeID nodeID);

            void Run(float dt);
        private:
            void AddNode(std::shared_ptr<AI::AINode> root, const std::vector<AI::NodeRep>& nodeReps);
            std::weak_ptr<AINode> recursiveFindNode(std::weak_ptr<AINode> currentNode, AINode::NodeID targetID);
        private:
            std::shared_ptr<AINode> root = nullptr;
            BehaviorTreeTemplate treeTemplate;
            TreeID treeID;
            System::IObject::ID objID;
            std::unordered_map<AINode::NodeID, std::shared_ptr<AINode>> IdToNodePtr;
        };
	}
}
