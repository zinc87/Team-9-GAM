using Script.Library;
using System.Collections.Generic;

namespace Script
{
    public class Delay : INode
    {
        public override void onEnter()
        {
            delay = 20.0f;
            base.onEnter();
        }

        public override void onUpdate(float dt)
        {
            delay -= dt;
            getChildren(out List<Script.Library.INode> childrens);
            INode child = childrens[0];
            if (child.isRunning() || child.isReady())
            {
                child.tick();
            }

            if (delay < 0.0f)
            {
                setStatus(child.getStatus());
                setResult(child.getResult());
            }
        }

        private float delay = 0.0f;
    }
}