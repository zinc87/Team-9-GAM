using System.Collections.Generic;

namespace Script {
    public class Selector : Script.Library.INode{
        public override void onEnter(){
            currentIndex = 0;
            base.onEnter();
        }

        public override void onUpdate(float dt){
            // if all child fails, the node fails
            // if any children succeed, the node succeeds
            getChildren(out List<Script.Library.INode> childrens);
            Script.Library.INode currentNode = childrens[currentIndex];
            currentNode.tick();

            if (currentNode.succeeded()){
                onSuccess();
            }
            else if (currentNode.failed()){
                // move to the next node
                ++currentIndex;

                if (currentIndex == childrens.Count){
                    onFailure();
                }
            }
        }

        private int currentIndex = 0;
    }
}