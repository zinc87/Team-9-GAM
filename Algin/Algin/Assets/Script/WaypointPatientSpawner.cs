// ============================================================================
// WaypointPatientSpawner.cs - Spawns patients on 'L' key press at Waypoint 1
// ============================================================================
// FEATURES:
// - Spawns patients with vape.agstaticmesh for visibility
// - 'N' key only affects the active (most recent) patient
// - Safe handling of deleted objects via exception catching
// ============================================================================

using Script.Library;
using System;
using System.Collections.Generic;
using System.Threading.Tasks;

namespace Script
{
    /// <summary>
    /// Spawns patient entities at Waypoint 1 when the 'L' key is pressed.
    /// Press 'N' to advance the most recently spawned patient through waypoints.
    /// </summary>
    public class WaypointPatientSpawner : Script.Library.IScript
    {

        public static WaypointPatientSpawner Instance { get; private set; }

        #region Waypoint Definitions

        // Update to gameobj
        public static Vector3D Waypoint1;
        public static Vector3D Waypoint2;
        public static Vector3D Waypoint3;
        public static Vector3D Waypoint4;
        public static Vector3D Waypoint5;
        public static Vector3D Waypoint6;
        public static Vector3D Waypoint7;
        public static Vector3D Waypoint8;
        public static Vector3D Waypoint9;
        public static Vector3D Waypoint10;

        [SerializeField] public Script.Library.Object wpt1;
        [SerializeField] public Script.Library.Object wpt2;
        [SerializeField] public Script.Library.Object wpt3;
        [SerializeField] public Script.Library.Object wpt4;
        [SerializeField] public Script.Library.Object wpt5;
        [SerializeField] public Script.Library.Object wpt6;
        [SerializeField] public Script.Library.Object wpt7;
        [SerializeField] public Script.Library.Object wpt8;
        [SerializeField] public Script.Library.Object wpt9;
        [SerializeField] public Script.Library.Object wpt10;

        private int numberOfPatients;
        private int vipSpawnFlag;

        #endregion

        #region State

        /// <summary>
        /// Counter for spawned patients (used for naming/logging).
        /// </summary>
        private int _spawnedCount = 0;

        /// <summary>
        /// The currently active patient controller (most recently spawned).
        /// This is the one that responds to 'N' key presses.
        /// </summary>
        private WaypointPatientController _activePatient = null;

        /// <summary>
        /// List of all spawned patient controllers for update propagation.
        /// </summary>
        public static WaypointPatientController[] _allPatients = new WaypointPatientController[8];

        private static readonly Random _rng = new Random();

        public int illnessMat = -1;
        public int mesh = -1;

        private bool _vipSpawned = false;
        public bool VipSpawned { get { return _vipSpawned; } }

        #endregion

        #region IScript Lifecycle

        public override void Awake()
        {
            // Clear static state for scene reload
            for (int i = 0; i < _allPatients.Length; i++)
                _allPatients[i] = null;
            
            // Fix: Ensure we clear out old controllers from previous scene load
            WaypointPatientControllerManager.ClearAll();

            Instance = this;
            Logger.log(Logger.LogLevel.Info,
                "WaypointPatientSpawner: Initialized - Press 'L' to spawn, 'N' to move active patient");

            _vipSpawned = false;

            // Zero error check cuz no time and living on the edge
            Waypoint1 = wpt1.getComponent<TransformComponent>().Transformation.position;
            Waypoint2 = wpt2.getComponent<TransformComponent>().Transformation.position;
            Waypoint3 = wpt3.getComponent<TransformComponent>().Transformation.position;
            Waypoint4 = wpt4.getComponent<TransformComponent>().Transformation.position;
            Waypoint5 = wpt5.getComponent<TransformComponent>().Transformation.position;
            Waypoint6 = wpt6.getComponent<TransformComponent>().Transformation.position;
            Waypoint7 = wpt7.getComponent<TransformComponent>().Transformation.position;
            Waypoint8 = wpt8.getComponent<TransformComponent>().Transformation.position;
            Waypoint9 = wpt9.getComponent<TransformComponent>().Transformation.position;
            Waypoint10 = wpt10.getComponent<TransformComponent>().Transformation.position;

            //spawn patients
            string sceneName = Scene.getSceneName();
            Logger.log(Logger.LogLevel.Info, $"WaypointPatientSpawner: Scene name = '{sceneName}'");
            if (sceneName.Contains("TempMap"))
            {
                numberOfPatients = 6;
                numberOfPatients = 6;
                vipSpawnFlag = 6;
            }
            else if (sceneName.Contains("Level_2"))
            {
                numberOfPatients = 8;
                vipSpawnFlag = 8;
            }
            else
            {
                numberOfPatients = 0;
                Logger.log(Logger.LogLevel.Warning,
                    $"WaypointPatientSpawner: Unrecognized scene '{sceneName}', spawning 0 patients");
            }

            for (int i = 0; i < numberOfPatients; i++)
            {
                SpawnPatientAtWaypoint1(i);
            }
        }

