using Script.Library;
using System.Collections.Generic;

namespace Script
{
    public class PatientTestPanel : IScript
    {
        [SerializeField] public Object testPanelRoot;   
        [SerializeField] public Object testPanelBackground; 
        
        [SerializeField] public Object txtHair;    
        [SerializeField] public Object txtStress;  
        [SerializeField] public Object txtUrine;
        [SerializeField] public Object txtRBC;     
        [SerializeField] public Object txtWBC;     

        [SerializeField] public Object closeButton;     
        [SerializeField] public Image2D closeButtonNormal;
        [SerializeField] public Image2D closeButtonHover;

        private enum TestStatus { Unknown, Pending, Done }
        private Dictionary<string, Dictionary<string, TestStatus>> testHistory = new Dictionary<string, Dictionary<string, TestStatus>>();
        private string currentPatientID = null;
        public bool IsVisible { get; private set; } = false;

        private AudioInstance hoverSound;
        private AudioInstance clickSound;
        private Object _lastHoveredButton = null;

        public static PatientTestPanel Instance { get; private set; }

        public PatientTestPanel() : base("") { }
        public PatientTestPanel(string id) : base(id) { }

        public override void Awake() 
        { 
            Instance = this; 
        }

        public override void Start() 
        { 
            Hide(); 

            hoverSound = Audio.getAudioInstance("HoverButton", Obj);
            clickSound = Audio.getAudioInstance("ButtonPress_003", Obj);
        }

        public override void Update(double dt)
        {
            if (!IsVisible) return;

            Vector2D mp;
            Input.getCurMosusePos(out mp);

            UpdateHover(closeButton, closeButtonNormal, closeButtonHover, mp);

            Object _currentHovered = null;
            if (IsMouseOver(closeButton, mp)) _currentHovered = closeButton;

            if (_currentHovered != _lastHoveredButton)
            {
                if (_currentHovered != null && hoverSound != null)
                    Audio.playAudio(hoverSound);
                _lastHoveredButton = _currentHovered;
            }

            // Keep visuals refreshing so text updates (Pending -> Result)
            RefreshVisuals();

            if (Input.isMouseClicked(Input.Mouse.kMOUSE_BUTTON_LEFT))
            {
                if (clickSound != null) Audio.playAudio(clickSound);

                if (IsMouseOver(closeButton, mp)) 
                {
                    Hide();
                    if (PatientUIPanel.Instance != null && currentPatientID != null) 
                    {
                        PatientUIPanel.Instance.OpenDetailsFor(currentPatientID);
                    }
                    return; 
                }
            }
        }

        private void UpdateHover(Object btn, Image2D normal, Image2D hover, Vector2D mp)
        {
            if (btn == null) return;

            if (IsMouseOver(btn, mp)) 
            {
                ApplyTexture(btn, hover);
            }
            else 
            {
                ApplyTexture(btn, normal);
            }
        }
       
        private void ApplyTexture(Object obj, Image2D tex) 
        { 
            if (obj == null || tex == null) return; 

            var img = obj.getComponent<Image2DComponent>(); 
            if (img != null) 
            { 
                var d = img.Data; 
                if (d.hashed != tex.hashedID) 
                { 
                    d.hashed = tex.hashedID; 
                    img.Data = d; 
                } 
            } 
        }

        public void Show(string patientID) 
        { 
            currentPatientID = patientID; 
            IsVisible = true; 
            
            SetAlpha(testPanelRoot, 1f); 
            SetAlpha(testPanelBackground, 1f); 
            
            // Show Texts
            SetAlpha(txtHair, 1f);
            SetAlpha(txtStress, 1f);
            SetAlpha(txtUrine, 1f);
            SetAlpha(txtRBC, 1f);
            SetAlpha(txtWBC, 1f);

            SetAlpha(closeButton, 1f); 
            RefreshVisuals(); 
        }

        public void Hide() 
        { 
            IsVisible = false; 
            
            SetAlpha(testPanelRoot, 0f); 
            SetAlpha(testPanelBackground, 0f); 
            
            // Hide Texts
            SetAlpha(txtHair, 0f);
            SetAlpha(txtStress, 0f);
            SetAlpha(txtUrine, 0f);
            SetAlpha(txtRBC, 0f);
            SetAlpha(txtWBC, 0f);

            SetAlpha(closeButton, 0f); 
        }
        
