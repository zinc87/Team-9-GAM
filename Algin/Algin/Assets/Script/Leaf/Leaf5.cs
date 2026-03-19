using Script.Library;
using System.Collections.Generic;

namespace Script
{
    public class Leaf5 : INode
    {
        public override void onEnter()
        {
            Logger.log(Logger.LogLevel.Info, $"Leaf 5 on enter");
            base.onLeafEnter();
        }
        public override void onUpdate(float dt)
        {
            Logger.log(Logger.LogLevel.Info, $"Leaf 5 on update");
            onSuccess();
        }
    }
}