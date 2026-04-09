using Script.Library;
using PrivateAPI;
using System.Collections.Generic;

namespace Script
{
    public class PatientPanelMgr : IScript
    {
        public static PatientPanelMgr Instance { get; private set; }

        // ==========================================
        // UNIVERSAL NAVIGATION (Tabs & Back Button)
        // ==========================================
        [SerializeField] public Script.Library.Object backButton;
        [SerializeField] public Image2D backNormal;
        [SerializeField] public Image2D backHover;

        [SerializeField] public Image2D highlightTexture;

        public enum Tab { Profile, Results, Diagnose }
        public Tab currentTab { get; private set; } = Tab.Profile;

        [SerializeField] public Script.Library.Object tabProfileBtn;
        [SerializeField] public Script.Library.Object tabResultsBtn;
        [SerializeField] public Script.Library.Object tabDiagnoseBtn;

        [SerializeField] public Image2D tabProfileNormal;
        [SerializeField] public Image2D tabResultsNormal;
        [SerializeField] public Image2D tabDiagnoseNormal;
        [SerializeField] public Script.Library.Object tabHighlightOverlay; 

        // ==========================================
        // PROFILE TAB CONTENT
        // ==========================================
        [SerializeField] public Script.Library.Object profileRoot; 
        [SerializeField] public Script.Library.Object profilePatientImage;
        [SerializeField] public Script.Library.Object profileNameText;
        [SerializeField] public Script.Library.Object profileAgeGenderText; 
        [SerializeField] public Script.Library.Object profileDiagnosisLabel; 
        [SerializeField] public Script.Library.Object profileDiagnosisText;  
        
        [SerializeField] public Script.Library.Object furtherTestingBtn;
        [SerializeField] public Image2D furtherTestingNormal;
        [SerializeField] public Image2D furtherTestingHover;

        // FACES CACHE
        [SerializeField] public Image2D profileMale1;
        [SerializeField] public Image2D profileMale2;
        [SerializeField] public Image2D profileFemale;
        [SerializeField] public Image2D profilePatient4;
        [SerializeField] public Image2D profilePatient6SA;
        [SerializeField] public Image2D profilePatient7;
        [SerializeField] public Image2D profileVIP;
        private Dictionary<string, Image2D> patientImageCache = new Dictionary<string, Image2D>();

        // ==========================================
        // OVERLAYS (Camera & VIP)
        // ==========================================
        [SerializeField] public Script.Library.Object vipOverlayText;
        [SerializeField] public float vipOverlayDuration = 2.5f;
        private float vipOverlayTimer = 0f;
        private const string VIP_OVERLAY_MESSAGE = "";

        [SerializeField] public Script.Library.Object corridorCameraObj;
        [SerializeField] public Script.Library.Object cameraOverlayText;
        [SerializeField] public float cameraOverlayDuration = 3.0f;
        private bool _cameraOverlayActive = false;
        private float _cameraOverlayTimer = 0f;
        private HashSet<string> _overlayTriggeredPatients = new HashSet<string>();
        private const string CAMERA_OVERLAY_MESSAGE = "Patient Coming";

        // ==========================================
        // STATE & AUDIO
        // ==========================================
        public bool IsOpen { get; private set; } = false;
        public string currentPatientID { get; private set; } = null;

        private AudioInstance openPanelSound;
        private AudioInstance hoverSound;
        private AudioInstance clickSound;
        private Script.Library.Object _lastHoveredButton = null;
        
        private bool _wasMouseDown = false;

        public override void Awake() { Instance = this; }

        public override void Start()
        {
            hoverSound = Audio.getAudioInstance("HoverButton", Obj);
            clickSound = Audio.getAudioInstance("ButtonPress_003", Obj);
            openPanelSound = Audio.getAudioInstance("Computer_Typing_Button_7", Obj);

            ApplyTexture(tabHighlightOverlay, highlightTexture);

            if (vipOverlayText != null) { SetText(vipOverlayText, VIP_OVERLAY_MESSAGE); SetAlpha(vipOverlayText, 0f); }
            if (cameraOverlayText != null) { SetText(cameraOverlayText, CAMERA_OVERLAY_MESSAGE); SetAlpha(cameraOverlayText, 0f); }

            if (corridorCameraObj != null)
            {
                CameraOverlay.SetCamera(corridorCameraObj);
                CameraOverlay.SetViewport(0.0f, 0.0f, 0.3f, 0.3f);
                CameraOverlay.SetActive(false);
            }

            ClosePanel();
        }

