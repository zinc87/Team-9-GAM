// ============================================================================
// IllnessDatabase.cs - Illness definitions and patient data structures
// ============================================================================

using System;
using System.Collections.Generic;

namespace Script
{
    #region Enums - Blood Work
    
    /// <summary>
    /// White blood cell count levels from blood work.
    /// </summary>
    public enum WBCLevel
    {
        Normal,
        Elevated,
        Low,
        Abnormal
    }

    /// <summary>
    /// Red blood cell count levels from blood work.
    /// </summary>
    public enum RBCLevel
    {
        Normal,
        Elevated,
        Low,
        Abnormal
    }

    /// <summary>
    /// Stress marker levels from blood work.
    /// </summary>
    public enum StressMarkers
    {
        Normal,
        Elevated,
        Abnormal
    }

    #endregion

    #region Enums - Urine & Hair/Skin Tests

    /// <summary>
    /// Urine test result states.
    /// </summary>
    public enum UrineState
    {
        Normal,
        InfectionMarkers,
        ToxicMarkers,
        Abnormal
    }

    /// <summary>
    /// Hair/Skin sample test results.
    /// </summary>
    public enum HairSkinTest
    {
        Normal,
        ImmuneReaction,
        ImmuneStress,
        Abnormal
    }

    #endregion

    #region Enums - Scan Results

    /// <summary>
    /// Lung scan result states.
    /// </summary>
    public enum LungState
    {
        Normal,
        MildIrritation,    // "Mild irritation / damaged tissue"
        LooksInfected,     // "Looks infected"
        Congested,
        Infected,
        Abnormal
    }

    /// <summary>
    /// Heart scan result states.
    /// </summary>
    public enum HeartState
    {
        Normal,
        SlightlyRed,       // "A little more red than usual"
        VeryRed,           // "Very red looking heart"
        LooksInfected,      // "Looks infected"
        Infected,
        Abnormal
    }

    /// <summary>
    /// Digestive system scan result states.
    /// </summary>
    public enum DigestiveState
    {
        Normal,
        Infected,
        Abnormal
    }

    /// <summary>
    /// Skin scan/visual result states.
    /// </summary>
    public enum SkinState
    {
        Normal,
        PaleAppearance,    // "Pale appearance"
        RashSwelling,      // "Rash / swelling (Red-ish skin color with spots)"
        MildIrritation,     // "Mild irritation (Slightly red-ish skin color with spots)"
        Rash,
        Abnormal
    }

    /// <summary>
    /// Hair/Skin sample test results (distinct from visual skin scan).
    /// </summary>
    public enum SkinTestResult
    {
        Normal,
        ImmuneReaction,
        Rash,
        Lesions,
        Abnormal
    }

    #endregion

    #region Data Structures

    /// <summary>
    /// Results from organ scans (X-ray, ultrasound, etc.)
    /// </summary>
    public struct ScanResults
    {
        public LungState Lungs;
        public HeartState Heart;
        public DigestiveState Digestive;
        public SkinState Skin;

        public override string ToString()
        {
            return $"[Lungs: {Lungs}, Heart: {Heart}, Digestive: {Digestive}, Skin: {Skin}]";
        }
    }

    /// <summary>
    /// Results from blood analysis.
    /// </summary>
    public struct BloodResults
    {
        public WBCLevel WBC;
        public RBCLevel RBC;
        public StressMarkers Stress;

        public override string ToString()
        {
            return $"[WBC: {WBC}, RBC: {RBC}, Stress: {Stress}]";
        }
    }

    /// <summary>
    /// All laboratory test results.
    /// </summary>
    public struct LabResults
    {
        public BloodResults Blood;
        public UrineState Urine;
        public HairSkinTest HairSkin;

        public bool bloodSampleCollected;
        public bool hairSampleCollected;
        public bool urineSampleCollected;

        public bool bloodSampleDeposited;
        public bool hairSampleDeposited;
        public bool urineSampleDeposited;


        public override string ToString()
        {
            return $"Blood: {Blood}, Urine: {Urine}, HairSkin: {HairSkin}";
        }
    }

    /// <summary>
    /// Patient-reported symptoms.
    /// </summary>
    public struct PatientSymptoms
    {
        /// <summary>How many days symptoms have been present.</summary>
        public int SymptomDurationDays;
        
