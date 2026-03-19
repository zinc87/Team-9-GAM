using Microsoft.Win32;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Script.Library
{
    public static class MathPlus
    {
        public static T Clamp<T>(T value, T min, T max) where T : IComparable<T>
        {
            if (value.CompareTo(min) < 0) return min;
            if (value.CompareTo(max) > 0) return max;
            return value;
        }

        public static T Max<T>(T val1, T val2) where T : IComparable<T>
        {
            if (val1.CompareTo(val2) >= 0)
                return val1;
            else
                return val2;
        }
    }
}
