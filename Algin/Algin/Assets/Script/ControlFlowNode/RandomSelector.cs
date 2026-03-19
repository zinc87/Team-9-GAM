using System.Collections.Generic;
using Script.Library;

namespace Script {
    public class RandomSelector : INode {
        public override void onEnter() {
            base.onEnter();
            chooseRandomNode();
        }

        public override void onUpdate(float dt) {
            // if any child succeeds, the node succeeds
            // if all children fail, the node fails

            getChildren(out List<Script.Library.INode> childrens);
            INode currentNode = childrens[randomIndex];
        }

        public void chooseRandomNode(){
            //Need the RNG libary first
        }

        private int randomIndex = 0;
    }
}