using Script.Library;
using System;
using System.Collections.Generic;

namespace Script
{
    public class SymptomManager : IScript
    {
        public static SymptomManager Instance;

        // Inspector: weight for each symptom (editable)
        [SerializeField] public float brainWeight = 1.0f;
        [SerializeField] public float lungsWeight = 1.0f;
        [SerializeField] public float heartWeight = 1.0f;

        private Random rng = new Random();

        public override void Awake()
        {
            Instance = this;
        }

        public override void Start() { }
        public override void Update(double dt) { }
        public override void LateUpdate(double dt) { }
        public override void Free() { }

        // ----------------------------
        // Assign random symptom
        // ----------------------------
        public void AssignRandomSymptom(StudentScript s)
        {
            float total = brainWeight + lungsWeight + heartWeight;
            float r = (float)(rng.NextDouble() * total);

            s.brain = s.lungs = s.heart = false;

            if (r < brainWeight)
            {
                s.brain = true;
                s.currentSymptom = "brain";
                return;
            }

            r -= brainWeight;
            if (r < lungsWeight)
            {
                s.lungs = true;
                s.currentSymptom = "lungs";
                return;
            }

            s.heart = true;
            s.currentSymptom = "heart";
        }
    }
}
