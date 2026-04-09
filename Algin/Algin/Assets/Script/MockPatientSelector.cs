// ============================================================================
// MockPatientSelector.cs - Mock implementation for testing diagnosis logic
// ============================================================================

using Script.Library;
using System.Collections.Generic;

namespace Script
{
    /// <summary>
    /// Mock patient selector for testing diagnosis logic in isolation.
    /// Press SPACEBAR to simulate selecting a patient with Pneumonia symptoms.
    /// </summary>
    public class MockPatientSelector : Script.Library.IScript, IPatientSelector
    {
        #region State

        private PatientData? _selectedPatient = null;
        private bool _isActive = true;

        #endregion

        #region IPatientSelector Implementation

        public bool HasSelection => _selectedPatient.HasValue;

        public PatientData? GetSelectedPatient()
        {
            return _selectedPatient;
        }

        public void ClearSelection()
        {
            _selectedPatient = null;
        }

        #endregion



        #region IScript Lifecycle

        public override void Awake()
        {
            Logger.log(Logger.LogLevel.Info, 
                "MockPatientSelector: Initialized - Press SPACEBAR to simulate patient selection");
        }

        public override void Start() { }

        public override void Update(double dt)
        {
            if (!_isActive) return;

            // Listen for Spacebar press
            if (Input.isKeyClicked(Input.KeyCode.kSPACE))
            {
                Logger.log(Logger.LogLevel.Info, 
                    "========================================");
                Logger.log(Logger.LogLevel.Info, 
                    "MockPatientSelector: SPACEBAR pressed!");
                Logger.log(Logger.LogLevel.Info, 
                    "Simulating patient selection...");
                Logger.log(Logger.LogLevel.Info, 
                    "========================================");

                // Create dummy patient with PNEUMONIA symptoms
                PatientData dummyPatient = CreatePneumoniaPatient();
                _selectedPatient = dummyPatient;

                // Trigger diagnosis
                TriggerDiagnosis(dummyPatient);
            }
        }

        public override void LateUpdate(double dt) { }

        public override void Free()
        {
            Logger.log(Logger.LogLevel.Info, "MockPatientSelector: Freed");
        }

        #endregion

        #region Mock Data Generation

        /// <summary>
        /// Creates a dummy patient with Pneumonia symptoms for testing.
        /// </summary>
        private PatientData CreatePneumoniaPatient()
        {
            return new PatientData
            {
                PatientID = "TEST-001",
                PatientName = "John Doe (Mock)",
                Age = 45,
                Gender = Gender.Male,
                
                Symptoms = new PatientSymptoms
                {
                    SymptomDurationDays = 21,
                    MainComplaint = "I've been coughing for over 2 weeks",
                    AdditionalSymptoms = new List<string> { "feverish", "hurts to breathe" }
                },
                
                Scans = new ScanResults
                {
                    Lungs = LungState.LooksInfected,
                    Heart = HeartState.SlightlyRed,
                    Digestive = DigestiveState.Normal,
                    Skin = SkinState.Normal
                },
                
                Labs = new LabResults
                {
                    Blood = new BloodResults 
                    { 
                        WBC = WBCLevel.Elevated, 
                        RBC = RBCLevel.Normal, 
                        Stress = StressMarkers.Normal 
                    },
                    Urine = UrineState.Normal,
                    HairSkin = HairSkinTest.Normal
                }
            };
        }

        /// <summary>
        /// Creates a dummy patient with Mild Flu symptoms.
        /// </summary>
        public static PatientData CreateMildFluPatient()
        {
            return new PatientData
            {
                PatientID = "TEST-002",
                PatientName = "Jane Smith (Mock)",
                Age = 28,
                Gender = Gender.Female,
                
                Symptoms = new PatientSymptoms
                {
                    SymptomDurationDays = 2,
                    MainComplaint = "I've been coughing and sneezing",
                    AdditionalSymptoms = new List<string> { "tired", "runny nose" }
                },
                
                Scans = new ScanResults
                {
                    Lungs = LungState.MildIrritation,
                    Heart = HeartState.Normal,
                    Digestive = DigestiveState.Normal,
                    Skin = SkinState.Normal
                },
                
                Labs = new LabResults
                {
                    Blood = new BloodResults 
                    { 
                        WBC = WBCLevel.Normal, 
                        RBC = RBCLevel.Normal, 
                        Stress = StressMarkers.Normal 
                    },
                    Urine = UrineState.Normal,
                    HairSkin = HairSkinTest.Normal
                }
            };
        }

