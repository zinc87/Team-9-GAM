﻿using Script.Library;
using System.Collections.Generic;

namespace Script
{
    public class StudentScript : IScript
    {
        public enum State
        {
            JoiningQueue,
            InQueue,
            WalkingToCheck,
            Waiting,
            MovingToArrest,
            MovingToFree,
            Done
        }

        public static List<StudentScript> All = new List<StudentScript>();

        // ---------------------
        // SYMPTOMS
        // ---------------------
        [SerializeField] public bool brain;
        [SerializeField] public bool lungs;
        [SerializeField] public bool heart;
        [SerializeField] public string currentSymptom = "";
        [SerializeField] public bool isVaping;

        // ---------------------
        // STATE
        // ---------------------
        public State CurrentState = State.JoiningQueue;

        [SerializeField] public string waypointCheckGUID;
        [SerializeField] public string waypointArrestGUID;
        [SerializeField] public string waypointFreeGUID;

        private Object waypointCheck;
        private Object waypointArrest;
        private Object waypointFree;

        private bool waypointsResolved = false;

        private Vector3D queueTargetPosition = new Vector3D(0, 0, 0);
        private bool hasQueueTarget = false;

        // Avoidance
        private float studentAvoidRadius = 1.2f;
        private float studentAvoidStrength = 1.5f;

        private static int clearedSeatIndex = 0;
        private static int quarantineSeatIndex = 0;

        public override void Awake()
        {
            All.Add(this);
        }

        public override void Start()
        {
            if (SymptomManager.Instance != null)
                SymptomManager.Instance.AssignRandomSymptom(this);
        }

        public override void Update(double dt)
        {
            if (!waypointsResolved)
                ResolveWaypoints();
            if (!waypointsResolved)
                return;

            HandleMovement(dt);
        }


        // ------------------------------------------------------
        // MAIN MOVEMENT STATE MACHINE
        // ------------------------------------------------------
        private void HandleMovement(double dt)
        {
            // ---------------------------
            // JOINING QUEUE / IN QUEUE
            // ---------------------------
            if (CurrentState == State.JoiningQueue || CurrentState == State.InQueue)
            {
                if (hasQueueTarget)
                {
                    MoveTowards(queueTargetPosition, dt);

                    if (Reached(queueTargetPosition, 0.6f))
                        CurrentState = State.InQueue;
                }
                return;
            }

            // ---------------------------
            // GOING TO CHECK
            // ---------------------------
            if (CurrentState == State.WalkingToCheck)
            {
                MoveTowards(GetPosition(waypointCheck), dt);

                if (Reached(GetPosition(waypointCheck), 0.4f))
                    CurrentState = State.Waiting;

                return;
            }

            // ---------------------------
            // TELEPORT TO ARREST AREA (HARD-CODED SEATS)
            // ---------------------------
            if (CurrentState == State.MovingToArrest)
            {
                Vector3D basePos = GetPosition(waypointArrest);
                Vector3D finalPos = GetSeatPosition(true, basePos);
                TeleportTo(finalPos);

                CurrentState = State.Done;
                return;
            }

            // ---------------------------
            // TELEPORT TO CLEARED AREA (HARD-CODED SEATS)
            // ---------------------------
            if (CurrentState == State.MovingToFree)
            {
                Vector3D basePos = GetPosition(waypointFree);
                Vector3D finalPos = GetSeatPosition(false, basePos);
                TeleportTo(finalPos);

                CurrentState = State.Done;
                return;
            }
        }


        // ------------------------------------------------------
        // PUBLIC CALLS
        // ------------------------------------------------------
        public void AssignQueuePosition(Vector3D pos)
        {
            queueTargetPosition = pos;
            hasQueueTarget = true;
        }

        public void SendToArrest() => CurrentState = State.MovingToArrest;
        public void SendToFree() => CurrentState = State.MovingToFree;


