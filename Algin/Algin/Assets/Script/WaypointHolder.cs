using Script.Library;
using System.Collections.Generic;

namespace Script
{
    public class WaypointHolder : IScript
    {
        // Global registry of all waypoint scripts
        public static List<WaypointHolder> All = new List<WaypointHolder>();

        [SerializeField] public string id = "";
        [SerializeField] public string waypointType = ""; 
        // Example values: "Check", "Arrest", "Free"

        public override void Awake()
        {
            id = Obj.ObjectID;     // auto assign GUID
            All.Add(this);         // register globally
        }

        public override void Free()
        {
            All.Remove(this);      // unregister
        }

        public override void Start() {}
        public override void Update(double dt) {}
        public override void LateUpdate(double dt) {}
    }
}