        /// <summary>
        /// Creates a dummy patient with Severe Allergy symptoms.
        /// </summary>
        public static PatientData CreateSevereAllergyPatient()
        {
            return new PatientData
            {
                PatientID = "TEST-003",
                PatientName = "Bob Wilson (Mock)",
                Age = 35,
                Gender = Gender.Male,
                
                Symptoms = new PatientSymptoms
                {
                    SymptomDurationDays = 1,
                    MainComplaint = "My skin is itchy and face feels swollen",
                    AdditionalSymptoms = new List<string> { "trouble breathing", "hives" }
                },
                
                Scans = new ScanResults
                {
                    Lungs = LungState.LooksInfected,
                    Heart = HeartState.VeryRed,
                    Digestive = DigestiveState.Normal,
                    Skin = SkinState.RashSwelling
                },
                
                Labs = new LabResults
                {
                    Blood = new BloodResults 
                    { 
                        WBC = WBCLevel.Normal, 
                        RBC = RBCLevel.Normal, 
                        Stress = StressMarkers.Normal 
                    },
                    Urine = UrineState.Normal,
                    HairSkin = HairSkinTest.ImmuneReaction
                }
            };
        }

        /// <summary>
        /// Creates a dummy patient with Drug-Induced Cardiac Stress symptoms.
        /// </summary>
        public static PatientData CreateCardiacStressPatient()
        {
            return new PatientData
            {
                PatientID = "TEST-004",
                PatientName = "Alex Chen (Mock)",
                Age = 22,
                Gender = Gender.Male,
                
                Symptoms = new PatientSymptoms
                {
                    SymptomDurationDays = 2,
                    MainComplaint = "My chest hurts and heart feels irregular",
                    AdditionalSymptoms = new List<string> { "feel weak", "palpitations" }
                },
                
                Scans = new ScanResults
                {
                    Lungs = LungState.LooksInfected,
                    Heart = HeartState.VeryRed,
                    Digestive = DigestiveState.Normal,
                    Skin = SkinState.Normal
                },
                
                Labs = new LabResults
                {
                    Blood = new BloodResults 
                    { 
                        WBC = WBCLevel.Normal, 
                        RBC = RBCLevel.Normal, 
                        Stress = StressMarkers.Elevated 
                    },
                    Urine = UrineState.ToxicMarkers,
                    HairSkin = HairSkinTest.Normal
                }
            };
        }

        #endregion

        #region Diagnosis Trigger

        private void TriggerDiagnosis(PatientData patient)
        {
            if (DiagnosisSystem.Instance == null)
            {
                Logger.log(Logger.LogLevel.Error, 
                    "MockPatientSelector: DiagnosisSystem.Instance is NULL!");
                Logger.log(Logger.LogLevel.Error, 
                    "  Make sure DiagnosisSystem is attached to a GameObject in the scene.");
                return;
            }

            Logger.log(Logger.LogLevel.Info, 
                "MockPatientSelector: Passing patient data to DiagnosisSystem...");
            
            Illness result = DiagnosisSystem.Instance.Diagnose(patient);

            Logger.log(Logger.LogLevel.Info, 
                "========================================");
            if (result != null)
            {
                Logger.log(Logger.LogLevel.Info, 
                    $"FINAL DIAGNOSIS: {result.Name}");
                Logger.log(Logger.LogLevel.Info, 
                    $"MEDICATION: {result.Medication}");
                Logger.log(Logger.LogLevel.Info, 
                    $"SUBSTANCE EXPOSURE: {(result.SuspectedSubstanceExposure ? "YES - Flag patient" : "No")}");
            }
            else
            {
                Logger.log(Logger.LogLevel.Warning, 
                    "FINAL DIAGNOSIS: Unable to determine illness");
            }
            Logger.log(Logger.LogLevel.Info, 
                "========================================");
        }

        #endregion

        #region Public Controls

        public void SetActive(bool active)
        {
            _isActive = active;
            Logger.log(Logger.LogLevel.Info, 
                $"MockPatientSelector: Active = {active}");
        }

        #endregion
    }
}
