#include "pch.h"
#include "AINode.h"

namespace AG {
	namespace AI {
		AINode::AINode()
		{
		}
		AINode::AINode(NodeRep nodeRep, System::IObject::ID ObjId) : nodeName{nodeRep.name}
		{
			nodeID = Data::GenerateGUID();
			//Create script node instance
			SCRIPTENGINE.createNodeInstance(ObjId, nodeID, "Script." + nodeName, fieldNameToData);
		}
		void AINode::tick(float dt)
		{
			(void)dt;
			if (status == NodeStatus::READY) {
				SCRIPTENGINE.nodeInstanceOnEnter(nodeID);
			}
			if(status == NodeStatus::RUNNING) {
				SCRIPTENGINE.nodeInstanceOnUpdate(nodeID, Benchmarker::GetInstance().GetDeltaTime());
			}
			if (status == NodeStatus::EXITING){
				SCRIPTENGINE.nodeInstanceOnExit(nodeID);
			}
		}

		void AINode::setStatus(NodeStatus newStatus)
		{
			status = newStatus;
		}

		NodeStatus AINode::getStatus()
		{
			return status;
		}

		void AINode::setResult(NodeResult newResult)
		{
			result = newResult;
		}

		NodeResult AINode::getResult()
		{
			return result;
		}
		
		void AINode::getChildren(std::vector<NodeID>& output)
		{
			for (auto& child : childrens) {
				output.emplace_back(child->nodeID);
			}
		}

		void AINode::OnEnter()
		{
			SCRIPTENGINE.nodeInstanceOnEnter(nodeID);
		}
		void AINode::OnUpdate(float)
		{
			SCRIPTENGINE.nodeInstanceOnUpdate(nodeID, Benchmarker::GetInstance().GetDeltaTime());
		}
		void AINode::OnExit()
		{
			SCRIPTENGINE.nodeInstanceOnExit(nodeID);
		}
	}
}