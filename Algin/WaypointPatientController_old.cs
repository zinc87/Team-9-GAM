// ============================================================================
// WaypointPatientController.cs - Handles patient movement through waypoints
// ============================================================================
// Supports 4 waypoints with smooth movement between each.
// ============================================================================

using Script.Library;
using System;

namespace Script
{
    /// <summary>
    /// Tracks which waypoint the patient is currently targeting.
    /// </summary>
    public enum WaypointTarget
    {
        AtWaypoint1,
        MovingToWaypoint2,
        AtWaypoint2,
        MovingToWaypoint3,
        AtWaypoint3,
        MovingToWaypoint4,
        AtWaypoint4,
        MovingToWaypoint5,
        AtWaypoint5,
        MovingToWaypoint6,
        AtWaypoint6,
        MovingToWaypoint7,
        AtWaypoint7,
        MovingToWaypoint8,
        AtWaypoint8,
        MovingToWaypoint9,
        AtWaypoint9,
        MovingToWaypoint10,
        AtWaypoint10
    }

    /// <summary>
    /// Controls patient movement through four waypoints.
    /// </summary>
    public class WaypointPatientController : Script.Library.IScript
    {
        #region Configuration

        private const float MOVE_SPEED = 5.0f;
        private const float TURN_SPEED = 100.0f;
        private const float ARRIVAL_THRESHOLD = 0.1f;

        #endregion

        #region Waypoints

        private Vector3D _wp1;
        private Vector3D _wp2;
        private Vector3D _wp3;
        private Vector3D _wp4;
        private Vector3D _wp5;
        private Vector3D _wp6;
        private Vector3D _wp7;
        private Vector3D _wp8;
        private Vector3D _wp9;
        private Vector3D _wp10;

        #endregion

        #region State

        public string ObjectId { get; private set; }
        public bool IsMarkedForRemoval { get; private set; } = false;
        public WaypointTarget CurrentTarget { get; private set; } = WaypointTarget.AtWaypoint1;

        private Vector3D _targetPosition;
        private Vector3D _currentPosition;
        private bool _isInitialized = false;
        private bool _isMoving = false;
        private bool _needsInitialPositionSet = false;

        #endregion

        #region Constructors

        public WaypointPatientController() : base()
        {
            ObjectId = Obj.ObjectID;
        }

        public WaypointPatientController(string objectId) : base(objectId)
        {
            ObjectId = objectId;
        }

        #endregion

        #region IScript Lifecycle

        public override void Awake()
        {
            Logger.log(Logger.LogLevel.Info, 
                $"WaypointPatientController: Created for object {ObjectId}");
        }

        public override void Start() { }
        public override void Update(double dt) { }
        public override void LateUpdate(double dt) { }

        public override void Free()
        {
            Logger.log(Logger.LogLevel.Info, 
                $"WaypointPatientController: Freed - Object {ObjectId}");
            WaypointPatientControllerManager.UnregisterController(this);
        }

        #endregion

        #region Initialization

        /// <summary>
        /// Initialize with 4 waypoint positions.
        /// </summary>
        public void Initialize(Vector3D wp1, Vector3D wp2, Vector3D wp3, Vector3D wp4, Vector3D wp5, Vector3D wp6, Vector3D wp7, Vector3D wp8, Vector3D wp9, Vector3D wp10)
        {
            _wp1 = wp1;
            _wp2 = wp2;
            _wp3 = wp3;
            _wp4 = wp4;
            _wp5 = wp5;
            _wp6 = wp6;
            _wp7 = wp7;
            _wp8 = wp8;
            _wp9 = wp9;
            _wp10 = wp10;

            _currentPosition = wp1;
            _targetPosition = wp1;
            CurrentTarget = WaypointTarget.AtWaypoint1;
            _isInitialized = true;
            _isMoving = false;
            _needsInitialPositionSet = true;

            Logger.log(Logger.LogLevel.Info, 
                $"WaypointPatientController [{ObjectId}]: Initialized at WP1 ({wp1.x}, {wp1.y}, {wp1.z})");
        }

        #endregion

        #region Public API

