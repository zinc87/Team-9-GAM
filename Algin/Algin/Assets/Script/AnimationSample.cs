using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Script.Library;

namespace Script
{
    public class AnimationController : Script.Library.IScript
    {
        /*
            available function
            .ChangeModel(string model_name)
            .PlayAnimation(string animation name)
            .SetLoopAnimation(bool shudLoop)
            .Play()
            .Stop()
            .ResetTimer()
            .SetTimer(float t)
            .SetSubmeshMaterial(string submesh_name,string material name)

        */
        public override void Awake() { }

        public override void Start()
        {
            
        }

        public override void Update(double dt)
        {
        /*
            available component function
            .ChangeModel(string model_name)
            .PlayAnimation(string animation name) -> set animation clip and play automatically
            .SetLoopAnimation(bool shudLoop) -> set true if animation shud loop
            .Play() -> u know
            .Stop() -> u know
            .ResetTimer() -> reset the tick for animation
            .SetTimer(float t) -> set the tick for animation, in seconds (s)
            .SetSubmeshMaterial(string submesh_name,string material name) -> if u want to add material

        */
            var mesh = Obj.getComponent<SkinnedMeshRenderer>();
            if (mesh != null)  
            {
                if (Input.isKeyPressed(Input.KeyCode.kG)) // set mesh
                {
                    mesh.ChangeModel("Patient1.agskinnedmesh");
                    mesh.SetSubmeshMaterial("Body", "P1_Body.agmat");
                }
                else if (Input.isKeyPressed(Input.KeyCode.kH)) // play animation
                {
                    Library.Logger.log(Library.Logger.LogLevel.Info, "set animation");
                    mesh.PlayAnimation("waving");
                }
                else if (Input.isKeyPressed(Input.KeyCode.kJ)) // set to loop
                {
                    mesh.SetLoopAnimation(true);
                }
                else if (Input.isKeyPressed(Input.KeyCode.kK)) // stop animation
                {
                    mesh.Stop();
                }
                else if (Input.isKeyPressed(Input.KeyCode.kL)) // reset animation timer
                {
                    mesh.ResetTimer();
                }
            }
        }



            
        

        public override void LateUpdate(double dt) { }
        public override void Free() { }





    }
}