// ============================================================================
// PatientSpawner.cs - Spawns patients with random data every 10 seconds
// ============================================================================

using Script.Library;
using System;
using System.Collections.Generic;

namespace Script
{
    /// <summary>
    /// Simple spawner that creates patient objects every 10 seconds.
    /// </summary>
    public class PatientSpawner : Script.Library.IScript
    {
        #region Configuration

        /// <summary>
        /// Time between spawns in seconds.
        /// </summary>
        private const double SPAWN_INTERVAL = 10.0;

        #endregion

        #region State

        //private double _timeSinceLastSpawn = 0.0;
        public static int _patientCounter = 0;
        public static Random _rng = new Random();

        // Name pools for random generation
        private static readonly string[] _maleNames = 
        {
            "James", "John", "Robert", "Michael", "David", 
            "William", "Richard", "Joseph", "Thomas", "Charles"
        };
        
        private static readonly string[] _femaleNames = 
        {
            "Mary", "Patricia", "Jennifer", "Linda", "Elizabeth",
            "Barbara", "Susan", "Jessica", "Sarah", "Karen"
        };
        
        private static readonly string[] _lastNames = 
        {
            "Smith", "Johnson", "Williams", "Brown", "Jones",
            "Garcia", "Miller", "Davis", "Rodriguez", "Martinez"
        };

        #endregion

        public static PatientSpawner Instance;

        #region IScript Lifecycle

        public override void Awake()
        {
            if (Instance == null)
                Instance = this;
            else
                Logger.log(Logger.LogLevel.Warning, "Duplicate PatientSpawner");

            _patientCounter = 0;
        }

        public override void Start() 
        {
            // Spawn first patient immediately
            //SpawnPatient();
        }

        public override void Update(double dt)
        {
            // _timeSinceLastSpawn += dt;

            // if (_timeSinceLastSpawn >= SPAWN_INTERVAL)
            // {
            //     _timeSinceLastSpawn = 0.0;
            //     SpawnPatient();
            // }
        }

        public override void LateUpdate(double dt) { }

        public override void Free()
        {
            Logger.log(Logger.LogLevel.Info, 
                $"PatientSpawner: Freed - Spawned {_patientCounter} patients total");
        }

        #endregion

        #region Spawning Logic

        /// <summary>
        /// Spawn a new patient with random data.
        /// </summary>
        public void SpawnPatient()
        {
            _patientCounter++;

            // 1. Create parent patient object
            string newObjectId = Script.Library.Object.createPatient();
            Logger.log(Logger.LogLevel.Info, $"[PatientSpawner] Created parent patient with ID: {newObjectId}");
            Script.Library.Object parent = new Script.Library.Object(newObjectId);
            
            // 2. Create child object with BoxCollider for collision
            Logger.log(Logger.LogLevel.Info, "[PatientSpawner] Creating child object...");
            Script.Library.Object child = parent.CreateChild();
            Logger.log(Logger.LogLevel.Info, $"[PatientSpawner] Created child with ID: {child?.ObjectID ?? "null"}");
            child.addComponent(ComponentTypes.Transform);
            child.addComponent(ComponentTypes.BoxCollider);
            Logger.log(Logger.LogLevel.Info, "[PatientSpawner] Added Transform and BoxCollider to child");
            
            // 3. Set BoxCollider to NPC layer
            var boxCollider = child.getComponent<BoxColliderComponent>();
            if (boxCollider != null)
            {
                BoxColliderComponentData bcData = boxCollider.Data;
                bcData.layerMask = (uint)CollisionLayers.Layer_NPC;
                bcData.drawCollider = false;
                boxCollider.Data = bcData;
                Logger.log(Logger.LogLevel.Info, "[PatientSpawner] Set BoxCollider to NPC layer");
            }
            
            // 4. Generate random patient data
            PatientData patientData = GenerateRandomPatientData();
            
            // 5. Get random illness and generate matching scan/lab data
            Illness illness = IllnessDatabase.GetRandomIllness();
            patientData = ApplyIllnessToPatient(patientData, illness);

            // Note: In a full implementation, you would attach PatientController 
            // to the new object and call Initialize(). Since script attachment 
            // requires engine support, we log the data for now.
            // 
            // Future implementation:
            // var controller = newObject.AddComponent<PatientController>();
            // controller.Initialize(patientData, illness);

            PatientManager.RegisterPatient(newObjectId, patientData, illness);
        }

