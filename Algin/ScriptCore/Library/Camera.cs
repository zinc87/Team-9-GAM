using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace Script.Library
{
    [StructLayout(LayoutKind.Sequential)]
    public struct CameraRay
    {
        public Vector3D origin;
        public Vector3D direction;
    }

    static public class Camera
    {
        static public CameraRay getCameraRay(Object obj)
        {
            if (obj == null)
            {
                Logger.log(Logger.LogLevel.Warning, "getCameraRay: Object is null");
                return new CameraRay();
            }
            
            // Let C++ handle the camera component lookup - it will warn if not found
            PrivateAPI.InternalCall.GetCameraRay(obj.ObjectID, out CameraRay ray);
            return ray;
        }
    }
}
