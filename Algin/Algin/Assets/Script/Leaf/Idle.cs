using Script.Library;

namespace Script
{
    public class Idle : INode
    {
        public override void onEnter()
        {
            timer = 5.0f;
        }
        public override void onUpdate(float dt)
        {
            timer -= dt;
            Logger.log(Logger.LogLevel.Info, $"Idleing");
            if (timer < 0.0f)
            {
                onSuccess();
            }
        }

        private float timer = 0.0f;
    }
}