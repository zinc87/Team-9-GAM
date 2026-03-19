using System;
using System.Runtime.InteropServices;

namespace Script.Library
{
    public enum CollisionLayers
    {
        Layer_Default = 1 << 0,
        Layer_Player = 1 << 1,
        Layer_NPC = 1 << 2,
        Layer_Trigger = 1 << 3,
        Layer_Computer = 1 << 4,
        Layer_Centrifuge = 1 << 5,
        Layer_SkinTest = 1 << 6,
        Layer_UrineTest = 1 << 7,
        Layer_All = ~0
    };

    [StructLayout(LayoutKind.Sequential)]
    public struct RaycastHitData
    {
        public Vector3D point;
        public Vector3D normal;
        public float distance;
        public Object obj;
        public CollisionLayers layer;
    }

    [StructLayout(LayoutKind.Sequential)]
    internal struct RaycastHitDataInternal
    {
        public Vector3D point;
        public Vector3D normal;
        public float distance;
        public string collidedObjectID;
        public CollisionLayers layer;
    }

    public static class Physics
    {
        // THIS is the only exposed raycast
        public static bool Raycast( 
            Vector3D origin,
            Vector3D direction,
            out RaycastHitData hit,
            float maxDistance = 3.0f
            )
        {
            hit = default;

            RaycastHitDataInternal internalHit;
            internalHit.point = hit.point;
            internalHit.normal = hit.normal;
            internalHit.distance = hit.distance;


            bool hasHit = PrivateAPI.InternalCall.isRayCastHit(
                origin,
                direction,
                out internalHit,
                maxDistance
            );

            hit.point = internalHit.point;
            hit.normal = internalHit.normal;
            hit.distance = internalHit.distance;
            hit.obj = new Object(internalHit.collidedObjectID);
            hit.layer = internalHit.layer;

            return hasHit;
        }
    }
}