        public void MarkForRemoval()
        {
            IsMarkedForRemoval = true;
        }

        public void MoveToTarget(WaypointTarget target) {
            CurrentTarget = target;
            switch (target) {
                case WaypointTarget.MovingToWaypoint2:
                _targetPosition = _wp2;
                break;

                case WaypointTarget.MovingToWaypoint3:
                    _targetPosition = _wp3;
                    break;

                case WaypointTarget.MovingToWaypoint4:
                    _targetPosition = _wp4;
                    break;

                case WaypointTarget.MovingToWaypoint5:
                    _targetPosition = _wp5;
                    break;

                case WaypointTarget.MovingToWaypoint6:
                    _targetPosition = _wp6;
                    break;

                case WaypointTarget.MovingToWaypoint7:
                    _targetPosition = _wp7;
                    break;

                case WaypointTarget.MovingToWaypoint8:
                    _targetPosition = _wp8;
                    break;

                case WaypointTarget.MovingToWaypoint9:
                    _targetPosition = _wp9;
                    break;

                case WaypointTarget.MovingToWaypoint10:
                    _targetPosition = _wp10;
                    break;

                default:
                    break;
            }
            _isMoving = true;
            Logger.log(Logger.LogLevel.Info, 
                $"WaypointPatientController [{ObjectId}]: Moving to WP2");
        }

                public void MoveToCounter()
        {
            if (!_isInitialized || IsMarkedForRemoval) return;

            Script.Library.Object patient = new Script.Library.Object(ObjectId);
            SkinnedMeshRenderer smr = patient.getComponent<SkinnedMeshRenderer>();


            if(CurrentTarget == WaypointTarget.AtWaypoint1)
            {
                CurrentTarget = WaypointTarget.MovingToWaypoint2;
                _targetPosition = _wp2;
                _isMoving = true;

                smr.PlayAnimation("walking_in_place");

                Logger.log(Logger.LogLevel.Info,
                    $"WaypointPatientController [{ObjectId}]: Moving to WP2");
            }
        }

        public void MoveToTesting()
        {
            if (!_isInitialized || IsMarkedForRemoval) return;

            Script.Library.Object patient = new Script.Library.Object(ObjectId);
            SkinnedMeshRenderer smr = patient.getComponent<SkinnedMeshRenderer>();


            if (CurrentTarget == WaypointTarget.AtWaypoint5)
            {
                CurrentTarget = WaypointTarget.MovingToWaypoint6;
                _targetPosition = _wp6;
                _isMoving = true;

                smr.PlayAnimation("walking_in_place");

                Logger.log(Logger.LogLevel.Info,
                    $"WaypointPatientController [{ObjectId}]: Moving to WP6");
            }
        }


        public void MoveToExit() {
            if (!_isInitialized || IsMarkedForRemoval) return;

            Script.Library.Object patient = new Script.Library.Object(ObjectId);
            SkinnedMeshRenderer smr = patient.getComponent<SkinnedMeshRenderer>();

            if (CurrentTarget == WaypointTarget.AtWaypoint5 || CurrentTarget == WaypointTarget.AtWaypoint6)
            {
                CurrentTarget = WaypointTarget.MovingToWaypoint7;
                _targetPosition = _wp7;
                _isMoving = true;

                smr.PlayAnimation("walking_in_place");

                Logger.log(Logger.LogLevel.Info,
                    $"WaypointPatientController [{ObjectId}]: Moving to WP7");
            }
        }

