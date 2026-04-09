// ============================================================================
// DiagnosisSystem.cs - Core diagnosis evaluation system
// ============================================================================

using Script.Library;
using System.Collections.Generic;

namespace Script
{
    /// <summary>
    /// Main diagnosis system that evaluates patient data against illness definitions.
    /// Uses Singleton pattern for easy access from other scripts.
    /// </summary>
    public class DiagnosisSystem : Script.Library.IScript
    {
        #region Singleton

        /// <summary>
        /// Singleton instance for global access.
        /// </summary>
        public static DiagnosisSystem Instance { get; private set; }

        #endregion



        #region IScript Lifecycle

        public override void Awake()
        {
            // Singleton assignment
            if (Instance != null && Instance != this)
            {
                Logger.log(Logger.LogLevel.Warning, 
                    "DiagnosisSystem: Multiple instances detected! Using latest.");
            }
            Instance = this;
            
            Logger.log(Logger.LogLevel.Info, "DiagnosisSystem: Initialized (Singleton ready)");
        }

        public override void Start() { }

        public override void Update(double dt) { }

        public override void LateUpdate(double dt) { }

        public override void Free()
        {
            if (Instance == this)
            {
                Instance = null;
            }
            Logger.log(Logger.LogLevel.Info, "DiagnosisSystem: Freed");
        }

        #endregion

        #region Public API

        /// <summary>
        /// Diagnose a patient by evaluating their data against all known illnesses.
        /// </summary>
        /// <param name="patient">Complete patient data packet.</param>
        /// <returns>The matching Illness, or null if no diagnosis could be made.</returns>
        public Illness Diagnose(PatientData patient)
        {
            Logger.log(Logger.LogLevel.Info, 
                $"DiagnosisSystem: Diagnosing patient '{patient.PatientName}' ({patient.PatientID})");
            Logger.log(Logger.LogLevel.Info, 
                $"  Symptoms: {patient.Symptoms}");
            Logger.log(Logger.LogLevel.Info, 
                $"  Scans: {patient.Scans}");
            Logger.log(Logger.LogLevel.Info, 
                $"  Labs: {patient.Labs}");

            IReadOnlyList<Illness> illnesses = IllnessDatabase.GetAllIllnesses();

            foreach (Illness illness in illnesses)
            {
                if (illness.Matches(patient))
                {
                    Logger.log(Logger.LogLevel.Info, 
                        $"DiagnosisSystem: *** DIAGNOSIS RESULT: {illness.Name} ***");
                    Logger.log(Logger.LogLevel.Info, 
                        $"  Description: {illness.Description}");
                    return illness;
                }
            }

            Logger.log(Logger.LogLevel.Warning, 
                "DiagnosisSystem: No matching diagnosis found. Recommend further tests.");
            return null;
        }

        /// <summary>
        /// Quick diagnosis that returns just the illness name.
        /// </summary>
        public string DiagnoseAndGetName(PatientData patient)
        {
            Illness result = Diagnose(patient);
            return result?.Name ?? "Unknown";
        }

        /// <summary>
        /// Returns a list of illness names for the hint feature:
        /// the correct diagnosis and random wrong diagnoses to make up the total count.
        /// Returns an empty list if the correct illness is not in the button list.
        /// </summary>
        public static List<string> GetHintList(
            string correctIllnessName, List<string> allButtonIllnesses, int totalToShow = 3)
        {
            // Check if the correct illness is actually in the button list
            if (!allButtonIllnesses.Contains(correctIllnessName))
            {
                return new List<string>();
            }

            List<string> wrongOptions = new List<string>();
            foreach (var name in allButtonIllnesses)
            {
                if (name != correctIllnessName)
                    wrongOptions.Add(name);
            }

            List<string> result = new List<string> { correctIllnessName };

            System.Random rnd = new System.Random();
            while (result.Count < totalToShow && wrongOptions.Count > 0)
            {
                int idx = rnd.Next(wrongOptions.Count);
                result.Add(wrongOptions[idx]);
                wrongOptions.RemoveAt(idx);
            }

            return result;
        }

        #endregion
    }
}