        /// <summary>Main complaint from dialogue.</summary>
        public string MainComplaint;
        
        /// <summary>Additional symptom descriptors from dialogue.</summary>
        public List<string> AdditionalSymptoms;

        public override string ToString()
        {
            string additional = AdditionalSymptoms != null ? string.Join(", ", AdditionalSymptoms) : "none";
            return $"[Duration: {SymptomDurationDays} days, Complaint: {MainComplaint}, Additional: {additional}]";
        }
    }

    /// <summary>
    /// Patient gender options.
    /// </summary>
    public enum Gender
    {
        Male,
        Female
    }

    /// <summary>
    /// Complete patient data packet for diagnosis.
    /// </summary>
    public struct PatientData
    {
        public string PatientID;
        public string PatientName;
        public int Age;
        public Gender Gender;
        public PatientSymptoms Symptoms;
        public ScanResults Scans;
        public LabResults Labs;
        public int meshNumber;
        public bool inPosition;

        public override string ToString()
        {
            return $"Patient '{PatientName}' ({PatientID}) Age: {Age}, {Gender}\n" +
                   $"  Symptoms: {Symptoms}\n" +
                   $"  Scans: {Scans}\n" +
                   $"  Labs: {Labs}";
        }
    }

    #endregion

    #region Illness Definition

    /// <summary>
    /// Represents an illness with diagnostic criteria and recommended treatment.
    /// </summary>
    public class Illness
    {
        public string Name { get; private set; }
        public string Description { get; private set; }
        public string Medication { get; private set; }
        public bool SuspectedSubstanceExposure { get; private set; }
        
        private readonly Func<PatientData, bool> _matchCriteria;

        public Illness(string name, string description, string medication, bool suspectedExposure, Func<PatientData, bool> matchCriteria)
        {
            Name = name;
            Description = description;
            Medication = medication;
            SuspectedSubstanceExposure = suspectedExposure;
            _matchCriteria = matchCriteria;
        }

        public bool Matches(PatientData patient)
        {
            return _matchCriteria(patient);
        }
    }

    #endregion

    #region Illness Database

    /// <summary>
    /// Static database of all illness definitions.
    /// </summary>
    public static class IllnessDatabase
    {
        private static readonly List<Illness> _illnesses = new List<Illness>();
        private static readonly Random _rng = new Random();

        static IllnessDatabase()
        {
            InitializeIllnesses();
        }

