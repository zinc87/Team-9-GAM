using Script.Library;

namespace Script
{
    public class PatientTestPanel : IScript
    {
        public static PatientTestPanel Instance { get; private set; }

        [SerializeField] public Script.Library.Object resultsRoot;
        [SerializeField] public Script.Library.Object txtWBC;
        [SerializeField] public Script.Library.Object txtRBC;
        [SerializeField] public Script.Library.Object txtStress;
        [SerializeField] public Script.Library.Object txtUrine;
        [SerializeField] public Script.Library.Object txtHair;

        private enum TestStatus { Unknown, Pending, Done }
        
        public bool IsVisible { get; private set; } = false;
        private string _patientID = null;

        public override void Awake() { Instance = this; }
        
        public override void Start() { Hide(); }

        public override void Update(double dt) 
        { 
            if (GameState.IsGameOver) return;
            if (IsVisible) RefreshResults(); 
        }

        public void Show(string patientID)
        {
            _patientID = patientID;
            IsVisible = true;
            SetAlpha(resultsRoot, 1f);
            SetAlpha(txtWBC, 1f);
            SetAlpha(txtRBC, 1f);
            SetAlpha(txtStress, 1f);
            SetAlpha(txtUrine, 1f);
            SetAlpha(txtHair, 1f);
            RefreshResults();
        }

        public void Hide()
        {
            IsVisible = false;
            SetAlpha(resultsRoot, 0f);
            SetAlpha(txtWBC, 0f);
            SetAlpha(txtRBC, 0f);
            SetAlpha(txtStress, 0f);
            SetAlpha(txtUrine, 0f);
            SetAlpha(txtHair, 0f);
        }

        private void RefreshResults()
        {
            if (_patientID == null) return;
            if (!PatientManager.TryGetPatientData(_patientID, out PatientData data)) return;
            
            TestStatus statusBlood = data.Labs.bloodSampleDeposited ? TestStatus.Done : TestStatus.Unknown;
            TestStatus statusHair = data.Labs.hairSampleDeposited ? TestStatus.Done : TestStatus.Unknown;
            TestStatus statusUrine = data.Labs.urineSampleDeposited ? TestStatus.Done : TestStatus.Unknown;

            UpdateResultText(txtWBC, data.Labs.Blood.WBC.ToString(), statusBlood);
            UpdateResultText(txtRBC, data.Labs.Blood.RBC.ToString(), statusBlood);
            UpdateResultText(txtStress, data.Labs.Blood.Stress.ToString(), statusBlood);
            UpdateResultText(txtUrine, data.Labs.Urine.ToString(), statusUrine);
            UpdateResultText(txtHair, data.Labs.HairSkin.ToString(), statusHair);
        }

        private void UpdateResultText(Script.Library.Object txtObj, string result, TestStatus status)
        {
            string display = "";
            float r = 1f, g = 1f, b = 1f;

            if (status == TestStatus.Unknown) { display = "UNTESTED"; r = 1f; g = 0.85f; b = 0f; }
            else if (status == TestStatus.Pending) { display = "PENDING..."; r = 0.8f; g = 0.8f; b = 0.8f; }
            else 
            {
                display = result.ToUpper();
                if (display == "NORMAL") { r = 0f; g = 1f; b = 0.2f; }
                else if (display == "ABNORMAL") { r = 1f; g = 0.2f; b = 0.2f; }
            }

            SetText(txtObj, display);
            SetTextColor(txtObj, r, g, b);
        }

        private void SetAlpha(Script.Library.Object obj, float a) { if (obj == null) return; var i = obj.getComponent<Image2DComponent>(); if (i != null) { var d = i.Data; d.color.a = a; i.Data = d; } var t = obj.getComponent<TextMeshUIComponent>(); if (t != null) { var d = t.Data; d.color.a = a; t.Data = d; } }
        private void SetText(Script.Library.Object obj, string text) { if (obj == null) return; var txt = obj.getComponent<TextMeshUIComponent>(); if (txt != null) { var d = txt.Data; d.text = text; txt.Data = d; } }
        private void SetTextColor(Script.Library.Object obj, float r, float g, float b) { if (obj == null) return; var txt = obj.getComponent<TextMeshUIComponent>(); if (txt != null) { var d = txt.Data; d.color.r = r; d.color.g = g; d.color.b = b; txt.Data = d; } }

        public override void LateUpdate(double dt) {}
        public override void Free() { Instance = null; }
    }
}