        public override void Start()
        {
        }

        public override void Update(double dt)
        {
            //// Listen for 'L' key to spawn a new patient
            //if (Input.isKeyClicked(Input.KeyCode.kL))
            //{
            //    SpawnPatientAtWaypoint1();
            //}

            // Listen for 'N' key to advance the ACTIVE patient only
            if (Input.isKeyClicked(Input.KeyCode.kN))
            {
                if (_activePatient != null && !_activePatient.IsMarkedForRemoval)
                {
                    _activePatient.AdvanceToNextWaypoint();
                }
                else
                {
                    Logger.log(Logger.LogLevel.Warning,
                        "WaypointPatientSpawner: No active patient! Press 'L' to spawn one.");
                }
            }

            // Update all patients and cleanup any that fail
            UpdateAndCleanupPatients(dt);



            //FOR SUBMISSION
            if (!_vipSpawned && DiagnosePanelMgr.totalDiagnosed >= vipSpawnFlag)
            {
                SpawnVipLevel1();
                _vipSpawned = true;
                //_ = PlayVipAudioSequenceAsync();
            }
        }

        public override void LateUpdate(double dt) { }

        public override void Free()
        {
            Logger.log(Logger.LogLevel.Info,
                $"WaypointPatientSpawner: Freed - Total spawned: {_spawnedCount}");
            for (int i = 0; i < _allPatients.Length; i++)
                _allPatients[i] = null;
            _activePatient = null;
            
            // Fix: Clear all registered controllers so they don't dangle after scene unload
            WaypointPatientControllerManager.ClearAll();
        }

        #endregion

        #region Patient Management

        /// <summary>
        /// Update all patients. If any patient's update fails (object deleted),
        /// mark it for removal.
        /// </summary>
        private void UpdateAndCleanupPatients(double dt)
        {
            // Update all registered controllers, even if they were removed from the _allPatients array
            var allControllers = WaypointPatientControllerManager.GetAllControllers();
            for (int i = 0; i < allControllers.Count; i++)
            {
                var patient = allControllers[i];
                if (patient != null && !patient.IsMarkedForRemoval)
                {
                    patient.UpdateMovement(dt);
                }
            }

            // Null out any patients that were marked for removal
            bool needsActivePatientUpdate = false;
            for (int i = 0; i < _allPatients.Length; i++)
            {
                if (_allPatients[i] != null && _allPatients[i].IsMarkedForRemoval)
                {
                    Logger.log(Logger.LogLevel.Info,
                        $"WaypointPatientSpawner: Removing patient {_allPatients[i].ObjectId}");

                    if (_allPatients[i] == _activePatient)
                    {
                        _activePatient = null;
                        needsActivePatientUpdate = true;
                    }

                    _allPatients[i] = null;
                }
            }

            // If active patient was removed, assign a new one
            if (needsActivePatientUpdate)
            {
                for (int i = _allPatients.Length - 1; i >= 0; i--)
                {
                    if (_allPatients[i] != null)
                    {
                        _activePatient = _allPatients[i];
                        Logger.log(Logger.LogLevel.Info,
                            $"WaypointPatientSpawner: New active patient: {_activePatient.ObjectId}");
                        break;
                    }
                }
            }
        }

