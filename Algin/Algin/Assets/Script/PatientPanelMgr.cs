using Script.Library;
using System.Collections.Generic;
using System.Text;

namespace Script
{
    public class PatientPanelMgr : IScript
    {
        // =========================================================
        // 1. LIST VIEW VARIABLES
        // =========================================================
        [SerializeField] public Object listPanelRoot;
        
        // Patient List Buttons
        [SerializeField] public Object btn1;
        [SerializeField] public Object btn2;
        [SerializeField] public Object btn3;
        [SerializeField] public Object btn4;
        [SerializeField] public Object btn5;
        [SerializeField] public Object btn6;
        [SerializeField] public Object btn7;
        [SerializeField] public Object btn8;
        
        // List Control Buttons
        [SerializeField] public Object listCloseButton;
        [SerializeField] public Object confirmButton;
        
        // List Textures
        [SerializeField] public Image2D patientProfileNormal; 
        [SerializeField] public Image2D patientProfileHover;
        [SerializeField] public Image2D listCloseNormal;
        [SerializeField] public Image2D listCloseHover;
        [SerializeField] public Image2D confirmNormal;
        [SerializeField] public Image2D confirmHover;

        private Object[] patientButtons;

        // =========================================================
        // 2. DETAILS VIEW VARIABLES
        // =========================================================
        [SerializeField] public Object detailsPanelRoot;
        [SerializeField] public Object detailsPatientImage;
        [SerializeField] public Object detailsNameText;
        [SerializeField] public Object detailsAgeText;
        [SerializeField] public Object detailsGenderText;
        [SerializeField] public Object detailsDiagnosisText;

        // Gender Profile Images (NEW)
        [SerializeField] public Image2D profileMale1;//glasses male, grey sleeveless
        [SerializeField] public Image2D profileMale2;// blue shirt male
        [SerializeField] public Image2D profileFemale;//pink shirt female, blonde
        [SerializeField] public Image2D profilePatient4;// purple hair female
        [SerializeField] public Image2D profilePatient5;// white top female
        [SerializeField] public Image2D profilePatient6SA;//RASH female
        [SerializeField] public Image2D profilePatient7;// grey shirt male
        [SerializeField] public Image2D profileVIP;//VIP

        // Internal Memory (Cache) to remember faces
        private Dictionary<string, Image2D> patientImageCache = new Dictionary<string, Image2D>();
        private System.Random rand = new System.Random();

        // Details Buttons
        [SerializeField] public Object detailsTestResultsButton; 
        [SerializeField] public Image2D detailsTestResultsNormal;
        [SerializeField] public Image2D detailsTestResultsHover;

        [SerializeField] public Object detailsFurtherTestingButton; 
        [SerializeField] public Image2D detailsFurtherTestingNormal;
        [SerializeField] public Image2D detailsFurtherTestingHover;

        [SerializeField] public Object detailsDiagnoseButton;
        [SerializeField] public Image2D detailsDiagnoseNormal;
        [SerializeField] public Image2D detailsDiagnoseHover;

        [SerializeField] public Object detailsCloseButton;
        [SerializeField] public Image2D detailsCloseNormal;
        [SerializeField] public Image2D detailsCloseHover;

        [SerializeField] public Object detailsBackButton;
        [SerializeField] public Image2D detailsBackNormal;
        [SerializeField] public Image2D detailsBackHover;

        // =========================================================
        // 3. STATE MANAGEMENT
        // =========================================================
        public enum ViewState { Closed, List, Details }
        public ViewState state = ViewState.Closed;
        private string selectedPatientID = null;
        private bool wasBlocked = false;
        private int selectedPatientNo = 0;

        private AudioInstance hoverSound;
        private AudioInstance clickSound;
        private Object _lastHoveredButton = null;

        public static PatientPanelMgr Instance { get; private set; }
        public PatientPanelMgr() : base("") { }
        public PatientPanelMgr(string id) : base(id) { }
        public override void Awake() { Instance = this; }