        private void RefreshVisuals() 
        { 
            if (currentPatientID == null) return; 
            if (!PatientManager.TryGetPatientData(currentPatientID, out PatientData data)) return; 
            
            TestStatus currStatusBlood = TestStatus.Unknown;
            TestStatus currStatusHair = TestStatus.Unknown;
            TestStatus currStatusUrine = TestStatus.Unknown;

            if(data.Labs.bloodSampleDeposited)
                currStatusBlood = TestStatus.Done;
            if(data.Labs.hairSampleDeposited)
                currStatusHair = TestStatus.Done;
            if(data.Labs.urineSampleDeposited)
                currStatusUrine = TestStatus.Done;

            UpdateTextVisuals(txtHair, "Hair", data.Labs.HairSkin.ToString(), currStatusHair); 
            UpdateTextVisuals(txtUrine, "Urine", data.Labs.Urine.ToString(), currStatusUrine);
            UpdateTextVisuals(txtStress, "Stress", data.Labs.Blood.Stress.ToString(), currStatusBlood);
            UpdateTextVisuals(txtRBC, "RBC", data.Labs.Blood.RBC.ToString(), currStatusBlood); 
            UpdateTextVisuals(txtWBC, "WBC", data.Labs.Blood.WBC.ToString(), currStatusBlood); 
        }

        private void UpdateTextVisuals(Object txt, string testName, string result, TestStatus currStatus) 
        { 
            TestStatus status = currStatus; 
            if (testHistory.ContainsKey(currentPatientID) && testHistory[currentPatientID].ContainsKey(testName)) 
            {
                status = testHistory[currentPatientID][testName]; 
            }
            
            string display = ""; 
            float r = 1f, g = 1f, b = 1f; // Default to White

            if (status == TestStatus.Unknown) 
            {
                display = "UNTESTED"; 
                r = 1f; g = 0.85f; b = 0f; // Yellow/Orange
            }
            else if (status == TestStatus.Pending) 
            {
                display = "PENDING..."; 
                r = 0.8f; g = 0.8f; b = 0.8f; // Light Gray
            }
            else 
            {
                // Convert to uppercase to match "NORMAL" or "ABNORMAL" perfectly
                display = result.ToUpper(); 

                if (display == "NORMAL") 
                {
                    r = 0f; g = 1f; b = 0.2f; // Green
                } 
                else if (display == "ABNORMAL") 
                {
                    r = 1f; g = 0.2f; b = 0.2f; // Red
                }
            }
            
            SetText(txt, display); 
            SetTextColor(txt, r, g, b); // Apply the color!
        }

        private void SetTextColor(Object obj, float r, float g, float b)
        {
            if (obj == null) return; 

            var txt = obj.getComponent<TextMeshUIComponent>(); 
            if (txt != null) 
            { 
                var d = txt.Data; 
                d.color.r = r; 
                d.color.g = g; 
                d.color.b = b; 
                // We leave d.color.a alone so it doesn't mess with the Hide/Show alpha fading
                txt.Data = d; 
            } 
        }

        private void SetAlpha(Object obj, float a) 
        { 
            if (obj == null) return; 

            var img = obj.getComponent<Image2DComponent>(); 
            if (img != null) 
            { 
                var d = img.Data; 
                d.color.a = a; 
                img.Data = d; 
            } 

            var txt = obj.getComponent<TextMeshUIComponent>(); 
            if (txt != null) 
            { 
                var d = txt.Data; 
                d.color.a = a; 
                txt.Data = d; 
            } 
        }

        private void SetText(Object obj, string t) 
        { 
            if (obj == null) return; 

            var txt = obj.getComponent<TextMeshUIComponent>(); 
            if (txt != null) 
            { 
                var d = txt.Data; 
                d.text = t; 
                txt.Data = d; 
            } 
        }

        private bool IsMouseOver(Object obj, Vector2D mouse) 
        { 
            if (obj == null) return false; 

            var rt = obj.getComponent<RectTransformComponent>(); 
            if (rt == null) return false; 

            var d = rt.Data; 
            float hx = d.scale.x * 0.5f; 
            float hy = d.scale.y * 0.5f; 

            Vector2D tl = new Vector2D(d.position.x - hx, d.position.y + hy); 
            Vector2D br = new Vector2D(d.position.x + hx, d.position.y - hy); 

            return Utility.AABB(tl, br, mouse); 
        }

        public override void LateUpdate(double dt) { }

        public override void Free() 
        { 
            Instance = null; 
        }
    }
}