// ============================================================================
// PatientController.cs - Data container for patient information
// ============================================================================

using Script.Library;
using System;
using System.Collections.Generic;

namespace Script
{
    public enum PatientRegistryState
    {
        Waiting,
        Moving,
        Diagnosing
    }

    public struct PatientRecord
    {
        public PatientData Data;
        public Illness Illness;
        public PatientRegistryState State;
        public bool IsVIP;
    }

    /// <summary>
    /// </summary>
    public class PatientManager : Script.Library.IScript
    {
        private static Dictionary<string, PatientRecord> s_patients =
            new Dictionary<string, PatientRecord>();

        // Fixed-slot array: each index corresponds to a UI card slot.
        // null means the slot is empty.
        private static string[] s_patientSlots = new string[8];

        private static bool isDiscard = false;

        public static int patientCount = 0;

        #region IScript Lifecycle
        public override void Awake()
        { 
            ClearAll();
        }

        public override void Start() 
        { }

        public override void Update(double dt)
        {
            //ItemType current_item = InventoryManager.Instance.GetCurrentObject();
            //Library.Logger.log(Library.Logger.LogLevel.Info, $"{current_item}");
            var patient_obj_id = GetAllPatientObjectIds();
            var inventory = InventoryManager.Instance;
            bool scannerActive = inventory != null && inventory.CurrentItem == ItemType.Scanner;
            if (scannerActive) 
            {   
                if (Input.isMouseClicked(Input.Mouse.kMOUSE_BUTTON_LEFT))// mouse down
                {
                    Library.Logger.log(Library.Logger.LogLevel.Info, "Changing NPC shader to use discard shader");
                    for (int i = 0; i < patient_obj_id.Count; i++)
                    {
                        if (patient_obj_id[i] == null) continue;
                        Script.Library.Object PatientObj = new Script.Library.Object(patient_obj_id[i]);
                        ShaderChanger.DiscardShader(PatientObj);
                        isDiscard = true;
                    }

                }
                else if (Input.isMouseReleased(Input.Mouse.kMOUSE_BUTTON_LEFT))// mouse up
                {
                    Library.Logger.log(Library.Logger.LogLevel.Info, "Changing NPC shader to use normal shader");
                    for (int i = 0; i < patient_obj_id.Count; i++)
                    {
                        if (patient_obj_id[i] == null) continue;
                        Script.Library.Object PatientObj = new Script.Library.Object(patient_obj_id[i]);
                        ShaderChanger.NormalShader(PatientObj);
                        isDiscard = false;
                    }
                }
            }
            else // edge case if held down and item swap
            {
                if (isDiscard)
                {
                    Library.Logger.log(Library.Logger.LogLevel.Info, "Changing NPC shader to use normal shader");
                    for (int i = 0; i < patient_obj_id.Count; i++)
                    {
                        if (patient_obj_id[i] == null) continue;
                        Script.Library.Object PatientObj = new Script.Library.Object(patient_obj_id[i]);
                        ShaderChanger.NormalShader(PatientObj);
                        isDiscard = false;
                    }
                }
            }
            

        
        }

        public override void LateUpdate(double dt) { }

        public override void Free()
        { }
        #endregion

        // Register when patient spawns (non-VIP by default)
        public static int RegisterPatient(
            string objectId,
            PatientData data,
            Illness illness)
        {
            return RegisterPatient(objectId, data, illness, false);
        }

        // Register when patient spawns (with explicit VIP flag)
        public static int RegisterPatient(
            string objectId,
            PatientData data,
            Illness illness,
            bool isVIP)
        {
            if (s_patients.ContainsKey(objectId))
                return -1;

            PatientRecord record = new PatientRecord
            {
                Data = data,
                Illness = illness,
                State = PatientRegistryState.Waiting,
                IsVIP = isVIP
            };

            s_patients.Add(objectId, record);
            ++patientCount;

            // Assign to the first empty slot
            int slotIndex = -1;
            for (int i = 0; i < s_patientSlots.Length; i++)
            {
                if (s_patientSlots[i] == null)
                {
                    s_patientSlots[i] = objectId;
                    slotIndex = i;
                    break;
                }
            }

            TryPrintPatientData(objectId);
            TryPrintPatientIllness(objectId);
            return slotIndex;
        }

        /// <summary>
        /// Returns true if the patient with the given object ID is a VIP patient.
        /// </summary>
        public static bool IsPatientVIP(string objectId)
        {
            if (!s_patients.ContainsKey(objectId))
                return false;
            return s_patients[objectId].IsVIP;
        }

        // Remove when patient leaves / despawns
        public static void RemovePatient(string objectId)
        {
            if (s_patients.ContainsKey(objectId))
                s_patients.Remove(objectId);

            // Clear the slot so the index stays stable
            for (int i = 0; i < s_patientSlots.Length; i++)
            {
                if (s_patientSlots[i] == objectId)
                {
                    s_patientSlots[i] = null;
                    break;
                }
            }
            --patientCount;
        }

        public static int getPatientCount()
        {
            return patientCount;
        }

        //Setting Patient Data
        public static void SetPatientData(string objID, PatientData data)
        {
            if(!s_patients.ContainsKey(objID))
                return;
            PatientRecord record = s_patients[objID];
            record.Data = data;
            s_patients[objID] = record;
        }

        // Change patient state
        public static void SetState(string objectId, PatientRegistryState newState)
        {
            if (!s_patients.ContainsKey(objectId))
                return;

            PatientRecord record = s_patients[objectId];
            record.State = newState;
            s_patients[objectId] = record;
        }

        // Get current state
        public static PatientRegistryState GetState(string objectId)
        {
            if (!s_patients.ContainsKey(objectId))
                return PatientRegistryState.Waiting;

            return s_patients[objectId].State;
        }

        // Get patient data
        public static bool TryGetPatientData(string objectId, out PatientData data)
        {
            data = default;

            if (!s_patients.ContainsKey(objectId))
                return false;

            data = s_patients[objectId].Data;
            return true;
        }

        public static void TryPrintPatientData(string objectId) {
            if (!s_patients.ContainsKey(objectId))
                return;

            PatientData patientData = s_patients[objectId].Data;

            Logger.log(Logger.LogLevel.Info, 
                $"  Name: {patientData.PatientName}");
            Logger.log(Logger.LogLevel.Info, 
                $"  Age: {patientData.Age}, Gender: {patientData.Gender}");
        }

        // Get illness info
        public static bool TryGetIllness(string objectId, out Illness illness)
        {
            illness = default;

            if (objectId == null || !s_patients.ContainsKey(objectId))
                return false;

            illness = s_patients[objectId].Illness;
            return true;
        }

        public static void TryPrintPatientIllness(string objectId) {
            if (!s_patients.ContainsKey(objectId))
                return;

           Illness illness = s_patients[objectId].Illness;

            Logger.log(Logger.LogLevel.Info, 
                $"  Assigned Illness: {illness.Name}");
            Logger.log(Logger.LogLevel.Info, 
                $"  Medication: {illness.Medication}");
            Logger.log(Logger.LogLevel.Info, 
                $"  Substance Exposure: {(illness.SuspectedSubstanceExposure ? "YES" : "No")}");
        }

        // Utility
        // Returns a fixed-size list (8 elements) where each index
        // corresponds to a UI card slot. Null means the slot is empty.
        public static List<string> GetAllPatientObjectIds()
        {
            return new List<string>(s_patientSlots);
        }

        public static void ClearAll()
        {
            s_patients.Clear();
            for (int i = 0; i < s_patientSlots.Length; i++)
                s_patientSlots[i] = null;
            patientCount = 0;
        }
    }
}
