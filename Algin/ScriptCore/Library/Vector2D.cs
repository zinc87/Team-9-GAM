using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

#pragma warning disable CS1591 // Missing XML comment for publicly visible type or member

namespace Script.Library
{
    [StructLayout(LayoutKind.Sequential)]

    public struct Vector2D
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
        /// Create a vector from components.
        /// </summary>
        public Vector2D(float X, float Y) { this.x = X; this.y = Y; }

        // Operators
        public static Vector2D operator +(Vector2D a, Vector2D b)
            => new Vector2D(a.x + b.x, a.y + b.y);

        public static Vector2D operator -(Vector2D v)
            => new Vector2D(-v.x, -v.y);

        public static Vector2D operator -(Vector2D a, Vector2D b)
            => new Vector2D(a.x - b.x, a.y - b.y);

        public static Vector2D operator *(Vector2D v, float s)
            => new Vector2D(v.x * s, v.y * s);

        public static Vector2D operator *(Vector2D v, double s)
            => new Vector2D(v.x * (float)s, v.y * (float)s);

        // Magnitude and normalization
        public float magnitude()
            => (float)Math.Sqrt(x * x + y * y);

        public Vector2D normalize()
        {
            float m = magnitude();
            if (m < 0.0001f)
                return new Vector2D(0, 0);

            return new Vector2D(x / m, y / m);
        }

        public override string ToString()
        {
            return $"({x:F2}, {y:F2})";
        }
    }
}
