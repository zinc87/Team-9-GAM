using Script.Library;
using System.Collections.Generic;

namespace Script {
    public class InvertedRepeater : INode {
        public override void onUpdate(float dt){
            getChildren(out List<Script.Library.INode> childrens);
            INode child = childrens[0];

            child.tick();

            if (child.succeeded()){
                onFailure();
            }
            else if (child.failed()){
                onSuccess();
            }
        }
        public override void onExit(){
            setStatus(NodeStatus.READY);
        }
    }
}