        public override void Start()
        {
            // Group buttons for easy looping
            patientButtons = new Object[] { btn1, btn2, btn3, btn4, btn5, btn6, btn7, btn8 };
            
            ResetAllTextures(); 
            // Ensure everything is hidden at start
            ShowAll(false);
            GameState.SetUIState(false); 

            hoverSound = Audio.getAudioInstance("HoverButton", Obj);
            clickSound = Audio.getAudioInstance("ButtonPress_003", Obj);
        }

        public override void Update(double dt)
        {
            if (GameState.IsGameOver) return;

            // Check if blocked by sub-panels (Diagnose or Test)
            bool isBlocked = (DiagnosePanelMgr.Instance != null && DiagnosePanelMgr.Instance.IsVisible) || 
                             (PatientTestPanel.Instance != null && PatientTestPanel.Instance.IsVisible);

            if (isBlocked) { wasBlocked = true; return; }

            // Prevent accidental clicks immediately after unblocking
            if (wasBlocked) {
                if (Input.isMouseClicked(Input.Mouse.kMOUSE_BUTTON_LEFT)) return; 
                wasBlocked = false; 
            }

            if (state == ViewState.Closed) return;

            Vector2D mp;
            Input.getCurMosusePos(out mp);

            // Handle Logic based on which View is open
            if (state == ViewState.List)
            {
                UpdateListHover(mp);
                UpdateListInteraction(mp);
            }
            else if (state == ViewState.Details)
            {
                UpdateDetailsHover(mp);
                UpdateDetailsInteraction(mp);
            }
        }

        // =========================================================
        // 4. LIST LOGIC
        // =========================================================
        private void UpdateListHover(Vector2D mp)
        {
            Object _currentHovered = null;

            UpdateHover(listCloseButton, listCloseNormal, listCloseHover, mp);
            if (IsOver(listCloseButton, mp)) _currentHovered = listCloseButton;
            UpdateHover(confirmButton, confirmNormal, confirmHover, mp);
            if (IsOver(confirmButton, mp)) _currentHovered = confirmButton;
            
            // Handle patient slots hover
            RefreshSlotVisuals();
            if (patientButtons != null) { for (int i = 0; i < patientButtons.Length; i++) HandlePatientHover(i, mp); }

            if (_currentHovered != _lastHoveredButton)
            {
                if (_currentHovered != null && hoverSound != null)
                    Audio.playAudio(hoverSound);
                _lastHoveredButton = _currentHovered;
            }
        }

        private void UpdateListInteraction(Vector2D mp)
        {
            if (!Input.isMouseClicked(Input.Mouse.kMOUSE_BUTTON_LEFT)) return;

            if (clickSound != null) Audio.playAudio(clickSound);

            if (IsOver(listCloseButton, mp) || IsOver(confirmButton, mp)) { CloseAll(); return; }
            
            for (int i = 0; i < patientButtons.Length; i++) {
                Object btn = patientButtons[i];
                if (btn != null && IsOver(btn, mp)) {
                    selectedPatientNo = i;
                    string pid = GetPatientIDByIndex(i);
                    // Open details if a valid patient is clicked
                    if (pid != null) OpenDetails(pid);
                }
            }
        }

        // =========================================================
        // 5. DETAILS LOGIC
        // =========================================================
        private void UpdateDetailsHover(Vector2D mp)
        {
            Object _currentHovered = null;

            UpdateHover(detailsTestResultsButton, detailsTestResultsNormal, detailsTestResultsHover, mp);
            if (IsOver(detailsTestResultsButton, mp)) _currentHovered = detailsTestResultsButton;
            UpdateHover(detailsFurtherTestingButton, detailsFurtherTestingNormal, detailsFurtherTestingHover, mp);
            if (IsOver(detailsFurtherTestingButton, mp)) _currentHovered = detailsFurtherTestingButton;
            UpdateHover(detailsDiagnoseButton, detailsDiagnoseNormal, detailsDiagnoseHover, mp);
            if (IsOver(detailsDiagnoseButton, mp)) _currentHovered = detailsDiagnoseButton;
            UpdateHover(detailsCloseButton, detailsCloseNormal, detailsCloseHover, mp);
            if (IsOver(detailsCloseButton, mp)) _currentHovered = detailsCloseButton;
            UpdateHover(detailsBackButton, detailsBackNormal, detailsBackHover, mp);
            if (IsOver(detailsBackButton, mp)) _currentHovered = detailsBackButton;

            if (_currentHovered != _lastHoveredButton)
            {
                if (_currentHovered != null && hoverSound != null)
                    Audio.playAudio(hoverSound);
                _lastHoveredButton = _currentHovered;
            }
        }