        private static void InitializeIllnesses()
        {
            // ----------------------------------------------------------------
            // 1.0 Healthy
            // Symptoms: “I feel a slight headache”
            // Scans: Lungs: Normal, Heart/Digestive/Skin: Normal
            // Labs: All normal
            // ----------------------------------------------------------------
            _illnesses.Add(new Illness(
                "No Illness",
                "NIL",
                "NIL",
                false,
                (patient) =>
                {
                    bool shortDuration = patient.Symptoms.SymptomDurationDays <= 3;
                    bool mildLungs = patient.Scans.Lungs == LungState.Normal;
                    bool normalHeart = patient.Scans.Heart == HeartState.Normal;
                    bool normalWBC = patient.Labs.Blood.WBC == WBCLevel.Normal;
                    bool normalUrine = patient.Labs.Urine == UrineState.Normal;
                    bool normalHairSkin = patient.Labs.HairSkin == HairSkinTest.Normal;

                    return shortDuration && mildLungs && normalHeart && normalWBC && normalUrine && normalHairSkin;
                }
            ));

            // ----------------------------------------------------------------
            // 1.1 Urinary tract infection (UTI)
            // Symptoms: “I have pain/pressure in my lower abdomen or pelvic area.”
            // Scans: Lungs: Normal, Heart/Digestive/Skin: Normal
            // Labs: All normal
            // ----------------------------------------------------------------
            _illnesses.Add(new Illness(
                "Urinary tract infection",
                "NIL",
                "NIL",
                false,
                (patient) =>
                {
                    bool shortDuration = patient.Symptoms.SymptomDurationDays <= 3;
                    bool mildLungs = patient.Scans.Lungs == LungState.Normal;
                    bool normalHeart = patient.Scans.Heart == HeartState.Normal;
                    bool normalWBC = patient.Labs.Blood.WBC == WBCLevel.Normal;
                    bool normalUrine = patient.Labs.Urine == UrineState.InfectionMarkers;
                    bool normalHairSkin = patient.Labs.HairSkin == HairSkinTest.Normal;

                    return shortDuration && mildLungs && normalHeart && normalWBC && normalUrine && normalHairSkin;
                }
            ));

            // ----------------------------------------------------------------
            // 1.2 MILD FLU
            // Symptoms: "coughing and sneezing", "only about 3 days", "tired but not too bad"
            // Scans: Lungs: Mild irritation, Heart/Digestive/Skin: Normal
            // Labs: All normal
            // ----------------------------------------------------------------
            _illnesses.Add(new Illness(
                "Mild Flu",
                "Common viral infection with mild respiratory symptoms.",
                "Flu Medication",
                false,
                (patient) =>
                {
                    bool shortDuration = patient.Symptoms.SymptomDurationDays <= 3;
                    bool mildLungs = patient.Scans.Lungs == LungState.MildIrritation;
                    bool normalHeart = patient.Scans.Heart == HeartState.Normal;
                    bool normalWBC = patient.Labs.Blood.WBC == WBCLevel.Normal;
                    bool normalUrine = patient.Labs.Urine == UrineState.Normal;
                    bool normalHairSkin = patient.Labs.HairSkin == HairSkinTest.Normal;

                    return shortDuration && mildLungs && normalHeart && normalWBC && normalUrine && normalHairSkin;
                }
            ));

            // ----------------------------------------------------------------
            // VIP ILLNESS
            // 1.3 Gastroenteritis
            // Symptoms: “I feel nauseous.”
            // Scans: Lungs: Normal, Heart/Digestive/Skin: Normal
            // Labs: All normal
            // ----------------------------------------------------------------
            _illnesses.Add(new Illness(
                "Gastroenteritis",
                "NIL",
                "NIL",
                false,
                (patient) =>
                {
                    bool shortDuration = patient.Symptoms.SymptomDurationDays <= 3;
                    bool mildLungs = patient.Scans.Lungs == LungState.Normal;
                    bool normalHeart = patient.Scans.Heart == HeartState.Normal;
                    bool infectedDigestive = patient.Scans.Digestive == DigestiveState.Infected;
                    bool normalWBC = patient.Labs.Blood.WBC == WBCLevel.Normal;
                    bool normalUrine = patient.Labs.Urine == UrineState.InfectionMarkers;
                    bool normalHairSkin = patient.Labs.HairSkin == HairSkinTest.Normal;

                    return shortDuration && mildLungs && normalHeart && normalWBC && normalUrine && normalHairSkin &&
                    infectedDigestive;
                }
            ));

            // ----------------------------------------------------------------
            // 2.0 SEVERE ALLERGY
            // Symptoms: "skin is itchy", "face feels swollen", "trouble breathing"
            // Scans: Lungs: Looks infected, Heart: Very red, Skin: Rash/swelling
            // Labs: WBC Normal, HairSkin: Immune reaction
            // ----------------------------------------------------------------
            _illnesses.Add(new Illness(
                "Severe Allergy",
                "Acute allergic reaction with systemic symptoms.",
                "Allergy Medication",
                false,
                (patient) =>
                {
                    string complaint = patient.Symptoms.MainComplaint?.ToLower() ?? "";
                    bool hasAllergySymptoms = complaint.Contains("itchy") || complaint.Contains("swollen");
                    bool veryRedHeart = patient.Scans.Heart == HeartState.Infected;
                    bool rashSkin = patient.Scans.Skin == SkinState.Rash;
                    bool normalWBC = patient.Labs.Blood.WBC == WBCLevel.Normal;
                    bool immuneReaction = patient.Labs.HairSkin == HairSkinTest.ImmuneReaction;
                    bool infectionMarker = patient.Labs.Urine == UrineState.InfectionMarkers;

                    return hasAllergySymptoms && veryRedHeart && rashSkin && normalWBC && immuneReaction
                    && infectionMarker;
                }
            ));


            // ----------------------------------------------------------------
            // 2.1 BLOOD INFECTION 
            // Symptoms: "extremely tired", "dizzy", "high fever"
            // Scans: Lungs: Normal, Heart: Looks infected, Skin: Pale
            // Labs: WBC Elevated, Urine: Infection markers
            // ----------------------------------------------------------------
            _illnesses.Add(new Illness(
                "Blood Infection",
                "Systemic infection affecting the bloodstream.",
                "Antibiotics",
                false,
                (patient) =>
                {
                    bool normalLungs = patient.Scans.Lungs == LungState.Normal;
                    bool infectedHeart = patient.Scans.Heart == HeartState.LooksInfected;
                    bool paleSkin = patient.Scans.Skin == SkinState.PaleAppearance;
                    bool elevatedWBC = patient.Labs.Blood.WBC == WBCLevel.Elevated;
                    bool urineInfection = patient.Labs.Urine == UrineState.InfectionMarkers;

                    return normalLungs && infectedHeart && paleSkin && elevatedWBC && urineInfection;
                }
            ));

            // ----------------------------------------------------------------
            // 2.2 DRUG-INDUCED CARDIAC STRESS (K-pods / Vaping)
            // Symptoms: "chest hurts", "heart feels irregular", "feel weak"
            // Scans: Lungs: Looks infected, Heart: Very red, Skin: Normal
            // Labs: Stress Elevated, Urine: Toxic markers, HairSkin: Normal
            // ----------------------------------------------------------------
            _illnesses.Add(new Illness(
                "Drug-Induced Cardiac Stress",
                "Cardiac stress induced by drug or substance exposure.",
                "Cardiac Stabilizer + Detox",
                true,
                (patient) =>
                {
                    string complaint = patient.Symptoms.MainComplaint?.ToLower() ?? "";
                    bool hasCardiacSymptoms = complaint.Contains("chest") || complaint.Contains("heart");
                    bool infectedLungs = patient.Scans.Lungs == LungState.LooksInfected;
                    bool veryRedHeart = patient.Scans.Heart == HeartState.VeryRed;
                    bool normalSkin = patient.Scans.Skin == SkinState.Normal;
                    bool normalWBC = patient.Labs.Blood.WBC == WBCLevel.Normal;
                    bool elevatedStress = patient.Labs.Blood.Stress == StressMarkers.Elevated;
                    bool toxicUrine = patient.Labs.Urine == UrineState.ToxicMarkers;
                    bool normalHairSkin = patient.Labs.HairSkin == HairSkinTest.Normal;

                    return hasCardiacSymptoms && infectedLungs && veryRedHeart && normalSkin &&
                           normalWBC && elevatedStress && toxicUrine && normalHairSkin;
                }
            ));

            // ----------------------------------------------------------------
            // VIP ILLNESS
            // 7. TOXIC-INDUCED NEUROLOGICAL DISTRESS (K-pods / Vaping)
            // Symptoms: "very confused", "can't focus", "anxious", "disconnected"
            // Scans: Lungs: Looks infected, Heart: Very red, Skin: Mild irritation
            // Labs: Stress Elevated, Urine: Toxic markers, HairSkin: Normal
            // ----------------------------------------------------------------
            _illnesses.Add(new Illness(
                "Toxic-Induced Neurological Distress",
                "Neurological symptoms caused by toxic substance exposure.",
                "Sedative / Stabilizing Treatment",
                true,
                (patient) =>
                {
                    string complaint = patient.Symptoms.MainComplaint?.ToLower() ?? "";
                    bool hasNeuroSymptoms = complaint.Contains("confus") || complaint.Contains("focus") ||
                                            complaint.Contains("anxious") || complaint.Contains("disconnect");
                    bool infectedLungs = patient.Scans.Lungs == LungState.LooksInfected;
                    bool veryRedHeart = patient.Scans.Heart == HeartState.VeryRed;
                    bool mildSkinIrritation = patient.Scans.Skin == SkinState.MildIrritation;
                    bool elevatedStress = patient.Labs.Blood.Stress == StressMarkers.Elevated;
                    bool toxicUrine = patient.Labs.Urine == UrineState.ToxicMarkers;
                    bool normalHairSkin = patient.Labs.HairSkin == HairSkinTest.Normal;

                    return hasNeuroSymptoms && infectedLungs && veryRedHeart && mildSkinIrritation &&
                           elevatedStress && toxicUrine && normalHairSkin;
                }
            ));

            // ----------------------------------------------------------------
            // 2. PNEUMONIA
            // Symptoms: "coughing for over 2 weeks", "feverish", "hurts to breathe deeply"
            // Scans: Lungs: Looks infected, Heart: Slightly red
            // Labs: WBC Elevated, rest normal
            // ----------------------------------------------------------------
            _illnesses.Add(new Illness(
                "Pneumonia",
                "Serious lung infection requiring immediate treatment.",
                "Antibiotics",
                false,
                (patient) =>
                {
                    bool longDuration = patient.Symptoms.SymptomDurationDays > 14;
                    string complaint = patient.Symptoms.MainComplaint?.ToLower() ?? "";
                    bool hasCough = complaint.Contains("cough");
                    bool infectedLungs = patient.Scans.Lungs == LungState.LooksInfected;
                    bool slightlyRedHeart = patient.Scans.Heart == HeartState.SlightlyRed;
                    bool elevatedWBC = patient.Labs.Blood.WBC == WBCLevel.Elevated;
                    bool normalUrine = patient.Labs.Urine == UrineState.Normal;
                    
                    return longDuration && hasCough && infectedLungs && slightlyRedHeart && elevatedWBC && normalUrine;
                }
            ));



            // ----------------------------------------------------------------
            // 5. SYNTHETIC SUBSTANCE TOXICITY (K-pods / Vaping)
            // Symptoms: "very dizzy", "heart is racing", "short of breath"
            // Scans: Lungs: Looks infected, Heart: Very red, Skin: Mild irritation
            // Labs: Stress Elevated, Urine: Toxic markers, HairSkin: Immune stress
            // ----------------------------------------------------------------
            _illnesses.Add(new Illness(
                "Synthetic Substance Toxicity",
                "Toxicity from synthetic substances like K-pods or vaping.",
                "Detox Treatment",
                true,
                (patient) =>
                {
                    bool infectedLungs = patient.Scans.Lungs == LungState.LooksInfected;
                    bool veryRedHeart = patient.Scans.Heart == HeartState.VeryRed;
                    bool mildSkinIrritation = patient.Scans.Skin == SkinState.MildIrritation;
                    bool normalWBC = patient.Labs.Blood.WBC == WBCLevel.Normal;
                    bool elevatedStress = patient.Labs.Blood.Stress == StressMarkers.Elevated;
                    bool toxicUrine = patient.Labs.Urine == UrineState.ToxicMarkers;
                    bool immuneStress = patient.Labs.HairSkin == HairSkinTest.ImmuneStress;
                    
                    return infectedLungs && veryRedHeart && mildSkinIrritation && 
                           normalWBC && elevatedStress && toxicUrine && immuneStress;
                }
            ));
        }