        /// <summary>
        /// Advance to the next waypoint. Called by spawner when 'N' is pressed.
        /// </summary>
        public void AdvanceToNextWaypoint()
        {
            if (!_isInitialized || IsMarkedForRemoval) return;

            Script.Library.Object patient = new Script.Library.Object(ObjectId);
            SkinnedMeshRenderer smr = patient.getComponent<SkinnedMeshRenderer>();

            switch (CurrentTarget)
            {
                case WaypointTarget.AtWaypoint1:
                    CurrentTarget = WaypointTarget.MovingToWaypoint2;
                    _targetPosition = _wp2;
                    _isMoving = true;

                    smr.PlayAnimation("walking_in_place");

                    Logger.log(Logger.LogLevel.Info, 
                        $"WaypointPatientController [{ObjectId}]: Moving to WP2");
                    break;

                case WaypointTarget.AtWaypoint2:
                    CurrentTarget = WaypointTarget.MovingToWaypoint3;
                    _targetPosition = _wp3;
                    _isMoving = true;

                    smr.PlayAnimation("walking_in_place");

                    Logger.log(Logger.LogLevel.Info, 
                        $"WaypointPatientController [{ObjectId}]: Moving to WP3");
                    break;

                case WaypointTarget.AtWaypoint3:
                    CurrentTarget = WaypointTarget.MovingToWaypoint4;
                    _targetPosition = _wp4;
                    _isMoving = true;

                    smr.PlayAnimation("walking_in_place");

                    Logger.log(Logger.LogLevel.Info, 
                        $"WaypointPatientController [{ObjectId}]: Moving to WP4");
                    break;

                case WaypointTarget.AtWaypoint4:
                    CurrentTarget = WaypointTarget.MovingToWaypoint5;
                    _targetPosition = _wp5;
                    _isMoving = true;

                    smr.PlayAnimation("walking_in_place");

                    Logger.log(Logger.LogLevel.Info,
                        $"WaypointPatientController [{ObjectId}]: Moving to WP5");
                    break;

                case WaypointTarget.AtWaypoint5:
                    CurrentTarget = WaypointTarget.MovingToWaypoint6;
                    _targetPosition = _wp6;
                    _isMoving = true;

                    smr.PlayAnimation("walking_in_place");

                    Logger.log(Logger.LogLevel.Info,
                        $"WaypointPatientController [{ObjectId}]: Moving to WP6");
                    break;

                case WaypointTarget.AtWaypoint6:
                    CurrentTarget = WaypointTarget.MovingToWaypoint7;
                    _targetPosition = _wp7;
                    _isMoving = true;

                    smr.PlayAnimation("walking_in_place");

                    Logger.log(Logger.LogLevel.Info,
                        $"WaypointPatientController [{ObjectId}]: Moving to WP7");
                    break;

                case WaypointTarget.AtWaypoint7:
                    CurrentTarget = WaypointTarget.MovingToWaypoint8;
                    _targetPosition = _wp8;
                    _isMoving = true;

                    smr.PlayAnimation("walking_in_place");

                    Logger.log(Logger.LogLevel.Info,
                        $"WaypointPatientController [{ObjectId}]: Moving to WP8");
                    break;

                case WaypointTarget.AtWaypoint8:
                    CurrentTarget = WaypointTarget.MovingToWaypoint9;
                    _targetPosition = _wp9;
                    _isMoving = true;

                    smr.PlayAnimation("walking_in_place");

                    Logger.log(Logger.LogLevel.Info,
                        $"WaypointPatientController [{ObjectId}]: Moving to WP9");
                    break;

                case WaypointTarget.AtWaypoint9:
                    CurrentTarget = WaypointTarget.MovingToWaypoint10;
                    _targetPosition = _wp10;
                    _isMoving = true;

                    smr.PlayAnimation("walking_in_place");

                    Logger.log(Logger.LogLevel.Info,
                        $"WaypointPatientController [{ObjectId}]: Moving to WP10");
                    break;

                case WaypointTarget.AtWaypoint10:
                    Logger.log(Logger.LogLevel.Info,
                        $"WaypointPatientController [{ObjectId}]: Moving to WP5");
                    break;

                default:
                    Logger.log(Logger.LogLevel.Info, 
                        $"WaypointPatientController [{ObjectId}]: Still moving, please wait");
                    break;
            }
        }

        /// <summary>
        /// Update movement. Called by spawner each frame.
        /// </summary>
        public void UpdateMovement(double dt)
        {
            if (!_isInitialized || IsMarkedForRemoval) return;

            UpdateTurn180(dt);

            if (_needsInitialPositionSet)
            {
                if (SafeApplyPosition(_currentPosition))
                {
                    _needsInitialPositionSet = false;
                }
                else
                {
                    MarkForRemoval();
                    return;
                }
            }

            if (_isMoving && !_isTurning180)
            {
                PerformSmoothMovement(dt);
            }
        }