        private void UpdateDetailsInteraction(Vector2D mp)
        {
            if (!Input.isMouseClicked(Input.Mouse.kMOUSE_BUTTON_LEFT)) return;

            if (clickSound != null) Audio.playAudio(clickSound);

            if (IsOver(detailsBackButton, mp)) { OpenList(); return; }
            if (IsOver(detailsCloseButton, mp)) { CloseAll(); return; }

            // Test Results Button
            if (IsOver(detailsTestResultsButton, mp))
            {
                if (PatientTestPanel.Instance != null) { PatientTestPanel.Instance.Show(selectedPatientID); HideDetailsPanel(); wasBlocked = true; }
                if(selectedPatientNo < WaypointPatientSpawner._allPatients.Length && WaypointPatientSpawner._allPatients[selectedPatientNo] != null) WaypointPatientSpawner._allPatients[selectedPatientNo].AdvanceToNextWaypoint();
                return;
            }

            // Further Testing Button (Placeholder)
            if (IsOver(detailsFurtherTestingButton, mp))
            {
                // Logger.log(Logger.LogLevel.Warning, "Hit");
                if (WaypointPatientSpawner._allPatients[selectedPatientNo] != null)
                {
                    WaypointPatientSpawner._allPatients[selectedPatientNo].MoveToTesting();
                }
                return;
            }

            // Diagnose Button
            if (IsOver(detailsDiagnoseButton, mp))
            {
                if (DiagnosePanelMgr.Instance != null) { DiagnosePanelMgr.Instance.ShowDiagnosePanel(selectedPatientID); HideDetailsPanel(); wasBlocked = true; }
                return;
            }
        }

        // =========================================================
        // 6. MAIN FUNCTIONS (OPEN/CLOSE)
        // =========================================================
        
        public void OpenList() 
        { 
            state = ViewState.List; 
            selectedPatientID = null; 
            GameState.SetUIState(true); 
            ResetAllTextures(); 
            RefreshSlotVisuals(); 
            
            // Show List, Hide Details
            ShowListUI(true);
            HideDetailsPanel(); 
            
            if (ObjectivesMgr.Instance != null) ObjectivesMgr.Instance.Show(false); 
        }
        
        public void CloseAll() 
        { 
            state = ViewState.Closed; 
            selectedPatientID = null; 
            ResetAllTextures(); 
            ShowAll(false); // Hides EVERYTHING
            GameState.SetUIState(false); 
            if (ObjectivesMgr.Instance != null) ObjectivesMgr.Instance.Show(true); 
            if (DiagnosePanelMgr.Instance != null) DiagnosePanelMgr.Instance.SetVisible(false); 
        }