        /// <summary>
        /// Returns all registered illnesses for diagnosis evaluation.
        /// </summary>
        public static IReadOnlyList<Illness> GetAllIllnesses()
        {
            return _illnesses.AsReadOnly();
        }

        /// <summary>
        /// Get a random illness from the database.
        /// </summary>
        public static Illness GetRandomIllness()
        {
            int index = _rng.Next(_illnesses.Count);
            return _illnesses[index];
        }

        public static Illness getLevel1Illnesses()
        {
            int index = _rng.Next(0,4);
            return _illnesses[index];
        }

        public static Illness getLevel1VIPIllness()
        {   
            //3 is the index of the vip Illness
            return _illnesses[3];
        }

        public static Illness getLevel2Illnesses()
        {
            int index = _rng.Next(1, 7);
            return _illnesses[index];
        }

        public static Illness getLevel2VIPIllness()
        {
            //3 is the index of the vip Illness
            return _illnesses[7];
        }

        /// <summary>
        /// Get illness by name.
        /// </summary>
        public static Illness GetIllnessByName(string name)
        {
            foreach (var illness in _illnesses)
            {
                if (illness.Name.Equals(name, StringComparison.OrdinalIgnoreCase))
                    return illness;
            }
            return null;
        }

        /// <summary>
        /// Register a new illness at runtime.
        /// </summary>
        public static void RegisterIllness(Illness illness)
        {
            _illnesses.Add(illness);
        }
    }

    #endregion

    #region Patient Selector Interface

    /// <summary>
    /// Contract for patient selection mechanisms.
    /// </summary>
    public interface IPatientSelector
    {
        bool HasSelection { get; }
        PatientData? GetSelectedPatient();
        void ClearSelection();
    }

    #endregion
}
