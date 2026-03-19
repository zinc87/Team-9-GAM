using Script.Library;
using System.Collections.Generic;

namespace Script
{
    public class PreDelay : INode
    {
        public override void onEnter()
        {
            delay = 5.0f;
            base.onEnter();
        }

        public override void onUpdate(float dt)
        {
            delay -= dt;
            if (delay > 0.0f) return;

            getChildren(out List<Script.Library.INode> childrens);
            INode child = childrens[0];
            if (child.isRunning() || child.isReady())
            {
                child.tick();
                setStatus(child.getStatus());
                setResult(child.getResult());
            }
        }

        private float delay = 5.0f;
    }
}