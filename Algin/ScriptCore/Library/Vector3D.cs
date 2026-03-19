using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

#pragma warning disable CS1591 // Missing XML comment for publicly visible type or member

namespace Script.Library
{
    [System.Runtime.InteropServices.StructLayout(System.Runtime.InteropServices.LayoutKind.Sequential)]
   
    public struct Vector3D
    {
        /// <summary>
        /// X component.
        /// </summary>
        public float x;
        /// <summary>
        /// Y component.
        /// </summary>
        public float y;
        /// <summary>
        /// Z component.
        /// </summary>
        public float z;

        /// <summary>
        /// Create a vector from components.
        /// </summary>
        public Vector3D(float X, float Y, float Z) { this.x = X; this.y = Y; z = Z; }
        
        public static Vector3D operator +(Vector3D a, Vector3D b)
            => new Vector3D(a.x + b.x, a.y + b.y, a.z + b.z);

        public static Vector3D operator -(Vector3D v)
        {
            return new Vector3D(-v.x, -v.y, -v.z);
        }

        public static Vector3D operator -(Vector3D a, Vector3D b)
            => new Vector3D(a.x - b.x, a.y - b.y, a.z - b.z);

        public static Vector3D operator *(Vector3D v, float s)
            => new Vector3D(v.x * s, v.y * s, v.z * s);

        public static Vector3D operator *(Vector3D v, double s)
            => new Vector3D(v.x * (float)s, v.y * (float)s, v.z * (float)s);

        public float magnitude()
            => (float)Math.Sqrt(x * x + y * y + z * z);

        public Vector3D normalize()
        {
            float m = magnitude();
            if (m < 0.0001f)
                return new Vector3D(0, 0, 0);

            return new Vector3D(x / m, y / m, z / m);
        }
        public override string ToString()
        {
            return $"({x:F2}, {y:F2}, {z:F2})";
        }
    }
}