        #endregion

        #region Funny Math implemetation
        private static float Clamp01(float v)
        {
            if (v < 0f) return 0f;
            if (v > 1f) return 1f;
            return v;
        }

        private static float Repeat(float t, float length)
        {
            return t - (float)Math.Floor(t / length) * length;
        }

        // Returns shortest signed angle difference in degrees in range [-180, 180]
        private static float DeltaAngle(float current, float target)
        {
            float delta = Repeat(target - current, 360f);
            if (delta > 180f) delta -= 360f;
            return delta;
        }

        // Moves current toward target by maxDelta degrees, handling wrap-around
        private static float MoveTowardsAngle(float current, float target, float maxDelta)
        {
            float delta = DeltaAngle(current, target);
            if (delta > maxDelta) delta = maxDelta;
            else if (delta < -maxDelta) delta = -maxDelta;
            return current + delta;
        }

        private static float Abs(float v) => (v < 0f) ? -v : v;

        // dir assumed normalized-ish and flat (y=0). Forward is -Z (matches your getFlatForward)
        private static float DirectionToYaw(Vector3D dir)
        {
            return (float)(Math.Atan2(dir.x, -dir.z) * (180.0 / Math.PI));
        }
#endregion

        #region Movement
        private bool _isTurning180 = false;
        private float _target180Yaw = 0f;

        public void StartTurn180()
        {
            Script.Library.Object obj = new Script.Library.Object(ObjectId);
            TransformComponent transform = obj.getComponent<TransformComponent>();

            if (transform == null) return;

            Transformation trf = transform.Transformation;

            // Target is current yaw + 180 (wrapped safely by MoveTowardsAngle)
            _target180Yaw = trf.Rotation.y + 180f;
            _isTurning180 = true;
        }

        private void UpdateTurn180(double dt)
        {
            if (!_isTurning180) return;

            Script.Library.Object obj = new Script.Library.Object(ObjectId);
            TransformComponent transform = obj.getComponent<TransformComponent>();

            if (transform == null) return;

            Transformation trf = transform.Transformation;

            float maxStep = TURN_SPEED * (float)dt;
            float newYaw = MoveTowardsAngle(trf.Rotation.y, _target180Yaw, maxStep);

            trf.Rotation.y = newYaw;
            transform.Transformation = trf;

            // Stop turning when close enough
            if (Abs(DeltaAngle(newYaw, _target180Yaw)) < 0.5f)
            {
                trf.Rotation.y = _target180Yaw;
                transform.Transformation = trf;
                _isTurning180 = false;
            }
        }

        private void PerformSmoothMovement(double dt)
        {
            Vector3D direction = _targetPosition - _currentPosition;
            float distance = direction.magnitude();

            if (distance <= ARRIVAL_THRESHOLD)
            {
                _currentPosition = _targetPosition;
                _isMoving = false;

                if (!SafeApplyPosition(_currentPosition))
                {
                    MarkForRemoval();
                    return;
                }

                OnArriveAtWaypoint();
                return;
            }

            // --- Rotate towards waypoint ---
            Vector3D normalizedDir = direction.normalize(); // you already have this

            Script.Library.Object obj = new Script.Library.Object(ObjectId);
            TransformComponent transform = obj.getComponent<TransformComponent>();
                
            Transformation trf = transform.Transformation;

            float targetYaw = DirectionToYaw(-normalizedDir);
            float currentYaw = trf.Rotation.y;

            float maxYawStep = TURN_SPEED * (float)dt;
            float newYaw = MoveTowardsAngle(currentYaw, targetYaw, maxYawStep);
            trf.Rotation.y = newYaw;

            transform.Transformation = trf;


            // ---- Move toward waypoint ----
            float moveDistance = MOVE_SPEED * (float)dt;
            if (moveDistance > distance) moveDistance = distance;

            _currentPosition = _currentPosition + normalizedDir * moveDistance;

            if (!SafeApplyPosition(_currentPosition))
            {
                MarkForRemoval();
            }
        }

