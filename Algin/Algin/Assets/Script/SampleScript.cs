using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Script
{
    public class SampleScript : Script.Library.IScript
    {
        [Script.Library.SerializeField]
        public float health;
        [Script.Library.SerializeField]
        public Script.Library.Object otherObj;
        public override void Awake() {
            Library.Logger.log(Library.Logger.LogLevel.Info, "Awake old script 1");
            Library.Logger.log(Library.Logger.LogLevel.Info, $"health is {health}");
            Library.Logger.log(Library.Logger.LogLevel.Info, $"otherObj is {otherObj.ObjectID}");
            Library.Logger.log(Library.Logger.LogLevel.Info, $"thisObj is {Obj.ObjectID}");
        }
        public override void Start() {
            //Library.Logger.log(Library.Logger.LogLevel.Info, $"otherObj is {otherObj.ObjectID}");
        }
        public override void Update(double dt) {

            Script.Library.TransformComponent transform = Obj.getComponent<Library.TransformComponent>();
            if (transform == null) {
                Library.Logger.log(Library.Logger.LogLevel.Error, $"cant get transform");
                return;
            }
            var trf = transform.Transformation;
            float moveSpeed = 5.0f * (float)dt;


            // camera
            /*var camera = obj.getComponent<Library.CameraComponent>();
            if (camera != null)
            {
                var camData = camera.Data;
                Library.Logger.log(Library.Logger.LogLevel.Info,
                    $"Camera: Near={camData.reflected_near}, Far={camData.reflected_far}, FOV={camData.reflected_fov}, " +
                    $"Vertical={camData.reflected_vertical}, Projection={(camData.reflected_proj == 0 ? "Orthographic" : "Perspective")}, " +
                    $"IsGameCam={camData.reflected_isGameCam}");
            }
            else
            {
                Library.Logger.log(Library.Logger.LogLevel.Warning, "Camera component not found!");
            }*/


            // --- LightComponent Test ---
            //var light = obj.getComponent<Library.LightComponent>();
            //if (light != null)
            //{
            //    var data = light.Data; // get the struct from C++ via internal call
            //    Library.Logger.log(Library.Logger.LogLevel.Info,
            //        $"Light: Pos={data.r_position}, Dir={data.r_direction}, " +
            //        $"Intensity={data.r_intensity}, Range={data.r_range}, Type={data.r_type_index}");
            //}
            //else
            //{
            //    Library.Logger.log(Library.Logger.LogLevel.Warning, "Light component not found!");
            //}

            ///

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

            // if (Library.Input.isKeyPressed(Library.Input.KeyCode.kA))
            // {
            //     Library.Logger.log(Library.Logger.LogLevel.Info, $"A is Pressed");
            //     trf.position.x -= moveSpeed;
            // }
            // if (Library.Input.isKeyPressed(Library.Input.KeyCode.kD))
            // {
            //     Library.Logger.log(Library.Logger.LogLevel.Info, $"D is Pressed");
            //     trf.position.x += moveSpeed;
            // }
            // transform.Transformation = trf;
        }
        public override void LateUpdate(double dt) { }
        public override void Free() { }
    }
}