        public override void Update(double dt)
        {
            if (GameState.IsGameOver)
            {
                // Hide overlays immediately when game ends
                if (vipOverlayText != null) SetAlpha(vipOverlayText, 0f);
                if (cameraOverlayText != null) SetAlpha(cameraOverlayText, 0f);
                CameraOverlay.SetActive(false);
                return;
            }

            if (vipOverlayText != null && vipOverlayTimer > 0f)
            {
                vipOverlayTimer -= (float)dt;
                if (vipOverlayTimer <= 0f) { vipOverlayTimer = 0f; SetAlpha(vipOverlayText, 0f); }
            }
            if (_cameraOverlayActive)
            {
                _cameraOverlayTimer += (float)dt;
                if (_cameraOverlayTimer >= cameraOverlayDuration)
                {
                    _cameraOverlayActive = false;
                    _cameraOverlayTimer = 0f;
                    CameraOverlay.SetActive(false);
                    SetAlpha(cameraOverlayText, 0f);
                }
            }

            if (!IsOpen) return;
            if (PauseMenu.Instance != null && PauseMenu.Instance.IsPaused) return;

            Vector2D mp;
            Input.getCurMosusePos(out mp);
            
            bool isMouseDown = Input.isMousePressed(Input.Mouse.kMOUSE_BUTTON_LEFT);
            bool clicked = isMouseDown && !_wasMouseDown;
            _wasMouseDown = isMouseDown;

            Script.Library.Object currentHover = null;

            UpdateHover(backButton, backNormal, backHover, mp, ref currentHover);
            if (clicked && IsOver(backButton, mp)) { ClosePanel(); return; }

            if (IsOver(tabProfileBtn, mp)) currentHover = tabProfileBtn;
            if (IsOver(tabResultsBtn, mp)) currentHover = tabResultsBtn;
            if (IsOver(tabDiagnoseBtn, mp)) currentHover = tabDiagnoseBtn;

            if (clicked)
            {
                if (IsOver(tabProfileBtn, mp) && currentTab != Tab.Profile) { SwitchTab(Tab.Profile); PlayClick(); }
                if (IsOver(tabResultsBtn, mp) && currentTab != Tab.Results) { SwitchTab(Tab.Results); PlayClick(); }
                if (IsOver(tabDiagnoseBtn, mp) && currentTab != Tab.Diagnose) { SwitchTab(Tab.Diagnose); PlayClick(); }
            }

            if (currentTab == Tab.Profile)
            {
                UpdateHover(furtherTestingBtn, furtherTestingNormal, furtherTestingHover, mp, ref currentHover);
                if (clicked && IsOver(furtherTestingBtn, mp))
                {
                    PlayClick();
                    var all = WaypointPatientControllerManager.GetAllControllers();
                    foreach (var p in all) { if (p != null && p.ObjectId == currentPatientID) p.MoveToTesting(); }
                    TutorialTextMgr.Instance.triggar(TutorialTextMgr.promptState.Change_To_Scanner);
                    ClosePanel();
                }
            }

            if (currentHover != _lastHoveredButton)
            {
                if (currentHover != null && hoverSound != null) Audio.playAudio(hoverSound);
                _lastHoveredButton = currentHover;
            }
        }

        public void OpenActivePatient()
        {
            var ids = PatientManager.GetAllPatientObjectIds();
            if (ids != null)
            {
                foreach (string id in ids)
                {
                    if (!string.IsNullOrEmpty(id))
                    {
                        OpenPanel(id);
                        return; 
                    }
                }
            }
        }

