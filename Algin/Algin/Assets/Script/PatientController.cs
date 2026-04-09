// ============================================================================
// PatientController.cs - Patient entity with state machine and mock movement
// ============================================================================

using Script.Library;

namespace Script
{
    #region Patient State Enum

    /// <summary>
    /// Possible states for a patient entity.
    /// </summary>
    public enum PatientState
    {
        /// <summary>Just created, at entrance.</summary>
        Spawning,
        /// <summary>Waiting in the lobby queue.</summary>
        Queueing,
        /// <summary>Walking to the diagnosis chair.</summary>
        MovingToDiagnosis,
        /// <summary>Sitting in front of the player for diagnosis.</summary>
        InDiagnosis,
        /// <summary>Leaving the map after diagnosis.</summary>
        Exiting
    }

    #endregion

    /// <summary>
    /// Patient entity controller with state machine and mock movement.
    /// </summary>
    public class PatientController : Script.Library.IScript
    {
        #region Static Positions (Mock Coordinates)

        /// <summary>Spawn/entrance position.</summary>
        public static readonly Vector3D EntrancePos = new Vector3D(0, 0, 0);
        
        /// <summary>Waiting room/queue position.</summary>
        public static readonly Vector3D QueuePos = new Vector3D(5, 0, 5);
        
        /// <summary>Diagnosis chair position.</summary>
        public static readonly Vector3D DiagnosisPos = new Vector3D(10, 0, 10);
        
        /// <summary>Exit position.</summary>
        public static readonly Vector3D ExitPos = new Vector3D(20, 0, 0);

        #endregion

        #region Configuration

        /// <summary>Movement speed in units per second.</summary>
        private const float MOVE_SPEED = 2.0f;
        
        /// <summary>Distance threshold to consider "arrived" at target.</summary>
        private const float ARRIVAL_THRESHOLD = 0.1f;

        #endregion

        #region Patient Data

        /// <summary>
        /// The patient data for this entity.
        /// </summary>
        public PatientData Data { get; private set; }

        /// <summary>
        /// Whether this controller has been initialized with data.
        /// </summary>
        public bool IsInitialized { get; private set; } = false;

        /// <summary>
        /// The illness this patient has (for reference).
        /// </summary>
        public Illness AssignedIllness { get; private set; }

        #endregion

        #region State Machine

        /// <summary>
        /// Current state of this patient.
        /// </summary>
        public PatientState CurrentState { get; private set; } = PatientState.Spawning;

        /// <summary>
        /// Target position for mock movement.
        /// </summary>
        public Vector3D TargetPosition { get; private set; }

        /// <summary>
        /// Current position (for mock movement when no transform available).
        /// </summary>
        private Vector3D _currentPosition;

        #endregion

        #region IScript Lifecycle

        public override void Awake()
        {
            _currentPosition = EntrancePos;
            TargetPosition = EntrancePos;
            
            Logger.log(Logger.LogLevel.Info, 
                $"PatientController: Created on object {Obj.ObjectID}");
        }

        /// <summary>
        /// Constructor for creating a controller bound to a specific object ID.
        /// </summary>
        /// <param name="objectId">The object ID to bind to.</param>
        public PatientController(string objectId) : base(objectId)
        {
            _currentPosition = EntrancePos;
            TargetPosition = EntrancePos;
        }

        /// <summary>
        /// Default constructor.
        /// </summary>
        public PatientController() : base()
        {
            _currentPosition = EntrancePos;
            TargetPosition = EntrancePos;
        }

        public override void Start() { }

        public override void Update(double dt)
        {
            if (!IsInitialized) return;

            // Perform mock movement toward target
            UpdateMovement(dt);

            // Check for state transitions based on arrival
            CheckArrival();
        }

        public override void LateUpdate(double dt) { }

        public override void Free()
        {
            Logger.log(Logger.LogLevel.Info, 
                $"PatientController: Freed - Patient '{Data.PatientName}'");
        }

        #endregion

        #region Movement

        /// <summary>
        /// Update position using mock movement (linear interpolation).
        /// </summary>
        private void UpdateMovement(double dt)
        {
            Vector3D direction = TargetPosition - _currentPosition;
            float distance = direction.magnitude();

            if (distance < ARRIVAL_THRESHOLD)
            {
                // Already at target
                return;
            }

            // Move toward target
            Vector3D normalizedDir = direction.normalize();
            float moveDistance = MOVE_SPEED * (float)dt;
            
            // Don't overshoot
            if (moveDistance > distance)
            {
                moveDistance = distance;
            }

            _currentPosition = _currentPosition + normalizedDir * moveDistance;

            // Update actual transform if available
            TransformComponent transform = Obj.getComponent<TransformComponent>();
            if (transform != null)
            {
                Transformation trf = transform.Transformation;
                trf.position = _currentPosition;
                transform.Transformation = trf;
            }
        }

