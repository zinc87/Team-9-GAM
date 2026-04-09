﻿using Script.Library;
using System.Collections.Generic;

namespace Script
{
    public class ChairManager : IScript
    {
        public static ChairManager Instance;

        private List<Vector3D> clearedSeatOffsets = new List<Vector3D>();
        private List<Vector3D> quarantineSeatOffsets = new List<Vector3D>();

        private int clearedIndex = 0;
        private int quarantineIndex = 0;

        // spacing between chairs
        private float spacingX = 2.0f;  // left-right
        private float spacingZ = 2.0f;  // forward-back

        public override void Awake()
        {
            Instance = this;

            BuildOffsets(clearedSeatOffsets);
            BuildOffsets(quarantineSeatOffsets);
        }

        private void BuildOffsets(List<Vector3D> list)
        {
            list.Clear();

            // Two rows, four columns
            for (int row = 0; row < 2; row++)
            {
                for (int col = 0; col < 4; col++)
                {
                    float x = col * spacingX;
                    float z = row * spacingZ;

                    list.Add(new Vector3D(x, 0, -z)); 
                    // negative Z pushes seats away from waypoint
                }
            }
        }

        public Vector3D GetClearedSeatPosition(Vector3D waypointPos)
        {
            Vector3D offset = clearedSeatOffsets[clearedIndex];
            clearedIndex = (clearedIndex + 1) % clearedSeatOffsets.Count;

            return waypointPos + offset;
        }

        public Vector3D GetQuarantineSeatPosition(Vector3D waypointPos)
        {
            Vector3D offset = quarantineSeatOffsets[quarantineIndex];
            quarantineIndex = (quarantineIndex + 1) % quarantineSeatOffsets.Count;

            return waypointPos + offset;
        }

        public override void Start() { }
        public override void Update(double dt) { }
        public override void LateUpdate(double dt) { }
        public override void Free() { }
    }
}
