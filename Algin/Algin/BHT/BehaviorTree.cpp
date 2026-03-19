#include "pch.h"
#include "BehaviorTree.h"

namespace AG {
	namespace AI {
		BehaviorTree::BehaviorTree(BehaviorTreeTemplate newTreeTemplate, System::IObject::ID newObjID) : treeTemplate{ newTreeTemplate }, treeID{}, objID{newObjID}
		{
			treeID = Data::GenerateGUID();
			AddNode(root, treeTemplate.getNodesVector());
		}
		const AI::BehaviorTree::TreeID& BehaviorTree::getID()
		{
			return treeID;
		}
		std::weak_ptr<AINode> BehaviorTree::getNode(AINode::NodeID nodeID)
		{
			if (IdToNodePtr.find(nodeID) == IdToNodePtr.end()) {
				AG_CORE_WARN("No such node {}", nodeID);
				return std::weak_ptr<AINode>();
			}
			return IdToNodePtr[nodeID];
		}
		void BehaviorTree::Run(float dt) {
			if (root) {
				root->tick(dt);

				//Restart the tree when done
				if (!(root->getStatus() == NodeStatus::RUNNING)) {
					root->status = NodeStatus::READY;
				}
			}
		}
		void BehaviorTree::AddNode(std::shared_ptr<AI::AINode> rootNode, const std::vector<AI::NodeRep>& nodeReps)
		{
			(void)rootNode;
			if (nodeReps.empty()) return;

			// Stack will store pairs: (depth, node)
			std::stack<std::pair<int, std::shared_ptr<AINode>>> stack;

			root = std::make_shared<AINode>(nodeReps[1], objID); // 0 is the empty root node
			IdToNodePtr[root->nodeID] = root;

			// Push the root node with its depth
			stack.push(std::pair<int, std::shared_ptr<AINode>>(0, root));

			// Start from index 1 because nodeReps[0] = root itself
			for (size_t i = 2; i < nodeReps.size(); ++i) {
				const AI::NodeRep& rep = nodeReps[i];

				// Make the new node
				std::shared_ptr<AINode> newNode = std::make_shared<AINode>(rep, objID);
				IdToNodePtr[newNode->nodeID] = newNode;

				int depth = rep.depth;

				// Find the correct parent by popping stack
				// until stack.back().depth < newNode.depth
				while (!stack.empty() && stack.top().first >= depth) {
					stack.pop();
				}

				if (stack.empty()) {
					AG_CORE_ERROR("Invalid node depth structure in BehaviorTree!");
					return;
				}

				// Parent is now at stack.back()
				std::shared_ptr<AINode> parent = stack.top().second;

				// Link nodes
				parent->childrens.push_back(newNode);
				newNode->parent = parent;

				// Push this new node to stack
				stack.push(std::pair<int, std::shared_ptr<AINode>>(depth, newNode));
			}
		}
		std::weak_ptr<AINode> BehaviorTree::recursiveFindNode(std::weak_ptr<AINode> currentNode, AINode::NodeID targetID)
		{
			if (currentNode.expired()) return std::weak_ptr<AINode>();
			auto node = currentNode.lock();
			if (node->nodeID == targetID) return currentNode;
			if (node->childrens.empty()) return std::weak_ptr<AINode>();
			for (auto& child : node->childrens) {
				std::weak_ptr<AINode> result = recursiveFindNode(child, targetID);
				if (!result.expired()) {
					return result;
				}
			}
			return std::weak_ptr<AINode>();
		}
		const std::vector<NodeRep>& BehaviorTreeTemplate::getNodesVector() const
		{
			return nodesVector;
		}
		const std::string& BehaviorTreeTemplate::getName() const
		{
			return name;
		}
		const BehaviorTreeTemplate::TreeTemplateID& BehaviorTreeTemplate::getID() const
		{
			return TemplateID;
		}
		void BehaviorTreeTemplate::setNodesVector(std::vector<NodeRep>&& newVector)
		{
			nodesVector = newVector;
		}
		void BehaviorTreeTemplate::setTemplateID(TreeTemplateID&& newID)
		{
			TemplateID = newID;
		}
		void BehaviorTreeTemplate::setTemplatePath(const std::filesystem::path& newFilePath)
		{
			filePath = newFilePath;
			name = newFilePath.filename().string();
		}
	}
}