        #endregion

        #region Spawning Logic

        public void SpawnVipLevel1()
        {
            // Create a new patient entity
            string newObjectId = Script.Library.Object.createPatient();

            // Wrap the ObjectID in an Object to manipulate it
            Script.Library.Object newPatientObj = new Script.Library.Object(newObjectId);

            // Create child object with BoxCollider for collision
            //Script.Library.Object childCollider = newPatientObj.CreateChild();
            newPatientObj.addComponent(ComponentTypes.Transform);
            newPatientObj.addComponent(ComponentTypes.BoxCollider);

            var boxCollider = newPatientObj.getComponent<BoxColliderComponent>();
            // Set BoxCollider to NPC layer
            //var boxCollider = childCollider.getComponent<BoxColliderComponent>();
            if (boxCollider != null)
            {
                BoxColliderComponentData bcData = boxCollider.Data;
                bcData.center_offset = new Vector3D(0f, 1.6f, 0f);
                bcData.halfExtent = new Vector3D(1.7f, 1.6f, 1.3f);
                bcData.isTrigger = true;
                bcData.drawCollider = false;
                bcData.layerMask = (int)CollisionLayers.Layer_NPC; ;
                boxCollider.Data = bcData;
                Logger.log(Logger.LogLevel.Info, "[WaypointPatientSpawner] Added BoxCollider child with NPC layer");
            }
            // Create the controller
            WaypointPatientController controller = new WaypointPatientController(newObjectId);
            controller.Initialize(0, Waypoint1, Waypoint2, Waypoint3, Waypoint4, Waypoint5, Waypoint6, Waypoint7, Waypoint8, Waypoint9, Waypoint10);

            // Make this the active patient
            _activePatient = controller;
            PatientSpawner._patientCounter++;

            //2. Generate VIP Level1 Data
            
            PatientData VipPatientData =
                new PatientData
                {
                    PatientID = "7",
                    PatientName = "Brandon Peh",
                    Age = 60,
                    Gender = Gender.Male,
                    Symptoms = new PatientSymptoms
                    {
                        SymptomDurationDays = _rng.Next(1, 30),
                        MainComplaint = "General discomfort",
                        AdditionalSymptoms = new List<string>()
                    },
                    Scans = new ScanResults(),
                    Labs = new LabResults(),
                    inPosition = false
                };
            VipPatientData.meshNumber = 7;

            // 3. Get random Level 1 Illness (Indices 1, 2, 3)
            var allIllnesses = IllnessDatabase.GetAllIllnesses();
            Illness VIPillness;

            string sceneName = Scene.getSceneName();
            Logger.log(Logger.LogLevel.Info, $"WaypointPatientSpawner: Scene name = '{sceneName}'");
            if (sceneName.Contains("TempMap"))
            {
                VIPillness = allIllnesses[_rng.Next(1, 4)];
            }
            else
            {
                VIPillness = allIllnesses[_rng.Next(1, 7)];
            }

            VipPatientData = PatientSpawner.ApplyIllnessToPatient(VipPatientData, VIPillness);
            VipPatientData.Symptoms.MainComplaint = "Something is Wrong with me";
            int slotIndex = PatientManager.RegisterPatient(newObjectId, VipPatientData, VIPillness, true);

            // Place controller into the matching fixed slot
            if (slotIndex >= 0 && slotIndex < _allPatients.Length)
            {
                _allPatients[slotIndex] = controller;
            }

            /////////////////////////////////////////////////////////////////////////////
            WaypointPatientControllerManager.RegisterController(controller);


            try
            {
                // Set the mesh to Vape.agstaticmesh (note: capital V is important!)
                SkinnedMeshRenderer skinnedMeshRenderer = newPatientObj.getComponent<SkinnedMeshRenderer>();
                if (skinnedMeshRenderer != null)
                {
 
                    skinnedMeshRenderer.ChangeModel("VIPAnimations.agskinnedmesh");
                    skinnedMeshRenderer.SetSubmeshMaterial("Body", "VIP_Body.agmat");

                    string matHeart = "HealthyHeart.agmat";
                    string matStomach = "HealthyStomach.agmat";
                    string matLungs = "HealthyLungs.agmat";

                    if (VIPillness != null)
                    {
                        if (VIPillness.Name == "No Illness")
                        {
                            illnessMat = 1;
                        }
                        else if (VIPillness.Name == "Urinary tract infection")
                        {
                            illnessMat = 2;
                        }
                        else if (VIPillness.Name == "Mild Flu")
                        {
                            illnessMat = 0;
                        }
                        else if (VIPillness.Name == "Gastroenteritis")
                        {
                            illnessMat = 3;
                        }
                        else if (VIPillness.Name == "Severe Allergy")
                        {
                            illnessMat = 4;
                        }
                        else if (VIPillness.Name == "Blood Infection")
                        {
                            illnessMat = 5;
                        }
                        else if (VIPillness.Name == "Drug-Induced Cardiac Stress")
                        {
                            illnessMat = 6;
                        }
                        else if (VIPillness.Name == "Toxic - Induced Neurological Distress")
                        {
                            illnessMat = 7;
                        }
                    }

                    if (illnessMat == 0)// MILD FLU
                    {
                        matHeart = "HealthyHeart.agmat";
                        matStomach = "HealthyStomach.agmat";
                        matLungs = "UnhealthyLungs.agmat";
                    }
                    if (illnessMat == 1)//NO ILLNESS
                    {
                        matHeart = "HealthyHeart.agmat";
                        matStomach = "HealthyStomach.agmat";
                        matLungs = "HealthyLungs.agmat";
                    }
                    if (illnessMat == 2)//UTI
                    {
                        matHeart = "HealthyHeart.agmat";
                        matStomach = "HealthyStomach.agmat";
                        matLungs = "HealthyLungs.agmat";
                    }
                    if (illnessMat == 3)//GASTRO
                    {
                        matHeart = "HealthyHeart.agmat";
                        matStomach = "UnhealthyStomach.agmat";
                        matLungs = "HealthyLungs.agmat";
                    }
                    if (illnessMat == 4)//Severe Allergy
                    {
                        matHeart = "UnhealthyHeart.agmat";
                        matStomach = "HealthyStomach.agmat";
                        matLungs = "UnhealthyLungs.agmat";
                    }
                    if (illnessMat == 5)//Blood Infection
                    {
                        matHeart = "UnhealthyHeart.agmat";
                        matStomach = "HealthyStomach.agmat";
                        matLungs = "HealthyLungs.agmat";
                    }
                    if (illnessMat == 6)//Drug Induced
                    {
                        matHeart = "UnhealthyHeart.agmat";
                        matStomach = "HealthyStomach.agmat";
                        matLungs = "UnhealthyLungs.agmat";
                    }
                    if (illnessMat == 7)//Toxic
                    {
                        matHeart = "HealthyHeart.agmat";
                        matStomach = "UnhealthyStomach.agmat";
                        matLungs = "UnhealthyLungs.agmat";
                    }

                    skinnedMeshRenderer.SetSubmeshMaterial("Organ_Heart", matHeart);
                    skinnedMeshRenderer.SetSubmeshMaterial("Organ_Stomach", matStomach);
                    skinnedMeshRenderer.SetSubmeshMaterial("Organ_Stomach_1", matStomach);
                    skinnedMeshRenderer.SetSubmeshMaterial("Organ_Lungs", matLungs);
                    skinnedMeshRenderer.SetSubmeshMaterial("Organ_Lungs_1", matLungs);

                    skinnedMeshRenderer.PlayAnimation("idle");
                    skinnedMeshRenderer.SetLoopAnimation(true);
                    // Flip rotation
                }

                // Set the mesh to Vape.agstaticmesh (note: capital V is important!)
                TransformComponent transform = newPatientObj.getComponent<TransformComponent>();
                if (transform != null)
                {
                    var trf = transform.Transformation;

                    trf.Scale.y = 0.02f;
                    trf.Scale.x = 0.02f;
                    trf.Scale.z = 0.02f;

                    trf.Rotation.y = 180.0f;
                    // trf.Rotation.x += mouseDelta.y * lookSpeed;
                    // trf.Rotation.x = MathPlus.Clamp(trf.Rotation.x, -89f, 89f);

                    transform.Transformation = trf;
                }
            }
            catch (Exception ex)
            {
                Logger.log(Logger.LogLevel.Warning,
                    $"WaypointPatientSpawner: Could not set mesh: {ex.Message}");
            }
        }

