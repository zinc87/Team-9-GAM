using Script.Library;
using System.Collections.Generic;

namespace Script
{
    public class SceneTest : IScript
    {
        [SerializeField] public string filename = "";
        bool isPressed = false;

        public override void Awake() {}
        public override void Start() {}

        public override void Update(double dt) {
            bool currentPressed = Input.isKeyPressed(Input.KeyCode.kL);
            if(currentPressed && !isPressed)
            {
                isPressed = true;
                Scene.loadScene(filename);
            }

            isPressed = currentPressed;
        }

        public override void LateUpdate(double dt) {}
        public override void Free() {}
    }
}
