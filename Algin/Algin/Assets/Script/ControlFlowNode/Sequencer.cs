using System.Collections.Generic;
using Script.Library;

namespace Script {
    public class Sequencer : Script.Library.INode{
        public override void onEnter(){
            currentIndex = 0;
            base.onEnter();
        }

        public override void onUpdate(float dt){
            // if any child fails, the node fails
            // if all children succeed, the node succeeds
            getChildren(out List<Script.Library.INode> childrens);
            Script.Library.INode currentNode = childrens[currentIndex];
            currentNode.tick();

            if (currentNode.failed()){
                onFailure();
            }
            else {
                ++currentIndex;
                Logger.log(Logger.LogLevel.Info, $"seq index: {currentIndex}");

                if (currentIndex == childrens.Count){
                    onSuccess();
                }
            }
        }

        private int currentIndex = 0;
    }
}