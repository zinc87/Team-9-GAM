using PrivateAPI;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Net.NetworkInformation;
using System.Text;
using System.Threading.Tasks;

#pragma warning disable CS1591 // Missing XML comment for publicly visible type or member

namespace Script.Library
{
    public enum NodeStatus
    {
        READY,
        RUNNING,
        EXITING,
        SUSPENDED
    }

    public enum NodeResult
    {
        IN_PROGRESS,
        SUCCESS,
        FAILURE
    }

    public class INode
    {
        [System.AttributeUsage(System.AttributeTargets.Field)]
        public class SerializeField : System.Attribute { }
        /// <summary>
        /// Base onEnter function that should be called by derived class for every node. Derived onEnter function should not be called recursively but should be called
        /// by c++ engine.
        /// User can choose not to implement onEnter function. OnEnter function should only be called on itself, calling on other nodes (child.onEnter() may lead to undefined behaviour
        /// </summary>
        virtual public void onEnter()
        {
            // base logic is to mark as running
            setStatus(NodeStatus.RUNNING);
            setResult(NodeResult.IN_PROGRESS);


            // and this node's children as ready to run
            setStatusChildren(NodeStatus.READY);
            setResultChildren(NodeResult.IN_PROGRESS);
        }
        public void onLeafEnter()
        {
            setStatus(NodeStatus.RUNNING);
            setResult(NodeResult.IN_PROGRESS);
        }
        virtual public void onUpdate(float dt)
        {

        }
        virtual public void onExit()
        {
            // base logic is to mark the node as done executing
            setStatus(NodeStatus.SUSPENDED);
        }

        //utility function for Node script
        public void tick()
        {
            InternalCall.nodeTick(Obj.ObjectID, NodeID);
        }

        public void onSuccess()
        {
            setStatus(NodeStatus.EXITING);
            setResult(NodeResult.SUCCESS);
        }
        public void onFailure()
        {
            setStatus(NodeStatus.EXITING);
            setResult(NodeResult.FAILURE);
        }
        /// <summary>
        /// Set the status of only the current node
        /// </summary>
        public void setStatus(NodeStatus newStatus)
        {
            InternalCall.setNodeStatus(Obj.ObjectID, NodeID, newStatus);
        }
        /// <summary>
        /// Set the status of the current nodes and all child node recursively
        /// </summary>
        public void setStatusAll(NodeStatus newStatus)
        {
            setStatus(newStatus);
            getChildren(out List<INode> childrens);
            foreach (INode child in childrens)
            {
                child.setStatusAll(newStatus);
            }
        }
        public void setStatusChildren(NodeStatus newStatus)
        {
            getChildren(out List<INode> childrens);
            foreach(INode child in childrens)
            {
                child.setStatus(newStatus);
            }
        }
        public bool isReady()
        {
            return getStatus() == NodeStatus.READY;
        }
        public bool succeeded()
        {
            return getResult() == NodeResult.SUCCESS;
        }
        public bool failed()
        {
            return getResult() == NodeResult.FAILURE;
        }
        public bool isRunning()
        {
            return getStatus() == NodeStatus.RUNNING;
        }
        public bool isSuspended()
        {
            return getStatus() == NodeStatus.SUSPENDED;
        }
        public void getChildren(out List<INode> children)
        {
            children = new List<INode>();

            string[] childrens = InternalCall.getNodeChildrens(Obj.ObjectID, NodeID);
            foreach (string childNodeID in childrens)
            {
                children.Add(new INode(Obj.ObjectID, childNodeID));
            }
        }
        public NodeStatus getStatus()
        {
            InternalCall.getNodeStatus(Obj.ObjectID, NodeID, out NodeStatus status);
            return status;
        }
        public NodeResult getResult()
        {
            InternalCall.getNodeResult(Obj.ObjectID, NodeID, out NodeResult result);
            return result;
        }
        public void setResult(NodeResult result)
        {
            InternalCall.setNodeResult(Obj.ObjectID, NodeID, result);
        }

        public INode()
        {
            Obj = new Object();
            NodeID = "0000";
        }
        public INode(string newObjID, string newNodeID)
        {
            Obj = new Object(newObjID);
            NodeID = newNodeID;
        }
        public Object Obj;
        private readonly string NodeID;
        
        private void setResultChildren(NodeResult result) {
            getChildren(out List<INode> childrens);
            foreach(INode child in childrens)
            {
                child.setResult(result);
            }
        }
       
    }
}