        /// <summary>
        /// Generate random patient data (name, age, gender).
        /// </summary>
        public static PatientData GenerateRandomPatientData()
        {
            Gender gender = _rng.Next(2) == 0 ? Gender.Male : Gender.Female;
            string firstName = gender == Gender.Male 
                ? _maleNames[_rng.Next(_maleNames.Length)]
                : _femaleNames[_rng.Next(_femaleNames.Length)];
            string lastName = _lastNames[_rng.Next(_lastNames.Length)];

            return new PatientData
            {
                PatientID = $"PAT-{_patientCounter:D4}",
                PatientName = $"{firstName} {lastName}",
                Age = _rng.Next(18, 80),
                Gender = gender,
                Symptoms = new PatientSymptoms
                {
                    SymptomDurationDays = _rng.Next(1, 30),
                    MainComplaint = "General discomfort",
                    AdditionalSymptoms = new List<string>()
                },
                Scans = new ScanResults(),
                Labs = new LabResults()
            };
        }

        /// <summary>
        /// Apply illness-specific symptoms and test results to patient data.
        /// </summary>
        public static PatientData ApplyIllnessToPatient(PatientData patient, Illness illness)
        {
            // Set symptoms, scans, and labs based on the illness
            switch (illness.Name)
            {   
                case "No Illness":
                    patient.Symptoms.SymptomDurationDays = _rng.Next(1, 4);
                    patient.Symptoms.MainComplaint = "I have a slight headache";
                    //patient.Symptoms.AdditionalSymptoms = new List<string> { "tired", "runny nose" };
                    patient.Scans = new ScanResults
                    {
                        Lungs = LungState.Normal,
                        Heart = HeartState.Normal,
                        Digestive = DigestiveState.Normal,
                        Skin = SkinState.Normal
                    };
                    patient.Labs = new LabResults
                    {
                        Blood = new BloodResults { WBC = WBCLevel.Normal, RBC = RBCLevel.Normal, Stress = StressMarkers.Normal },
                        Urine = UrineState.Normal,
                        HairSkin = HairSkinTest.Normal
                    };
                    break;

                case "Urinary tract infection":
                    patient.Symptoms.SymptomDurationDays = _rng.Next(1, 3);
                    patient.Symptoms.MainComplaint = "I have pain in my lower abdomen";
                    //patient.Symptoms.AdditionalSymptoms = new List<string> { "tired", "runny nose" };
                    patient.Scans = new ScanResults
                    {
                        Lungs = LungState.Normal,
                        Heart = HeartState.Normal,
                        Digestive = DigestiveState.Normal,
                        Skin = SkinState.Normal
                    };
                    patient.Labs = new LabResults
                    {
                        Blood = new BloodResults { WBC = WBCLevel.Normal, RBC = RBCLevel.Normal, Stress = StressMarkers.Normal },
                        Urine = UrineState.Abnormal,
                        HairSkin = HairSkinTest.Normal
                    };
                    break;

                case "Gastroenteritis":
                    patient.Symptoms.SymptomDurationDays = _rng.Next(1, 3);
                    patient.Symptoms.MainComplaint = "I feel nauseous";
                    patient.Symptoms.AdditionalSymptoms = new List<string> { "tired", "runny nose" };
                    patient.Scans = new ScanResults
                    {
                        Lungs = LungState.Normal,
                        Heart = HeartState.Normal,
                        Digestive = DigestiveState.Abnormal,
                        Skin = SkinState.Normal
                    };
                    patient.Labs = new LabResults
                    {
                        Blood = new BloodResults { WBC = WBCLevel.Normal, RBC = RBCLevel.Normal, Stress = StressMarkers.Normal },
                        Urine = UrineState.Abnormal,
                        HairSkin = HairSkinTest.Normal
                    };
                    break;

                case "Mild Flu":
                    patient.Symptoms.SymptomDurationDays = _rng.Next(1, 3);
                    patient.Symptoms.MainComplaint = "I've been coughing and sneezing";
                    patient.Symptoms.AdditionalSymptoms = new List<string> { "tired", "runny nose" };
                    patient.Scans = new ScanResults
                    {
                        Lungs = LungState.Abnormal,
                        Heart = HeartState.Normal,
                        Digestive = DigestiveState.Normal,
                        Skin = SkinState.Normal
                    };
                    patient.Labs = new LabResults
                    {
                        Blood = new BloodResults { WBC = WBCLevel.Normal, RBC = RBCLevel.Normal, Stress = StressMarkers.Normal },
                        Urine = UrineState.Normal,
                        HairSkin = HairSkinTest.Normal
                    };
                    break;

                case "Severe Allergy":
                    patient.Symptoms.SymptomDurationDays = _rng.Next(1, 3);
                    patient.Symptoms.MainComplaint = "My skin is itchy and face feels swollen";
                    patient.Symptoms.AdditionalSymptoms = new List<string> { "trouble breathing", "hives" };
                    patient.Scans = new ScanResults
                    {
                        Lungs = LungState.Abnormal,
                        Heart = HeartState.Abnormal,
                        Digestive = DigestiveState.Normal,
                        Skin = SkinState.Abnormal
                    };
                    patient.Labs = new LabResults
                    {
                        Blood = new BloodResults { WBC = WBCLevel.Normal, RBC = RBCLevel.Normal, Stress = StressMarkers.Normal },
                        Urine = UrineState.Abnormal,
                        HairSkin = HairSkinTest.Abnormal
                    };
                    break;

                case "Blood Infection":
                    patient.Symptoms.SymptomDurationDays = _rng.Next(3, 10);
                    patient.Symptoms.MainComplaint = "I feel extremely tired and dizzy";
                    patient.Symptoms.AdditionalSymptoms = new List<string> { "high fever", "weak" };
                    patient.Scans = new ScanResults
                    {
                        Lungs = LungState.Normal,
                        Heart = HeartState.Abnormal,
                        Digestive = DigestiveState.Normal,
                        Skin = SkinState.Normal
                    };
                    patient.Labs = new LabResults
                    {
                        Blood = new BloodResults { WBC = WBCLevel.Abnormal, RBC = RBCLevel.Normal, Stress = StressMarkers.Normal },
                        Urine = UrineState.Abnormal,
                        HairSkin = HairSkinTest.Normal
                    };
                    break;

                case "Drug-Induced Cardiac Stress":
                    patient.Symptoms.SymptomDurationDays = _rng.Next(1, 5);
                    patient.Symptoms.MainComplaint = "My chest hurts";
                    patient.Symptoms.AdditionalSymptoms = new List<string> { "feel weak", "palpitations" };
                    patient.Scans = new ScanResults
                    {
                        Lungs = LungState.Abnormal,
                        Heart = HeartState.Abnormal,
                        Digestive = DigestiveState.Normal,
                        Skin = SkinState.Normal
                    };
                    patient.Labs = new LabResults
                    {
                        Blood = new BloodResults { WBC = WBCLevel.Normal, RBC = RBCLevel.Normal, Stress = StressMarkers.Abnormal },
                        Urine = UrineState.Abnormal,
                        HairSkin = HairSkinTest.Normal
                    };
                    break;

                case "Toxic-Induced Neurological Distress":
                    patient.Symptoms.SymptomDurationDays = _rng.Next(1, 5);
                    patient.Symptoms.MainComplaint = "I feel very confused and disconnected";
                    patient.Symptoms.AdditionalSymptoms = new List<string> { "can't focus", "anxious", "restless" };
                    patient.Scans = new ScanResults
                    {
                        Lungs = LungState.Abnormal,
                        Heart = HeartState.Normal,
                        Digestive = DigestiveState.Abnormal,
                        Skin = SkinState.Abnormal
                    };
                    patient.Labs = new LabResults
                    {
                        Blood = new BloodResults { WBC = WBCLevel.Normal, RBC = RBCLevel.Normal, Stress = StressMarkers.Abnormal },
                        Urine = UrineState.Abnormal,
                        HairSkin = HairSkinTest.Abnormal
                    };
                    break;

                case "Pneumonia":
                    patient.Symptoms.SymptomDurationDays = _rng.Next(15, 25);
                    patient.Symptoms.MainComplaint = "I've been coughing for over 2 weeks";
                    patient.Symptoms.AdditionalSymptoms = new List<string> { "feverish", "hurts to breathe" };
                    patient.Scans = new ScanResults
                    {
                        Lungs = LungState.LooksInfected,
                        Heart = HeartState.SlightlyRed,
                        Digestive = DigestiveState.Normal,
                        Skin = SkinState.Normal
                    };
                    patient.Labs = new LabResults
                    {
                        Blood = new BloodResults { WBC = WBCLevel.Elevated, RBC = RBCLevel.Normal, Stress = StressMarkers.Normal },
                        Urine = UrineState.Normal,
                        HairSkin = HairSkinTest.Normal
                    };
                    break;



                case "Synthetic Substance Toxicity":
                    patient.Symptoms.SymptomDurationDays = _rng.Next(1, 5);
                    patient.Symptoms.MainComplaint = "I suddenly feel very dizzy, heart is racing";
                    patient.Symptoms.AdditionalSymptoms = new List<string> { "short of breath", "nauseous" };
                    patient.Scans = new ScanResults
                    {
                        Lungs = LungState.LooksInfected,
                        Heart = HeartState.VeryRed,
                        Digestive = DigestiveState.Normal,
                        Skin = SkinState.MildIrritation
                    };
                    patient.Labs = new LabResults
                    {
                        Blood = new BloodResults { WBC = WBCLevel.Normal, RBC = RBCLevel.Normal, Stress = StressMarkers.Elevated },
                        Urine = UrineState.ToxicMarkers,
                        HairSkin = HairSkinTest.ImmuneStress
                    };
                    break;


            }

            return patient;
        }

        #endregion
    }
}
