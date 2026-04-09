using System.Collections.Generic;
using Script.Library;

namespace Script{
    public class ParallelSequencer : INode{
        public override void onUpdate(float dt){
            bool allSuccess = true;
            getChildren(out List<Script.Library.INode> childrens);

            foreach(INode child in childrens){
                child.tick();

                if (child.failed()){
                    onFailure();
                }
                else if (child.isRunning() && child.failed()){
                    allSuccess = false;
                }
            }

            if (allSuccess){
                onSuccess();
            }
        }
        
    }
}