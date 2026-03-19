using Script.Library;
using System;
using System.Collections.Generic;

namespace Script
{
    public class SymptomChecker : IScript
    {
        public SymptomChecker() : base("") {}

        // Engine requires ctor(string)
        public SymptomChecker(string id) : base(id) {}

        // List of symptom names (not serialized)
        private readonly List<string> symptomList = new List<string> 
        { 
            "brain", 
            "lungs", 
            "heart" 
        };

        // --- Serialized fields ---
        [SerializeField] public bool brain;
        [SerializeField] public bool lungs;
        [SerializeField] public bool heart;

        [SerializeField] public string currentSymptoms = "";

        // Key state tracking
        private bool keyWasDownY = false;
        private bool keyWasDownP = false;

        // Use ONE Random instance, not a new Random() each frame
        private static readonly Random rng = new Random();

        public override void Awake()
        {
        }

        public override void Start() {}

        public override void Update(double dt)
        {
            bool isDownY = Input.isKeyPressed(Input.KeyCode.kY);
            bool isDownP = Input.isKeyPressed(Input.KeyCode.kP);

            // -------------------------
            // Y pressed → random symptom
            // -------------------------
            if (isDownY && !keyWasDownY)
            {
                currentSymptoms = GetRandomSymptom();
                Logger.log(Logger.LogLevel.Info,
                    $"New Symptom: {currentSymptoms}");
            }

            // -------------------------
            // P pressed → check answer
            // -------------------------
            if (isDownP && !keyWasDownP)
            {
                bool correct = CheckSymptoms();
                Logger.log(Logger.LogLevel.Info,
                    correct ? "Correct" : "Wrong");
            }

            keyWasDownY = isDownY;
            keyWasDownP = isDownP;
        }

        public override void LateUpdate(double dt) {}
        public override void Free() {}

        // Pick a random symptom
        private string GetRandomSymptom()
        {
            int idx = rng.Next(0, symptomList.Count);
            return symptomList[idx];
        }

        // Validate the selected symptom
        private bool CheckSymptoms()
        {
            switch (currentSymptoms)
            {
                case "brain": return brain && !lungs && !heart;
                case "lungs": return lungs && !brain && !heart;
                case "heart": return heart && !brain && !lungs;
            }
            return false;
        }
    }
}
