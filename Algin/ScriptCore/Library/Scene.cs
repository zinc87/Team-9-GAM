using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Script.Library
{
    public static class Scene
    {
        public static void loadScene(string filename)
        {
            PrivateAPI.InternalCall.loadScene(filename);
        }

        public static string getSceneName()
        {
            return PrivateAPI.InternalCall.getSceneName();
        }
    }

    public static class Graphics
    {
        public static float Gamma
        {
            get => PrivateAPI.InternalCall.getGamma();
            set => PrivateAPI.InternalCall.setGamma(value);
        }

        public static bool TonemapEnabled
        {
            get => PrivateAPI.InternalCall.getTonemapEnabled();
            set => PrivateAPI.InternalCall.setTonemapEnabled(value);
        }
    }
}
