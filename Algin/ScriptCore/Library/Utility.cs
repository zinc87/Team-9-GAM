using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Script.Library
{
    public static class Utility
    {
        public static bool AABB(Vector2D topLeft, Vector2D btmRight, Vector2D mousePos)
        {
            return (mousePos.x >= topLeft.x &&
            mousePos.x <= btmRight.x &&
            mousePos.y <= topLeft.y &&
            mousePos.y >= btmRight.y);
        }

        public static void setAlpha(Object obj, float alphaValue)
        {
            if (obj == null) return;

            var img = obj.getComponent<Image2DComponent>();
            if (img != null)
            {
                var d = img.Data;
                d.color.a = alphaValue;
                img.Data = d;
            }

            var txt = obj.getComponent<TextMeshUIComponent>();
            if (txt != null)
            {
                var d = txt.Data;
                d.color.a = alphaValue;
                txt.Data = d;
            }
        }
    }
}
