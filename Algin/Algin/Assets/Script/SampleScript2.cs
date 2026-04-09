using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Script.Library;

namespace Script
{
    public class SampleScript2 : IScript
    {
        [SerializeField]
        public int someNum;
        public override void Awake()
        {
            Library.Logger.log(Library.Logger.LogLevel.Info, "Awake for old script 2");
            Library.Logger.log(Library.Logger.LogLevel.Info, $"someNum is {someNum}");
        }
        public override void Start()
        {
        }
        public override void Update(double dt)
        {

            //Operate on Component
            //1. Get the Component from the object. Refer to Entity.cs for getComponent() usage. Refer to Component.cs for all the available component. 
            // Example: Obj.getComponent<Library.TransformComponent>() 
            //2. Get the inner struct of each component. Refer to Component.cs
            // Example: Transformation trf = transform.Transformation;
            //3. Operate on the inner struct to change the data
            //Example: trf.position.y += moveSpeed;
            //4. Assign back to the inner struct 
            //Example: transform.Transformation = trf;

            Script.Library.TransformComponent transform = Obj.getComponent<Library.TransformComponent>();
            if (transform == null)
            {
                Library.Logger.log(Library.Logger.LogLevel.Error, $"cant get transform");
                return;
            }
            Transformation trf = transform.Transformation;
            float moveSpeed = 5.0f * (float)dt;
            if (Library.Input.isKeyPressed(Library.Input.KeyCode.kW))
            {
                Library.Logger.log(Library.Logger.LogLevel.Info, $"W is Pressed");
                trf.position.y += moveSpeed;
            }
            if (Library.Input.isKeyPressed(Library.Input.KeyCode.kS))
            {
                Library.Logger.log(Library.Logger.LogLevel.Info, $"S is Pressed");
                trf.position.y -= moveSpeed;
            }

            if (Library.Input.isKeyPressed(Library.Input.KeyCode.kA))
            {
                Library.Logger.log(Library.Logger.LogLevel.Info, $"A is Pressed");
                trf.position.x -= moveSpeed;
            }
            if (Library.Input.isKeyPressed(Library.Input.KeyCode.kD))
            {
                Library.Logger.log(Library.Logger.LogLevel.Info, $"D is Pressed");
                trf.position.x += moveSpeed;
            }
            transform.Transformation = trf;
        }
        public override void LateUpdate(double dt) { }
        public override void Free() { }
    }
}