        /// <summary>
        /// Check if we've arrived at the target and handle state transitions.
        /// </summary>
        private void CheckArrival()
        {
            Vector3D direction = TargetPosition - _currentPosition;
            float distance = direction.magnitude();

            if (distance >= ARRIVAL_THRESHOLD)
            {
                return; // Not arrived yet
            }

            // Handle arrival based on current state
            switch (CurrentState)
            {
                case PatientState.Spawning:
                    // Arrived at entrance, transition to queueing
                    SetState(PatientState.Queueing);
                    break;

                case PatientState.MovingToDiagnosis:
                    // Arrived at diagnosis chair
                    CurrentState = PatientState.InDiagnosis;
                    Logger.log(Logger.LogLevel.Info, 
                        $"PatientController: '{Data.PatientName}' arrived at diagnosis chair");
                    break;

                case PatientState.Exiting:
                    // Arrived at exit - could destroy object here
                    Logger.log(Logger.LogLevel.Info, 
                        $"PatientController: '{Data.PatientName}' has exited the map");
                    break;
            }
        }

        #endregion

        #region State Control

        /// <summary>
        /// Change the patient's state and update target position.
        /// </summary>
        /// <param name="newState">The new state to transition to.</param>
        public void SetState(PatientState newState)
        {
            PatientState oldState = CurrentState;
            CurrentState = newState;

            // Set target position based on new state
            switch (newState)
            {
                case PatientState.Spawning:
                    TargetPosition = EntrancePos;
                    break;

                case PatientState.Queueing:
                    TargetPosition = QueuePos;
                    // Note: PatientManager uses static registration via RegisterPatient(objectId, data, illness)
                    // which is called from PatientSpawner when spawning. No need to register again here.
                    Logger.log(Logger.LogLevel.Info, 
                        $"PatientController: '{Data.PatientName}' entered queue");
                    break;

                case PatientState.MovingToDiagnosis:
                    TargetPosition = DiagnosisPos;
                    break;

                case PatientState.InDiagnosis:
                    TargetPosition = DiagnosisPos;
                    break;

                case PatientState.Exiting:
                    TargetPosition = ExitPos;
                    break;
            }

            Logger.log(Logger.LogLevel.Info, 
                $"PatientController: '{Data.PatientName}' state changed: {oldState} -> {newState}");
            Logger.log(Logger.LogLevel.Info, 
                $"  Target position: {TargetPosition}");
        }

        #endregion

        #region Public API

        /// <summary>
        /// Initialize this patient with data.
        /// </summary>
        /// <param name="data">The patient data to assign.</param>
        public void Initialize(PatientData data)
        {
            Data = data;
            IsInitialized = true;
            
            Logger.log(Logger.LogLevel.Info, 
                $"PatientController: Initialized with patient '{data.PatientName}' (Age: {data.Age}, {data.Gender})");
            
            // Start in spawning state, then transition to queueing
            SetState(PatientState.Queueing);
        }

        /// <summary>
        /// Initialize with data and a known illness assignment.
        /// </summary>
        /// <param name="data">The patient data.</param>
        /// <param name="illness">The illness this patient has.</param>
        public void Initialize(PatientData data, Illness illness)
        {
            Data = data;
            AssignedIllness = illness;
            IsInitialized = true;
            
            Logger.log(Logger.LogLevel.Info, 
                $"PatientController: Initialized - '{data.PatientName}' with {illness.Name}");
            
            // Start in spawning state, then transition to queueing
            SetState(PatientState.Queueing);
        }

        /// <summary>
        /// Get a summary of this patient for display.
        /// </summary>
        public string GetSummary()
        {
            if (!IsInitialized)
                return "Patient not initialized";
            
            return $"{Data.PatientName} ({Data.Age}yo {Data.Gender}) - State: {CurrentState}";
        }

        /// <summary>
        /// Get the current position of this patient.
        /// </summary>
        public Vector3D GetPosition()
        {
            return _currentPosition;
        }

        #endregion
    }
}
