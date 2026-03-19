using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Script.Library
{
    static public class DebugRenderer
    {
        static public void drawLine(Vector3D a, Vector3D b)
        {
            PrivateAPI.InternalCall.drawLine(a, b);
        }
        static public void drawWireFrame()
        {

        }
        static public void drawCube()
        {

        }
    }
}