        public void OpenPanel(string patientID)
        {
            if (string.IsNullOrEmpty(patientID)) return;

            if (openPanelSound != null && !IsOpen) Audio.playAudio(openPanelSound);

            currentPatientID = patientID;
            IsOpen = true;
            GameState.SetUIState(true);

            // =================================================================
            // FIX: When you open the panel, shout "Next Patient!" to the 3D model
            // =================================================================
            var allControllers = WaypointPatientControllerManager.GetAllControllers();
            foreach (var p in allControllers)
            {
                if (p != null && p.ObjectId == patientID)
                {
                    if (p.CurrentTarget == WaypointTarget.AtWaypoint1)
                    {
                        p.MoveToCounter();
                    }
                    break;
                }
            }

            if (!_overlayTriggeredPatients.Contains(patientID))
            {
                if (corridorCameraObj != null && cameraOverlayDuration > 0f)
                {
                    CameraOverlay.SetCamera(corridorCameraObj);
                    CameraOverlay.SetViewport(0.0f, 0.0f, 0.3f, 0.3f);
                    CameraOverlay.SetActive(true);
                    _cameraOverlayActive = true;
                    _cameraOverlayTimer = 0f;
                    if (cameraOverlayText != null) { SetText(cameraOverlayText, CAMERA_OVERLAY_MESSAGE); SetAlpha(cameraOverlayText, 1f); }
                }
                _overlayTriggeredPatients.Add(patientID);
            }

            if (PatientManager.IsPatientVIP(patientID))
            {
                if (PlayerController.Instance != null)
                {
                    PlayerController.Instance.TriggerShake(4.0f, 1.5f, 3.0f);
                    if (!PlayerController.Instance.HasTriggeredVipThisLevel)
                    {
                        PlayerController.Instance.TriggerVipOverlay();
                    }
                }
                if (LightingManager.Instance != null) LightingManager.Instance.TriggerSpotlight(4.0f, 3.0f);
                if (ConfettiSpawner.Instance != null) ConfettiSpawner.Instance.TriggerConfetti(6.0f, 3.0f);
                
                if (vipOverlayText != null)
                {
                    vipOverlayTimer = vipOverlayDuration;
                    SetText(vipOverlayText, VIP_OVERLAY_MESSAGE);
                    SetAlpha(vipOverlayText, 1f);
                }
            }

            if (TutorialTextMgr.Instance != null) 
                TutorialTextMgr.Instance.triggar(TutorialTextMgr.promptState.Furthur_Testing);

            SetAlpha(backButton, 1f);
            SetAlpha(tabProfileBtn, 1f);
            SetAlpha(tabResultsBtn, 1f);
            SetAlpha(tabDiagnoseBtn, 1f);
            SetAlpha(tabHighlightOverlay, 1f);

            SwitchTab(Tab.Profile);
        }

        public void ClosePanel()
        {
            IsOpen = false;
            currentPatientID = null;
            GameState.SetUIState(false);

            SetAlpha(backButton, 0f);
            SetAlpha(tabProfileBtn, 0f);
            SetAlpha(tabResultsBtn, 0f);
            SetAlpha(tabDiagnoseBtn, 0f);
            SetAlpha(tabHighlightOverlay, 0f);

            SetProfileAlpha(0f);

            if (PatientTestPanel.Instance != null) PatientTestPanel.Instance.Hide();
            if (DiagnosePanelMgr.Instance != null) DiagnosePanelMgr.Instance.Hide();

            if (ObjectivesMgr.Instance != null) ObjectivesMgr.Instance.Show(true);
        }

        private void SwitchTab(Tab newTab)
        {
            currentTab = newTab;

            ApplyTexture(tabProfileBtn, tabProfileNormal);
            ApplyTexture(tabResultsBtn, tabResultsNormal);
            ApplyTexture(tabDiagnoseBtn, tabDiagnoseNormal);

            Script.Library.Object targetTab = null;
            if (currentTab == Tab.Profile) targetTab = tabProfileBtn;
            else if (currentTab == Tab.Results) targetTab = tabResultsBtn;
            else if (currentTab == Tab.Diagnose) 
            {
                targetTab = tabDiagnoseBtn;
            }
            SnapOverlay(targetTab, tabHighlightOverlay);

            float pAlpha = currentTab == Tab.Profile ? 1f : 0f;
            SetProfileAlpha(pAlpha);
            if (currentTab == Tab.Profile) RefreshProfile();

            if (currentTab == Tab.Results && PatientTestPanel.Instance != null) PatientTestPanel.Instance.Show(currentPatientID);
            else if (PatientTestPanel.Instance != null) PatientTestPanel.Instance.Hide();

            if (currentTab == Tab.Diagnose && DiagnosePanelMgr.Instance != null) DiagnosePanelMgr.Instance.Show(currentPatientID);
            else if (DiagnosePanelMgr.Instance != null) DiagnosePanelMgr.Instance.Hide();
        }