        /// <summary>
        /// Spawns a new patient entity at Waypoint 1 with mesh and components.
        /// </summary>
        private void SpawnPatientAtWaypoint1(int index)
        {
            ++_spawnedCount;
            mesh = _rng.Next(6);
            // Create a new patient entity
            string newObjectId = Script.Library.Object.createPatient();

            Logger.log(Logger.LogLevel.Info,
                $"WaypointPatientSpawner: Spawning patient #{_spawnedCount} with ID: {newObjectId}");

            // Wrap the ObjectID in an Object to manipulate it
            Script.Library.Object newPatientObj = new Script.Library.Object(newObjectId);

            // Create child object with BoxCollider for collision
            //Script.Library.Object childCollider = newPatientObj.CreateChild();
            newPatientObj.addComponent(ComponentTypes.Transform);
            newPatientObj.addComponent(ComponentTypes.BoxCollider);

            var boxCollider = newPatientObj.getComponent<BoxColliderComponent>();
            // Set BoxCollider to NPC layer
            //var boxCollider = childCollider.getComponent<BoxColliderComponent>();
            if (boxCollider != null)
            {
                BoxColliderComponentData bcData = boxCollider.Data;
                bcData.center_offset = new Vector3D(0f, 1.6f, 0f);
                bcData.halfExtent = new Vector3D(1.7f, 1.6f, 1.3f);
                bcData.isTrigger = true; // Set to true so they don't physically collide
                bcData.drawCollider = false;
                bcData.layerMask = (int)CollisionLayers.Layer_NPC;
                boxCollider.Data = bcData;
                Logger.log(Logger.LogLevel.Info, "[WaypointPatientSpawner] Added BoxCollider child with NPC layer");
            }
            // Create the controller
            WaypointPatientController controller = new WaypointPatientController(newObjectId);
            controller.Initialize(index, Waypoint1, Waypoint2, Waypoint3, Waypoint4, Waypoint5, Waypoint6, Waypoint7, Waypoint8, Waypoint9, Waypoint10);

            // Make this the active patient
            _activePatient = controller;

            /////////////////////////////////////////////////////////////////////////////
            PatientSpawner._patientCounter++;

            // 2. Generate random patient data
            PatientData patientData = PatientSpawner.GenerateRandomPatientData();
            // 3. Get Level 1 Illnesses
            Illness illness;

            string sceneName = Scene.getSceneName();
            Logger.log(Logger.LogLevel.Info, $"WaypointPatientSpawner: Scene name = '{sceneName}'");
            if (sceneName.Contains("TempMap"))
            {
                if (index == 0)
                {
                    illness = IllnessDatabase.GetIllnessByName("Gastroenteritis");
                }
                else
                {
                    illness = IllnessDatabase.getLevel1Illnesses();
                }
            }
            else
            {
                illness = IllnessDatabase.getLevel2Illnesses();
            }

            if (illness.Name == "Severe Allergy")
            {
                patientData.meshNumber = 6;
            }
            else
            {
                patientData.meshNumber = mesh;
            }


            patientData = PatientSpawner.ApplyIllnessToPatient(patientData, illness);
            int slotIndex = PatientManager.RegisterPatient(newObjectId, patientData, illness);

            // Place controller into the matching fixed slot
            if (slotIndex >= 0 && slotIndex < _allPatients.Length)
            {
                _allPatients[slotIndex] = controller;
            }

            /////////////////////////////////////////////////////////////////////////////
            WaypointPatientControllerManager.RegisterController(controller);

            try
            {
                // Set the mesh to Vape.agstaticmesh (note: capital V is important!)
                SkinnedMeshRenderer skinnedMeshRenderer = newPatientObj.getComponent<SkinnedMeshRenderer>();
                if (skinnedMeshRenderer != null)
                {
                    string meshName = "";
                    string skinMeshName = "";
                    string matHeart = "";
                    string matStomach = "";
                    string matLungs = "";

                    if (PatientManager.TryGetIllness(newObjectId, out Illness data))
                    {
                        if (data.Name == "No Illness")
                        {
                            illnessMat = 1;
                        }
                        else if (data.Name == "Urinary tract infection")
                        {
                            illnessMat = 2;
                        }
                        else if (data.Name == "Mild Flu")
                        {
                            illnessMat = 0;
                        }
                        else if (data.Name == "Gastroenteritis")
                        {
                            illnessMat = 3;
                        }
                        else if (data.Name == "Severe Allergy")
                        {
                            illnessMat = 4;
                        }
                        else if (data.Name == "Blood Infection")
                        {
                            illnessMat = 5;
                        }
                        else if (data.Name == "Drug-Induced Cardiac Stress")
                        {
                            illnessMat = 6;
                        }
                        else if (data.Name == "Toxic - Induced Neurological Distress")
                        {
                            illnessMat = 7;
                        }
                    }

                    if (mesh == 0)
                    {
                        meshName = "P1_Body.agmat";// blue shirt male
                        skinMeshName = "Patient1.agskinnedmesh";
                    }
                    if (mesh == 1)
                    {
                        meshName = "P2_Body.agmat";//glasses male, grey sleeveless
                        skinMeshName = "Patient2.agskinnedmesh";
                    }
                    if (mesh == 2)
                    {
                        meshName = "P3_Body.agmat";//pink shirt female, blonde
                        skinMeshName = "Patient3.agskinnedmesh";
                    }
                    if (mesh == 3)
                    {
                        meshName = "P4_Body.agmat";// purple hair female
                        skinMeshName = "Patient4Animations.agskinnedmesh";
                    }
                    if (mesh == 4)
                    {
                        //meshName = "P5_Body.agmat";// white top female
                        //skinMeshName = "Patient5Animation.agskinnedmesh";
                        meshName = "P4_Body.agmat";// purple hair female
                        skinMeshName = "Patient4Animations.agskinnedmesh";

                    }
                    if (mesh == 5)
                    {
                        meshName = "P7_Body.agmat";// grey shirt male
                        skinMeshName = "Patient7Animations.agskinnedmesh";
                    }

                    if (illnessMat == 0)// MILD FLU
                    {
                        matHeart = "HealthyHeart.agmat";
                        matStomach = "HealthyStomach.agmat";
                        matLungs = "UnhealthyLungs.agmat";
                    }
                    if (illnessMat == 1)//NO ILLNESS
                    {
                        matHeart = "HealthyHeart.agmat";
                        matStomach = "HealthyStomach.agmat";
                        matLungs = "HealthyLungs.agmat";
                    }
                    if (illnessMat == 2)//UTI
                    {
                        matHeart = "HealthyHeart.agmat";
                        matStomach = "HealthyStomach.agmat";
                        matLungs = "HealthyLungs.agmat";
                    }
                    if (illnessMat == 3)//GASTRO
                    {
                        matHeart = "HealthyHeart.agmat";
                        matStomach = "UnhealthyStomach.agmat";
                        matLungs = "HealthyLungs.agmat";
                    }
                    if (illnessMat == 4)//Severe Allergy
                    {
                        meshName = "P6_Body_SA.agmat";//RASH female
                        skinMeshName = "Patient6-SevereAllergyAnimations.agskinnedmesh";
                        matHeart = "UnhealthyHeart.agmat";
                        matStomach = "HealthyStomach.agmat";
                        matLungs = "UnhealthyLungs.agmat";
                        patientData.meshNumber = 6;
                    }
                    if (illnessMat == 5)//Blood Infection
                    {
                        matHeart = "UnhealthyHeart.agmat";
                        matStomach = "HealthyStomach.agmat";
                        matLungs = "HealthyLungs.agmat";
                    }
                    if (illnessMat == 6)//Drug Induced
                    {
                        matHeart = "UnhealthyHeart.agmat";
                        matStomach = "HealthyStomach.agmat";
                        matLungs = "UnhealthyLungs.agmat";
                    }
                    if (illnessMat == 7)//Toxic
                    {
                        matHeart = "HealthyHeart.agmat";
                        matStomach = "UnhealthyStomach.agmat";
                        matLungs = "UnhealthyLungs.agmat";
                    }

                    skinnedMeshRenderer.ChangeModel(skinMeshName);

                    skinnedMeshRenderer.SetSubmeshMaterial("Body", meshName);
                    skinnedMeshRenderer.SetSubmeshMaterial("Organ_Heart", matHeart);
                    skinnedMeshRenderer.SetSubmeshMaterial("Organ_Heart_1", matHeart);
                    skinnedMeshRenderer.SetSubmeshMaterial("Organ_Stomach", matStomach);
                    skinnedMeshRenderer.SetSubmeshMaterial("Organ_Lungs", matLungs);
                    skinnedMeshRenderer.SetSubmeshMaterial("Organ_Lungs_1", matLungs);

                    skinnedMeshRenderer.PlayAnimation("idle");
                    skinnedMeshRenderer.SetLoopAnimation(true);
                    // Flip rotation
                }

                // Set the mesh to Vape.agstaticmesh (note: capital V is important!)
                TransformComponent transform = newPatientObj.getComponent<TransformComponent>();
                if (transform != null)
                {
                    var trf = transform.Transformation;

                    trf.Scale.y = 0.02f;
                    trf.Scale.x = 0.02f;
                    trf.Scale.z = 0.02f;

                    trf.Rotation.y = 180.0f;
                    // trf.Rotation.x += mouseDelta.y * lookSpeed;
                    // trf.Rotation.x = MathPlus.Clamp(trf.Rotation.x, -89f, 89f);

                    transform.Transformation = trf;
                }
            }
            catch (Exception ex)
            {
                Logger.log(Logger.LogLevel.Warning,
                    $"WaypointPatientSpawner: Could not set mesh: {ex.Message}");
            }


            Logger.log(Logger.LogLevel.Info,
                $"WaypointPatientSpawner: Patient #{_spawnedCount} is now ACTIVE - Press 'N' to move it");
        }

        #endregion

        #region Hash Utility

        /// <summary>
        /// Compute FNV-1a 64-bit hash that matches C++ AssetManager::fnv1a_64.
        /// This is also available via HASH2 macro in C++.
        /// </summary>
        private static ulong ComputeFnv1a64Hash(string s)
        {
            // Same as AssetManager::fnv1a_64 in AssetManager.h
            const ulong FNV_OFFSET_BASIS = 14695981039346656037UL;
            const ulong FNV_PRIME = 1099511628211UL;

            ulong hash = FNV_OFFSET_BASIS;
            foreach (char c in s)
            {
                hash ^= (byte)c;
                hash *= FNV_PRIME;
            }
            return hash;
        }

        #endregion
    }
}