        private void OpenDetails(string patientID) 
        { 
            selectedPatientID = patientID; 
            state = ViewState.Details; 
            GameState.SetUIState(true); 
            ResetAllTextures(); 

            // Hide List
            ApplyAlpha(listPanelRoot, 0f); 
            SetArrayAlpha(patientButtons, 0f); 
            ApplyAlpha(listCloseButton, 0f); 
            ApplyAlpha(confirmButton, 0f); 

            // Use the Master Function
            Image2D finalImage = GetSavedPatientImage(patientID);
            ApplyTexture(detailsPatientImage, finalImage);

            // Show Details
            ApplyAlpha(detailsPanelRoot, 1f); 
            ApplyAlpha(detailsPatientImage, 1f); 
            ApplyAlpha(detailsNameText, 1f); 
            ApplyAlpha(detailsAgeText, 1f); 
            ApplyAlpha(detailsGenderText, 1f); 
            ApplyAlpha(detailsDiagnosisText, 1f); 
            ApplyAlpha(detailsTestResultsButton, 1f); 
            ApplyAlpha(detailsFurtherTestingButton, 1f); 
            ApplyAlpha(detailsDiagnoseButton, 1f); 
            ApplyAlpha(detailsCloseButton, 1f); 
            ApplyAlpha(detailsBackButton, 1f); 

            RefreshDetailsText(); 
            if (ObjectivesMgr.Instance != null) ObjectivesMgr.Instance.Show(false); 
        }

        // Public helpers for external scripts
        public void OpenDetailsFromDiagnose(string patientID) 
        { 
            selectedPatientID = patientID; 
            if(selectedPatientID == null)
                Logger.log(Logger.LogLevel.Error, "OpenDetailsFromDiagnose Selected Patient is NULL");

            Logger.log(Logger.LogLevel.Warning, $"OpenDetailsFromDiagnose Selected Patient: {selectedPatientID} ");
            OpenDetails(patientID); 
        }
        public void OpenDetailsFromTest(string patientID) { OpenDetails(patientID); }

        // =========================================================
        // 7. VISUAL HELPERS
        // =========================================================
        
        private void ShowListUI(bool show) {
            float a = show ? 1f : 0f;
            ApplyAlpha(listPanelRoot, a);
            SetArrayAlpha(patientButtons, a);
            ApplyAlpha(listCloseButton, a);
            ApplyAlpha(confirmButton, a);
        }

        private void ShowDetailsUI(bool show) {
            float a = show ? 1f : 0f;
            ApplyAlpha(detailsPanelRoot, a);
            ApplyAlpha(detailsPatientImage, a);
            ApplyAlpha(detailsNameText, a);
            ApplyAlpha(detailsAgeText, a);
            ApplyAlpha(detailsGenderText, a);
            ApplyAlpha(detailsDiagnosisText, a);
            ApplyAlpha(detailsTestResultsButton, a);
            ApplyAlpha(detailsFurtherTestingButton, a);
            ApplyAlpha(detailsDiagnoseButton, a);
            ApplyAlpha(detailsCloseButton, a);
            ApplyAlpha(detailsBackButton, a);
        }

        private void HideDetailsPanel() { ShowDetailsUI(false); }
        private void ShowAll(bool show) { ShowListUI(show); ShowDetailsUI(show); }

        private void UpdateHover(Object btn, Image2D normal, Image2D hover, Vector2D mp) {
            if (btn == null) return;
            if (IsOver(btn, mp)) ApplyTexture(btn, hover);
            else ApplyTexture(btn, normal);
        }

        private void HandlePatientHover(int index, Vector2D mouse) { 
            Object btn = patientButtons[index]; 
            if (btn == null) return; 
            if (GetPatientIDByIndex(index) == null) { ApplyTexture(btn, patientProfileNormal); return; } 
            if (IsOver(btn, mouse)) ApplyTexture(btn, patientProfileHover); else ApplyTexture(btn, patientProfileNormal); 
        }

        private void RefreshSlotVisuals() { 
            if (patientButtons == null) return; 
            for (int i = 0; i < patientButtons.Length; i++) { 
                Object btn = patientButtons[i]; 
                if (btn == null) continue; 
                string pid = GetPatientIDByIndex(i); 
                var img = btn.getComponent<Image2DComponent>(); 
                if (img != null) { 
                    var d = img.Data; 
                    if (pid != null) { d.color.r = 1f; d.color.g = 1f; d.color.b = 1f; } 
                    else { d.color.r = 0.3f; d.color.g = 0.3f; d.color.b = 0.3f; } 
                    img.Data = d; 
                } 
            } 
        }