        private void SetProfileAlpha(float pAlpha)
        {
            SetAlpha(profileRoot, pAlpha);
            SetAlpha(profilePatientImage, pAlpha);
            SetAlpha(profileNameText, pAlpha);
            SetAlpha(profileAgeGenderText, pAlpha);
            SetAlpha(profileDiagnosisLabel, pAlpha); 
            SetAlpha(profileDiagnosisText, pAlpha);
            SetAlpha(furtherTestingBtn, pAlpha);
        }

        private void RefreshProfile()
        {
            if (currentPatientID == null) return;
            if (PatientManager.TryGetPatientData(currentPatientID, out PatientData data))
            {
                SetText(profileNameText, data.PatientName);
                SetText(profileAgeGenderText, $"Age {data.Age} | {data.Gender}");
                SetText(profileDiagnosisText, $"\"{data.Symptoms.MainComplaint}\"");

                Image2D face = GetSavedPatientImage(currentPatientID, data);
                ApplyTexture(profilePatientImage, face);
            }
        }

        private void PlayClick() { if (clickSound != null) Audio.playAudio(clickSound); }

        private void UpdateHover(Script.Library.Object btn, Image2D normal, Image2D hover, Vector2D mp, ref Script.Library.Object currentHover)
        {
            if (btn == null) return;
            if (IsOver(btn, mp)) { ApplyTexture(btn, hover); currentHover = btn; }
            else ApplyTexture(btn, normal);
        }

        private void SnapOverlay(Script.Library.Object target, Script.Library.Object overlay)
        {
            if (target == null || overlay == null) return;
            var targetRt = target.getComponent<RectTransformComponent>();
            var overlayRt = overlay.getComponent<RectTransformComponent>();
            if (targetRt != null && overlayRt != null)
            {
                var overlayData = overlayRt.Data;
                overlayData.position.x = targetRt.Data.position.x;
                overlayData.position.y = targetRt.Data.position.y;
                overlayData.scale.x = targetRt.Data.scale.x;
                overlayData.scale.y = targetRt.Data.scale.y;
                overlayRt.Data = overlayData;
            }
        }

        public Image2D GetSavedPatientImage(string patientID)
        {
            if (PatientManager.TryGetPatientData(patientID, out PatientData data)) return GetSavedPatientImage(patientID, data);
            return profileMale1;
        }

        private Image2D GetSavedPatientImage(string patientID, PatientData data)
        {
            if (patientImageCache.ContainsKey(patientID)) return patientImageCache[patientID];
            Image2D generatedImage = profileMale1;
            if (data.meshNumber == 0) generatedImage = profileMale2; 
            else if (data.meshNumber == 1) generatedImage = profileMale1; 
            else if (data.meshNumber == 2) generatedImage = profileFemale;
            else if (data.meshNumber == 3 || data.meshNumber == 4) generatedImage = profilePatient4;
            else if (data.meshNumber == 5) generatedImage = profilePatient7;
            else if (data.meshNumber == 6) generatedImage = profilePatient6SA;
            else if (data.meshNumber == 7) generatedImage = profileVIP;
            patientImageCache[patientID] = generatedImage;
            return generatedImage;
        }

        private void ApplyTexture(Script.Library.Object obj, Image2D tex) { if (obj == null || tex == null) return; var img = obj.getComponent<Image2DComponent>(); if (img != null) { var d = img.Data; d.hashed = tex.hashedID; img.Data = d; } }
        private void SetAlpha(Script.Library.Object obj, float a) { if (obj == null) return; var i = obj.getComponent<Image2DComponent>(); if (i != null) { var d = i.Data; d.color.a = a; i.Data = d; } var t = obj.getComponent<TextMeshUIComponent>(); if (t != null) { var d = t.Data; d.color.a = a; t.Data = d; } }
        private void SetText(Script.Library.Object obj, string text) { if (obj == null) return; var txt = obj.getComponent<TextMeshUIComponent>(); if (txt != null) { var d = txt.Data; d.text = text; txt.Data = d; } }
        private bool IsOver(Script.Library.Object obj, Vector2D mp) { if (obj == null) return false; var rt = obj.getComponent<RectTransformComponent>(); if (rt == null) return false; var d = rt.Data; float hx = d.scale.x * 0.5f; float hy = d.scale.y * 0.5f; Vector2D tl = new Vector2D(d.position.x - hx, d.position.y + hy); Vector2D br = new Vector2D(d.position.x + hx, d.position.y - hy); return Utility.AABB(tl, br, mp); }
        
        public override void LateUpdate(double dt) {}
        public override void Free() { Instance = null; }
    }
}