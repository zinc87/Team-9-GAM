// ============================================================================
// PatientDebugUI.cs - Visual debug label showing patient name and state
// ============================================================================

using Script.Library;

namespace Script
{
    /// <summary>
    /// Displays a debug label above the patient showing their name and current state.
    /// Requires PatientController and TextMeshUIComponent on the same object.
    /// </summary>
    public class PatientDebugUI : Script.Library.IScript
    {
        #region Colors

        /// <summary>White color for Spawning/Queueing states.</summary>
        private static readonly Color ColorWhite = new Color(1.0f, 1.0f, 1.0f, 1.0f);
        
        /// <summary>Green color for MovingToDiagnosis state.</summary>
        private static readonly Color ColorGreen = new Color(0.2f, 1.0f, 0.2f, 1.0f);
        
        /// <summary>Yellow color for InDiagnosis state.</summary>
        private static readonly Color ColorYellow = new Color(1.0f, 1.0f, 0.2f, 1.0f);
        
        /// <summary>Red color for Exiting state.</summary>
        private static readonly Color ColorRed = new Color(1.0f, 0.2f, 0.2f, 1.0f);

        #endregion

        #region References

        private PatientController _patientController;
        private TextMeshUIComponent _textComponent;
        private bool _isSetup = false;

        #endregion

        #region Constructors

        /// <summary>
        /// Constructor for creating bound to a specific object ID.
        /// </summary>
        public PatientDebugUI(string objectId) : base(objectId) { }

        /// <summary>
        /// Default constructor.
        /// </summary>
        public PatientDebugUI() : base() { }

        #endregion

        #region IScript Lifecycle

        public override void Awake()
        {
            Logger.log(Logger.LogLevel.Info, 
                $"PatientDebugUI: Created on object {Obj.ObjectID}");
        }

        public override void Start()
        {
            // Try to get required components
            SetupComponents();
        }

        public override void Update(double dt)
        {
            if (!_isSetup)
            {
                SetupComponents();
                if (!_isSetup) return;
            }

            UpdateLabel();
        }

        public override void LateUpdate(double dt) { }

        public override void Free()
        {
            Logger.log(Logger.LogLevel.Info, 
                $"PatientDebugUI: Freed");
        }

        #endregion

        #region Setup

        /// <summary>
        /// Get references to required components.
        /// </summary>
        private void SetupComponents()
        {
            _textComponent = Obj.getComponent<TextMeshUIComponent>();
            
            if (_textComponent == null)
            {
                // Text component not found - will use console logging instead
                Logger.log(Logger.LogLevel.Warning, 
                    $"PatientDebugUI: No TextMeshUIComponent on object {Obj.ObjectID} - using console output");
            }

            _isSetup = true;
        }

        /// <summary>
        /// Set the patient controller reference (called externally).
        /// </summary>
        public void SetPatientController(PatientController controller)
        {
            _patientController = controller;
        }

        #endregion

        #region Label Update

        private PatientState _lastLoggedState = PatientState.Spawning;
        private bool _hasLoggedOnce = false;

        /// <summary>
        /// Update the label text and color based on patient state.
        /// </summary>
        private void UpdateLabel()
        {
            if (_patientController == null || !_patientController.IsInitialized)
            {
                return;
            }

            // Get patient info
            string patientName = _patientController.Data.PatientName;
            PatientState currentState = _patientController.CurrentState;
            
            // Build label text: "Name\nSTATE"
            string labelText = $"{patientName}\n{currentState.ToString().ToUpper()}";
            
            // Get color based on state
            Color labelColor = GetColorForState(currentState);

            // If we have a text component, update it visually
            if (_textComponent != null)
            {
                TextMeshUIComponentData textData = _textComponent.Data;
                textData.text = labelText;
                textData.color = labelColor;
                _textComponent.Data = textData;
            }
            else
            {
                // Fallback: Log to console when state changes
                if (!_hasLoggedOnce || currentState != _lastLoggedState)
                {
                    string colorName = GetColorName(currentState);
                    Logger.log(Logger.LogLevel.Info, 
                        $"[DEBUG UI] {patientName}: {currentState.ToString().ToUpper()} ({colorName})");
                    _lastLoggedState = currentState;
                    _hasLoggedOnce = true;
                }
            }
        }

        /// <summary>
        /// Get color name for logging.
        /// </summary>
        private string GetColorName(PatientState state)
        {
            switch (state)
            {
                case PatientState.Spawning:
                case PatientState.Queueing:
                    return "White";
                case PatientState.MovingToDiagnosis:
                    return "Green";
                case PatientState.InDiagnosis:
                    return "Yellow";
                case PatientState.Exiting:
                    return "Red";
                default:
                    return "White";
            }
        }

        /// <summary>
        /// Get color for the given patient state.
        /// </summary>
        private Color GetColorForState(PatientState state)
        {
            switch (state)
            {
                case PatientState.Spawning:
                case PatientState.Queueing:
                    return ColorWhite;
                case PatientState.MovingToDiagnosis:
                    return ColorGreen;
                case PatientState.InDiagnosis:
                    return ColorYellow;
                case PatientState.Exiting:
                    return ColorRed;
                default:
                    return ColorWhite;
            }
        }

        #endregion
    }
}

