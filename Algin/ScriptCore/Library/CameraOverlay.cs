using Script.Library;

namespace Script.Library
{
    public static class CameraOverlay
    {
        public static void SetCamera(Object cameraObj)
        {
            if (cameraObj == null)
            {
                PrivateAPI.InternalCall.setOverlayCamera("");
                return;
            }

            PrivateAPI.InternalCall.setOverlayCamera(cameraObj.ObjectID);
        }

        public static void SetActive(bool active)
        {
            PrivateAPI.InternalCall.setOverlayActive(active);
        }

        // Normalized coordinates, origin at top-left (0..1)
        public static void SetViewport(float x, float y, float w, float h)
        {
            PrivateAPI.InternalCall.setOverlayViewport(x, y, w, h);
        }
    }
}