        // ------------------------------------------------------
        // WAYPOINT RESOLUTION
        // ------------------------------------------------------
        private void ResolveWaypoints()
        {
            if (waypointCheck == null)
                waypointCheck = FindWaypoint("Check");

            if (waypointArrest == null)
                waypointArrest = FindWaypoint("Arrest");

            if (waypointFree == null)
                waypointFree = FindWaypoint("Free");

            if (waypointCheck == null || waypointArrest == null || waypointFree == null)
                return;

            waypointCheckGUID = waypointCheck.ObjectID;
            waypointArrestGUID = waypointArrest.ObjectID;
            waypointFreeGUID = waypointFree.ObjectID;

            waypointsResolved = true;
        }


        // ------------------------------------------------------
        // MOVEMENT + AVOIDANCE
        // ------------------------------------------------------
        private void MoveTowards(Vector3D target, double dt)
        {
            TransformComponent tr = Obj.getComponent<TransformComponent>();
            if (tr == null) return;

            var t = tr.Transformation;
            Vector3D pos = t.position;

            Vector3D desired = (target - pos);
            float dist = desired.magnitude();
            if (dist < 0.05f) return;

            desired = desired.normalize();

            // Avoid bumping into students
            Vector3D avoid = new Vector3D(0, 0, 0);

            foreach (var other in StudentScript.All)
            {
                if (other == this) continue;

                var otherTr = other.Obj.getComponent<TransformComponent>();
                Vector3D otherPos = otherTr.Transformation.position;

                float d = (pos - otherPos).magnitude();
                if (d < studentAvoidRadius && d > 0.01f)
                {
                    Vector3D push = (pos - otherPos).normalize() * (studentAvoidRadius - d);
                    avoid = avoid + push;
                }
            }

            if (avoid.magnitude() > 0.01f)
                desired = (desired + avoid * studentAvoidStrength).normalize();

            float speed = 5.0f;
            t.position = t.position + desired * (float)(speed * dt);
            tr.Transformation = t;
        }


        // ------------------------------------------------------
        // SEAT POSITION CALCULATION
        // ------------------------------------------------------
        private Vector3D GetSeatPosition(bool isQuarantine, Vector3D basePos)
        {
            // index → seat number (0 to 7)
            int index = isQuarantine ? quarantineSeatIndex : clearedSeatIndex;

            // 2 rows, 4 columns
            int row = index / 4;      // 0 or 1
            int col = index % 4;      // 0..3

            // Real world spacing from your screenshot
            float spacing = 13.918f;

            float x = spacing * col;
            float z = -spacing * row;    // rows go BACKWARDS

            // Increment seat index
            if (isQuarantine)
                quarantineSeatIndex = (quarantineSeatIndex + 1) % 8;
            else
                clearedSeatIndex = (clearedSeatIndex + 1) % 8;

            return new Vector3D(
                basePos.x + x,
                basePos.y,
                basePos.z + z
            );
        }

        // ------------------------------------------------------
        // TELEPORT HELPER
        // ------------------------------------------------------
        private void TeleportTo(Vector3D pos)
        {
            var tr = Obj.getComponent<TransformComponent>();
            if (tr == null) return;

            var t = tr.Transformation;
            t.position = pos;
            tr.Transformation = t;
        }


        private Object FindWaypoint(string type)
        {
            foreach (var wp in WaypointHolder.All)
                if (wp.waypointType == type)
                    return new Object(wp.id);

            return null;
        }

        private Vector3D GetPosition(Object obj)
        {
            var tr = obj?.getComponent<TransformComponent>();
            return tr?.Transformation.position ?? new Vector3D(0, 0, 0);
        }

        private bool Reached(Vector3D target, float threshold)
        {
            var tr = Obj.getComponent<TransformComponent>();
            if (tr == null) return false;

            return (tr.Transformation.position - target).magnitude() < threshold;
        }

        public override void LateUpdate(double dt) { }

        public override void Free()
        {
            All.Remove(this);
        }
    }
}
