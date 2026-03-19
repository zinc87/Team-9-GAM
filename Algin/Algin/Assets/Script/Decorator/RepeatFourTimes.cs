using Script.Library;
using System.Collections.Generic;

namespace Script {
    public class RepeatFourTimes : INode{
        public override void onEnter(){
            counter = 0;
            base.onEnter();
        }

        public override void onUpdate(float dt){
            // repeat the child until 4 successes, stop on any failure
            getChildren(out List<INode> childrens);
            INode child = childrens[0];

            child.tick();

            if (child.succeeded()){
                ++counter;

                if (counter == 4){
                    onSuccess();
                }
                else {
                    child.setStatus(NodeStatus.READY);
                }
            }
            else if (child.failed()){
                onFailure();
            }
        }

        private int counter = 0;
    }

    
}