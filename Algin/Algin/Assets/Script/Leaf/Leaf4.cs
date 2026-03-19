using Script.Library;
using System.Collections.Generic;

namespace Script
{
    public class Leaf4 : INode
    {
        public override void onEnter()
        {
            Logger.log(Logger.LogLevel.Info, $"Leaf 4 on enter");
            base.onLeafEnter();
        }
        public override void onUpdate(float dt)
        {
            Logger.log(Logger.LogLevel.Info, $"Leaf 4 on update");
            onSuccess();
        }
    }
}