        private void RefreshDetailsText() { 
            if (selectedPatientID == null) return; 
            if (PatientManager.TryGetPatientData(selectedPatientID, out PatientData data)) { 
                SetText(detailsNameText, data.PatientName); 
                SetText(detailsAgeText, data.Age.ToString()); 
                SetText(detailsGenderText, data.Gender.ToString()); 
                SetText(detailsDiagnosisText, "\"" + data.Symptoms.MainComplaint + "\""); 
            } 
        }

        public Image2D GetSavedPatientImage(string patientID)
        {
            // A. Check if we already remembered this patient's face
            if (patientImageCache.ContainsKey(patientID))
            {
                return patientImageCache[patientID];
            }

            // B. If not in memory, we MUST generate it now
            if (PatientManager.TryGetPatientData(patientID, out PatientData data))
            {
                // Default image so it's always assigned
                Image2D generatedImage = profileMale1;

                if (data.meshNumber == 0)
                {
                    generatedImage = profileMale2; // blue shirt male
                }
                else if (data.meshNumber == 1)
                {
                    generatedImage = profileMale1; //glasses male, grey sleeveless
                }
                else if (data.meshNumber == 2)
                {
                    generatedImage = profileFemale;//pink shirt female, blonde
                }
                else if (data.meshNumber == 3)
                {
                    generatedImage = profilePatient4;// purple hair female
                }
                else if (data.meshNumber == 4)
                {
                    //generatedImage = profilePatient5;// white top female
                    generatedImage = profilePatient4;// purple hair female
                }
                else if (data.meshNumber == 5)
                {
                    generatedImage = profilePatient7;// grey shirt male
                }
                else if (data.meshNumber == 6)
                {
                    generatedImage = profilePatient6SA;//RASH female
                }
                else if (data.meshNumber == 7)
                {
                    generatedImage = profileVIP;//VIP
                }

                // C. SAVE IT to memory so it never changes again
                patientImageCache[patientID] = generatedImage;
                return generatedImage;
            }

            return profileMale1;
        }

        private string GetPatientIDByIndex(int index) { List<string> allIDs = PatientManager.GetAllPatientObjectIds(); if (allIDs != null && index < allIDs.Count) return allIDs[index]; return null; }
        private void ResetAllTextures() { if (patientButtons == null) return; foreach (var btn in patientButtons) ApplyTexture(btn, patientProfileNormal); }
        private void ApplyTexture(Object obj, Image2D tex) { if (obj == null || tex == null) return; var img = obj.getComponent<Image2DComponent>(); if (img != null) { var d = img.Data; if (d.hashed != tex.hashedID) { d.hashed = tex.hashedID; img.Data = d; } } }
        private void SetArrayAlpha(Object[] objs, float a) { if (objs!=null) foreach(var o in objs) ApplyAlpha(o, a); }
        private void ApplyAlpha(Object o, float a) { if (o==null) return; var i=o.getComponent<Image2DComponent>(); if (i!=null) { var d=i.Data; d.color.a=a; i.Data=d; } var t=o.getComponent<TextMeshUIComponent>(); if (t!=null) { var d=t.Data; d.color.a=a; t.Data=d; } }
        private void SetText(Object o, string tx) { if (o==null) return; var t=o.getComponent<TextMeshUIComponent>(); if (t!=null) { var d=t.Data; d.text=tx; t.Data=d; } }
        private bool IsOver(Object o, Vector2D mp) { if (o==null) return false; var rt=o.getComponent<RectTransformComponent>(); if (rt==null) return false; var d=rt.Data; float hx=d.scale.x*0.5f; float hy=d.scale.y*0.5f; Vector2D tl=new Vector2D(d.position.x-hx, d.position.y+hy); Vector2D br=new Vector2D(d.position.x+hx, d.position.y-hy); return Utility.AABB(tl, br, mp); }
        
        public override void LateUpdate(double dt) { }
        public override void Free() { Instance = null; }
    }
}