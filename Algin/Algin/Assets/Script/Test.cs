using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Script
{
    public class Test : Script.Library.IScript
    {
        [Script.Library.SerializeField]
        public Script.Library.Object obj1;

        [Script.Library.SerializeField]
        public Script.Library.Object obj2;

        [Script.Library.SerializeField]
        public Script.Library.Object obj3;

        private bool keyWasDown1 = false;
        private bool keyWasDown2 = false;
        private bool keyWasDown3 = false;



        public override void Awake() {
            
        }
        public override void Start() {

        }
        public override void Update(double dt) {
            if( obj1 == null || obj2 == null || obj3 == null)
            {
                Library.Logger.log(Library.Logger.LogLevel.Error, $"obj1 or obj2 or obj3 is null");
                return;
            }

            bool isDown1 = Library.Input.isKeyPressed(Library.Input.KeyCode.k1);
            bool isDown2 = Library.Input.isKeyPressed(Library.Input.KeyCode.k2);
            bool isDown3 = Library.Input.isKeyPressed(Library.Input.KeyCode.k3);


            if (isDown1 && !keyWasDown1)
            {
                obj1.setActive(!obj1.isActive());
                Library.Logger.log(Library.Logger.LogLevel.Error, $"K1 Pressed");
            }

            if (isDown2 && !keyWasDown2)
            {
                obj2.setActive(!obj2.isActive());
                Library.Logger.log(Library.Logger.LogLevel.Error, $"K2 Pressed");
            }

            if (isDown3 && !keyWasDown3)
            {
                obj3.setActive(!obj3.isActive());
                Library.Logger.log(Library.Logger.LogLevel.Error, $"K3 Pressed");
            }

            keyWasDown1 = isDown1;
            keyWasDown2 = isDown2;
            keyWasDown3 = isDown3;

        }
        public override void LateUpdate(double dt) { }
        public override void Free() { }
    }
}