        private bool SafeApplyPosition(Vector3D position)
        {
            if (IsMarkedForRemoval) return false;

            try
            {
                Script.Library.Object obj = new Script.Library.Object(ObjectId);
                TransformComponent transform = obj.getComponent<TransformComponent>();
                
                if (transform != null)
                {
                    Transformation trf = transform.Transformation;
                    trf.position = position;
                    transform.Transformation = trf;
                    return true;
                }
                return true;
            }
            catch (Exception)
            {
                return false;
            }
        }

        private void OnArriveAtWaypoint()
        {
            Script.Library.Object patient = new Script.Library.Object(ObjectId);
            SkinnedMeshRenderer smr = patient.getComponent<SkinnedMeshRenderer>();

            switch (CurrentTarget)
            {
                case WaypointTarget.MovingToWaypoint2:
                    CurrentTarget = WaypointTarget.AtWaypoint2;

                    smr.PlayAnimation("idle");

                    Logger.log(Logger.LogLevel.Info, 
                        $"WaypointPatientController [{ObjectId}]: Arrived at WP2. Press 'N' for WP3.");
                    AdvanceToNextWaypoint();
                    break;

                case WaypointTarget.MovingToWaypoint3:
                    CurrentTarget = WaypointTarget.AtWaypoint3;

                    smr.PlayAnimation("idle");

                    Logger.log(Logger.LogLevel.Info, 
                        $"WaypointPatientController [{ObjectId}]: Arrived at WP3. Press 'N' for WP4.");
                    AdvanceToNextWaypoint();
                    break;

                case WaypointTarget.MovingToWaypoint4:
                    CurrentTarget = WaypointTarget.AtWaypoint4;

                    smr.PlayAnimation("idle");
                    
                    Logger.log(Logger.LogLevel.Info, 
                        $"WaypointPatientController [{ObjectId}]: Arrived at final WP4!");
                    AdvanceToNextWaypoint();
                    break;

                case WaypointTarget.MovingToWaypoint5:
                    CurrentTarget = WaypointTarget.AtWaypoint5;

                    smr.PlayAnimation("idle");
                    
                    Logger.log(Logger.LogLevel.Info,
                        $"WaypointPatientController [{ObjectId}]: Arrived at final WP5!");
                    break;

                case WaypointTarget.MovingToWaypoint6:
                    CurrentTarget = WaypointTarget.AtWaypoint6;
                    
                    StartTurn180();

                    smr.PlayAnimation("idle");
                    
                    Logger.log(Logger.LogLevel.Info,
                        $"WaypointPatientController [{ObjectId}]: Arrived at final WP6!");
                    break;

                case WaypointTarget.MovingToWaypoint7:
                    CurrentTarget = WaypointTarget.AtWaypoint7;

                    smr.PlayAnimation("idle");
                    
                    Logger.log(Logger.LogLevel.Info,
                        $"WaypointPatientController [{ObjectId}]: Arrived at final WP7!");
                    AdvanceToNextWaypoint();
                    break;

                case WaypointTarget.MovingToWaypoint8:
                    CurrentTarget = WaypointTarget.AtWaypoint8;

                    smr.PlayAnimation("idle");
                    
                    Logger.log(Logger.LogLevel.Info,
                        $"WaypointPatientController [{ObjectId}]: Arrived at final WP8!");
                    AdvanceToNextWaypoint();
                    break;

                case WaypointTarget.MovingToWaypoint9:
                    CurrentTarget = WaypointTarget.AtWaypoint9;

                    smr.PlayAnimation("idle");
                    
                    Logger.log(Logger.LogLevel.Info,
                        $"WaypointPatientController [{ObjectId}]: Arrived at final WP9!");
                    AdvanceToNextWaypoint();
                    break;

                case WaypointTarget.MovingToWaypoint10:
                    CurrentTarget = WaypointTarget.AtWaypoint10;

                    smr.PlayAnimation("idle");
                    
                    Logger.log(Logger.LogLevel.Info,
                        $"WaypointPatientController [{ObjectId}]: Arrived at final WP10!");
                    break;
            }
        }

        #endregion
    }
}
