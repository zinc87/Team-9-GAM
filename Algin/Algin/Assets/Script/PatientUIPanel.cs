using Script.Library;
using System.Collections.Generic;

namespace Script
{
    public class PatientUIPanel : IScript
    {
        public PatientUIPanel() : base("") { }
        public PatientUIPanel(string id) : base(id) { }

        private enum State { Closed, List, Details }
        private State state = State.Closed;
        private bool pWasDown = false;
        public bool IsOpen { get; private set; }
        private bool wasBlocked = false;

        // ======================
        // UI OBJECTS
        // ======================
        [SerializeField] public Image2D patientProfileNormal;
        [SerializeField] public Image2D patientProfileHover;

        [SerializeField] public Object listPanel;
        [SerializeField] public Object patientButton1;
        [SerializeField] public Object patientButton2;
        [SerializeField] public Object patientButton3;
        [SerializeField] public Object patientButton4;
        [SerializeField] public Object patientButton5;
        [SerializeField] public Object patientButton6;
        [SerializeField] public Object patientButton7;
        [SerializeField] public Object patientButton8;

        [SerializeField] public Object closeButton;
        [SerializeField] public Image2D closeButtonNormal;
        [SerializeField] public Image2D closeButtonHover;

        [SerializeField] public Object confirmButton;
        [SerializeField] public Image2D confirmButtonNormal;
        [SerializeField] public Image2D confirmButtonHover;

        // DETAILS PANEL
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

        // VIP Overlay
        [SerializeField] public Object vipOverlayText;
        [SerializeField] public float vipOverlayDuration = 2.5f;

        // Camera Overlay (Corridor POV only)
        [SerializeField] public Object corridorCameraObj;
        [SerializeField] public Object cameraOverlayText;
        [SerializeField] public float cameraOverlayDuration = 3.0f;

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

        private Object[] patientButtons;
        private string selectedPatientID = null;
        public static int selectedPatientNo = 0;

        // Internal Memory Cache for faces (NEW)
        private Dictionary<string, Image2D> patientImageCache = new Dictionary<string, Image2D>();
        private System.Random rand = new System.Random();

        private AudioInstance hoverSound;
        private AudioInstance clickSound;
        private Object _lastHoveredButton = null;
        private AudioInstance openPanelSound;
        private const string VIP_OVERLAY_MESSAGE = "VIP is Here!";
        private const string CAMERA_OVERLAY_MESSAGE = "Patient Coming";
        private float vipOverlayTimer = 0f;

        private const float CAMERA_OVERLAY_VIEWPORT_X = 0.0f;
        private const float CAMERA_OVERLAY_VIEWPORT_Y = 0.0f;
        private const float CAMERA_OVERLAY_VIEWPORT_W = 0.3f;
        private const float CAMERA_OVERLAY_VIEWPORT_H = 0.3f;

        private bool _cameraOverlayActive = false;
        private float _cameraOverlayTimer = 0f;
        private HashSet<string> _overlayTriggeredPatients = new HashSet<string>();

        public static PatientUIPanel Instance { get; private set; }

        public override void Awake()
        {
            Instance = this;
        }

        public override void Start()
        {
            patientButtons = new Object[] {
                patientButton1, patientButton2, patientButton3, patientButton4,
                patientButton5, patientButton6, patientButton7, patientButton8
            };
            if (vipOverlayText != null)
            {
                SetText(vipOverlayText, VIP_OVERLAY_MESSAGE);
                SetAlpha(vipOverlayText, 0f);
            }
            if (cameraOverlayText != null)
            {
                SetText(cameraOverlayText, CAMERA_OVERLAY_MESSAGE);
                SetAlpha(cameraOverlayText, 0f);
            }
            CloseAll();

            hoverSound = Audio.getAudioInstance("HoverButton", Obj);
            clickSound = Audio.getAudioInstance("ButtonPress_003", Obj);
            openPanelSound = Audio.getAudioInstance("Computer_Typing_Button_7", Obj);

            if (corridorCameraObj != null)
            {
                CameraOverlay.SetCamera(corridorCameraObj);
                CameraOverlay.SetViewport(CAMERA_OVERLAY_VIEWPORT_X, CAMERA_OVERLAY_VIEWPORT_Y, CAMERA_OVERLAY_VIEWPORT_W, CAMERA_OVERLAY_VIEWPORT_H);
                CameraOverlay.SetActive(false);
            }
        }

        public override void Update(double dt)
        {
            UpdateVipOverlay(dt);
            UpdateCameraOverlay(dt);
            if (GameState.IsGameOver) return;

            bool isBlocked = (DiagnosePanelMgr.Instance != null && DiagnosePanelMgr.Instance.IsVisible) ||
                             (PatientTestPanel.Instance != null && PatientTestPanel.Instance.IsVisible);

            if (isBlocked)
            {
                wasBlocked = true;
                return;
            }

            if (wasBlocked)
            {
                if (Input.isMouseClicked(Input.Mouse.kMOUSE_BUTTON_LEFT)) return;
                wasBlocked = false;
            }

            bool pDown = Input.isKeyPressed(Input.KeyCode.kP);
            if (pDown && !pWasDown)
            {
                if (state == State.Closed)
                {
                    OpenList();
                }
                else
                {
                    CloseAll();
                }
            }
            pWasDown = pDown;

            if (state == State.Closed) return;

            Vector2D mp;
            Input.getCurMosusePos(out mp);

            // HOVER LOGIC
            Object _currentHovered = null;
            if (state == State.List)
            {
                UpdateHover(closeButton, closeButtonNormal, closeButtonHover, mp);
                if (IsMouseOver(closeButton, mp)) _currentHovered = closeButton;
                UpdateHover(confirmButton, confirmButtonNormal, confirmButtonHover, mp);
                if (IsMouseOver(confirmButton, mp)) _currentHovered = confirmButton;
            }
            else if (state == State.Details)
            {
                UpdateHover(detailsTestResultsButton, detailsTestResultsNormal, detailsTestResultsHover, mp);
                if (IsMouseOver(detailsTestResultsButton, mp)) _currentHovered = detailsTestResultsButton;
                UpdateHover(detailsFurtherTestingButton, detailsFurtherTestingNormal, detailsFurtherTestingHover, mp);
                if (IsMouseOver(detailsFurtherTestingButton, mp)) _currentHovered = detailsFurtherTestingButton;
                UpdateHover(detailsDiagnoseButton, detailsDiagnoseNormal, detailsDiagnoseHover, mp);
                if (IsMouseOver(detailsDiagnoseButton, mp)) _currentHovered = detailsDiagnoseButton;
                UpdateHover(detailsCloseButton, detailsCloseNormal, detailsCloseHover, mp);
                if (IsMouseOver(detailsCloseButton, mp)) _currentHovered = detailsCloseButton;
                UpdateHover(detailsBackButton, detailsBackNormal, detailsBackHover, mp);
                if (IsMouseOver(detailsBackButton, mp)) _currentHovered = detailsBackButton;
            }

            if (_currentHovered != _lastHoveredButton)
            {
                if (_currentHovered != null && hoverSound != null)
                    Audio.playAudio(hoverSound);
                _lastHoveredButton = _currentHovered;
            }

            if (state == State.List)
            {
                UpdateList(mp);
            }
            else if (state == State.Details)
            {
                UpdateDetails(mp);
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

        private void UpdateList(Vector2D mp)
        {
            if (patientButtons != null)
            {
                for (int i = 0; i < patientButtons.Length; i++)
                {
                    Object btn = patientButtons[i];
                    if (btn == null) continue;

                    string pid = GetPatientIDByIndex(i);
                    if (pid != null)
                    {
                        // Apply the customized face instead of the generic hover profile
                        Image2D face = GetSavedPatientImage(pid);
                        ApplyTexture(btn, face);
                    }
                    else
                    {
                        // For empty slots, keep the generic hover logic
                        if (IsMouseOver(btn, mp))
                        {
                            ApplyTexture(btn, patientProfileHover);
                        }
                        else
                        {
                            ApplyTexture(btn, patientProfileNormal);
                        }
                    }
                }
            }

            // Click handling
            if (Input.isMouseClicked(Input.Mouse.kMOUSE_BUTTON_LEFT))
            {
                if (clickSound != null) Audio.playAudio(clickSound);

                if (IsMouseOver(closeButton, mp) || IsMouseOver(confirmButton, mp))
                {
                    CloseAll();
                    return;
                }

                for (int i = 0; i < patientButtons.Length; i++)
                {
                    Object btn = patientButtons[i];
                    if (btn != null && IsMouseOver(btn, mp))
                    {
                        string overlayKey = null;
                        if (WaypointPatientSpawner._allPatients[i] != null)
                        {
                            WaypointPatientSpawner._allPatients[i].MoveToCounter();
                            overlayKey = WaypointPatientSpawner._allPatients[i].ObjectId;
                        }
                        selectedPatientNo = i;

                        string pid = GetPatientIDByIndex(i);
                        if (pid != null)
                        {
                            OpenDetails(pid, btn);
                            TutorialTextMgr.Instance.triggar(TutorialTextMgr.promptState.Furthur_Testing);
                            if (overlayKey == null) overlayKey = pid;
                            if (overlayKey != null && !_overlayTriggeredPatients.Contains(overlayKey))
                            {
                                StartCameraOverlay();
                                _overlayTriggeredPatients.Add(overlayKey);
                            }
                        }

                        // Check if this is the VIP patient and trigger camera shake if so
                        if (PatientManager.IsPatientVIP(pid))
                        {
                            if (PlayerController.Instance != null)
                            {
                                Logger.log(Logger.LogLevel.Info, "[PatientUIPanel] VIP clicked, triggering camera shake in 3 seconds!");
                                // 4 seconds of shaking, 1.5 magnitude, starting after a 3 second delay
                                PlayerController.Instance.TriggerShake(4.0f, 1.5f, 3.0f);
                            }

                            if (LightingManager.Instance != null)
                                LightingManager.Instance.TriggerSpotlight(4.0f, 3.0f);

                            if (ConfettiSpawner.Instance != null)
                            {
                                ConfettiSpawner.Instance.TriggerConfetti(6.0f, 3.0f);
                            }
                            ShowVipOverlay();
                            if (PlayerController.Instance != null && !PlayerController.Instance.HasTriggeredVipThisLevel)
                            {
                                PlayerController.Instance.TriggerVipOverlay();
                            }
                        }

                        // Debug Logs (guarded)
                        if (PatientManager.TryGetIllness(pid, out Illness illness) && illness != null)
                        {
                            Logger.log(Logger.LogLevel.Warning, $"Assigned Illness: {illness.Name}");
                        }
                        if (PatientManager.TryGetPatientData(pid, out PatientData Pdata))
                        {
                            Logger.log(Logger.LogLevel.Warning, $"Patient Name: {Pdata.PatientName}");
                            Logger.log(Logger.LogLevel.Warning, $"Lungs Scan: {Pdata.Scans.Lungs}");
                            Logger.log(Logger.LogLevel.Warning, $"Heart Scan: {Pdata.Scans.Heart}");
                            Logger.log(Logger.LogLevel.Warning, $"Digestive Scan: {Pdata.Scans.Digestive}");
                            Logger.log(Logger.LogLevel.Warning, $"Urine Results Test: {Pdata.Labs.Urine}");
                        }
                        
                    }
                }
            }
        }

        private void UpdateDetails(Vector2D mp)
        {
            if (Input.isMouseClicked(Input.Mouse.kMOUSE_BUTTON_LEFT))
            {
                if (IsMouseOver(detailsBackButton, mp))
                {
                    OpenList();
                    return;
                }
                if (IsMouseOver(detailsCloseButton, mp))
                {
                    CloseAll();
                    return;
                }

                if (IsMouseOver(detailsTestResultsButton, mp))
                {
                    if (PatientTestPanel.Instance != null)
                    {
                        PatientTestPanel.Instance.Show(selectedPatientID);
                        HideDetailsPanel();
                        wasBlocked = true;
                    }
                    return;
                }

                if (IsMouseOver(detailsFurtherTestingButton, mp))
                {
                    Logger.log(Logger.LogLevel.Warning, "Hit");
                    if (WaypointPatientSpawner._allPatients[selectedPatientNo] != null)
                    {
                        WaypointPatientSpawner._allPatients[selectedPatientNo].MoveToTesting();
                        TutorialTextMgr.Instance.triggar(TutorialTextMgr.promptState.Scan_Patient);
                    }
                    return;
                }

                if (IsMouseOver(detailsDiagnoseButton, mp))
                {
                    if (DiagnosePanelMgr.Instance != null)
                    {
                        DiagnosePanelMgr.Instance.ShowDiagnosePanel(selectedPatientID);
                        HideDetailsPanel();
                        wasBlocked = true;
                        TutorialTextMgr.Instance.triggar(TutorialTextMgr.promptState.End);
                    }
                    return;
                }
            }
        }

        // ======================
        // HELPERS
        // ======================
        private void ShowVipOverlay()
        {
            if (vipOverlayText == null) return;

            vipOverlayTimer = vipOverlayDuration;
            SetText(vipOverlayText, VIP_OVERLAY_MESSAGE);
            SetAlpha(vipOverlayText, 1f);
        }

        private void UpdateVipOverlay(double dt)
        {
            if (vipOverlayText == null) return;

            if (vipOverlayTimer > 0f)
            {
                vipOverlayTimer -= (float)dt;
                if (vipOverlayTimer <= 0f)
                {
                    vipOverlayTimer = 0f;
                    SetAlpha(vipOverlayText, 0f);
                }
            }
        }

        private void StartCameraOverlay()
        {
            if (corridorCameraObj == null) return;
            if (cameraOverlayDuration <= 0f) return;

            CameraOverlay.SetCamera(corridorCameraObj);
            CameraOverlay.SetViewport(CAMERA_OVERLAY_VIEWPORT_X, CAMERA_OVERLAY_VIEWPORT_Y, CAMERA_OVERLAY_VIEWPORT_W, CAMERA_OVERLAY_VIEWPORT_H);
            CameraOverlay.SetActive(true);

            _cameraOverlayActive = true;
            _cameraOverlayTimer = 0f;
            if (cameraOverlayText != null)
            {
                SetText(cameraOverlayText, CAMERA_OVERLAY_MESSAGE);
                SetAlpha(cameraOverlayText, 1f);
            }
        }

        private void UpdateCameraOverlay(double dt)
        {
            if (!_cameraOverlayActive) return;

            _cameraOverlayTimer += (float)dt;
            if (_cameraOverlayTimer >= cameraOverlayDuration)
            {
                StopCameraOverlay();
            }
        }

        private void StopCameraOverlay()
        {
            _cameraOverlayActive = false;
            _cameraOverlayTimer = 0f;
            CameraOverlay.SetActive(false);
            SetAlpha(cameraOverlayText, 0f);
        }

        public void OpenList()
        {
            state = State.List;
            GameState.SetUIState(true);
            IsOpen = true;

            if (openPanelSound != null)
                Audio.playAudio(openPanelSound);

            SetAlpha(listPanel, 1f);
            SetArrayAlpha(patientButtons, 1f);
            SetAlpha(closeButton, 1f);
            SetAlpha(confirmButton, 1f);

            HideDetailsPanel();

            if (PatientTestPanel.Instance != null)
            {
                PatientTestPanel.Instance.Hide();
            }

            RefreshSlotVisuals();

            if (ObjectivesMgr.Instance != null)
            {
                ObjectivesMgr.Instance.Show(false);
            }
        }

        public void CloseAll()
        {
            state = State.Closed;
            selectedPatientID = null;
            GameState.SetUIState(false);
            IsOpen = false;

            SetAlpha(listPanel, 0f);
            SetArrayAlpha(patientButtons, 0f);
            SetAlpha(closeButton, 0f);
            SetAlpha(confirmButton, 0f);

            HideDetailsPanel();

            if (ObjectivesMgr.Instance != null)
            {
                ObjectivesMgr.Instance.Show(true);
            }
            if (DiagnosePanelMgr.Instance != null)
            {
                DiagnosePanelMgr.Instance.SetVisible(false);
            }
        }

        public void OpenDetailsFor(string patientID)
        {
            OpenDetails(patientID, null);
        }

        private void OpenDetails(string patientID, Object clickedBtn)
        {
            selectedPatientID = patientID;
            state = State.Details;
            GameState.SetUIState(true);
            IsOpen = true;

            // Hide List Elements
            SetAlpha(listPanel, 0f);
            SetArrayAlpha(patientButtons, 0f);
            SetAlpha(closeButton, 0f);
            SetAlpha(confirmButton, 0f);

            if (PatientTestPanel.Instance != null)
            {
                PatientTestPanel.Instance.Hide();
            }

            // Sync Face Image
            Image2D masterFace = GetSavedPatientImage(patientID);
            ApplyTexture(detailsPatientImage, masterFace);

            // Show Details Elements
            SetAlpha(detailsPanelRoot, 1f);
            SetAlpha(detailsPatientImage, 1f);
            SetAlpha(detailsNameText, 1f);
            SetAlpha(detailsAgeText, 1f);
            SetAlpha(detailsGenderText, 1f);
            SetAlpha(detailsDiagnosisText, 1f);
            SetAlpha(detailsTestResultsButton, 1f);
            SetAlpha(detailsFurtherTestingButton, 1f);
            SetAlpha(detailsDiagnoseButton, 1f);
            SetAlpha(detailsCloseButton, 1f);
            SetAlpha(detailsBackButton, 1f);

            RefreshDetailsText();
        }

        private void HideDetailsPanel()
        {
            StopCameraOverlay();
            SetAlpha(detailsPanelRoot, 0f);
            SetAlpha(detailsPatientImage, 0f);
            SetAlpha(detailsNameText, 0f);
            SetAlpha(detailsAgeText, 0f);
            SetAlpha(detailsGenderText, 0f);
            SetAlpha(detailsDiagnosisText, 0f);
            SetAlpha(detailsTestResultsButton, 0f);
            SetAlpha(detailsFurtherTestingButton, 0f);
            SetAlpha(detailsDiagnoseButton, 0f);
            SetAlpha(detailsCloseButton, 0f);
            SetAlpha(detailsBackButton, 0f);
        }

        private void RefreshDetailsText()
        {
            if (selectedPatientID == null) return;

            if (PatientManager.TryGetPatientData(selectedPatientID, out PatientData data))
            {
                SetText(detailsNameText, data.PatientName);
                SetText(detailsAgeText, data.Age.ToString());
                SetText(detailsGenderText, data.Gender.ToString());
                SetText(detailsDiagnosisText, "\"" + data.Symptoms.MainComplaint + "\"");
            }
        }

        private void RefreshSlotVisuals()
        {
            if (patientButtons == null) return;

            for (int i = 0; i < patientButtons.Length; i++)
            {
                Object btn = patientButtons[i];
                if (btn == null) continue;

                string pid = GetPatientIDByIndex(i);
                var img = btn.getComponent<Image2DComponent>();

                if (img != null)
                {
                    var d = img.Data;
                    if (pid != null)
                    {
                        // Occupied Slot: Reset to full brightness and opacity
                        d.color.r = 1f;
                        d.color.g = 1f;
                        d.color.b = 1f;
                        d.color.a = 1f;

                        // Apply the unique patient face
                        Image2D face = GetSavedPatientImage(pid);
                        if (face != null)
                        {
                            d.hashed = face.hashedID;
                        }
                    }
                    else
                    {
                        // Empty Slot: Hide the image completely instead of using the old image.
                        // This removes the dark grey box with the generic white icon.
                        d.color.a = 0f;
                    }
                    img.Data = d;
                }
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

        private string GetPatientIDByIndex(int index)
        {
            var all = PatientManager.GetAllPatientObjectIds();
            if (all != null && index < all.Count)
            {
                return all[index];
            }
            return null;
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

        private void SetArrayAlpha(Object[] arr, float alpha)
        {
            if (arr == null) return;

            foreach (var o in arr)
            {
                SetAlpha(o, alpha);
            }
        }

        private void SetAlpha(Object obj, float alpha)
        {
            if (obj == null) return;

            var img = obj.getComponent<Image2DComponent>();
            if (img != null)
            {
                var d = img.Data;
                d.color.a = alpha;
                img.Data = d;
            }

            var txt = obj.getComponent<TextMeshUIComponent>();
            if (txt != null)
            {
                var d = txt.Data;
                d.color.a = alpha;
                txt.Data = d;
            }
        }

        private void SetText(Object obj, string text)
        {
            if (obj == null) return;

            var txt = obj.getComponent<TextMeshUIComponent>();
            if (txt != null)
            {
                var d = txt.Data;
                d.text = text;
                txt.Data = d;
            }
        }

        private void CopyImageTexture(Object s, Object t)
        {
            if (s == null || t == null) return;

            var sc = s.getComponent<Image2DComponent>();
            var tc = t.getComponent<Image2DComponent>();

            if (sc != null && tc != null)
            {
                var d = tc.Data;
                d.hashed = sc.Data.hashed;
                tc.Data = d;
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

        public override void LateUpdate(double dt)
        {
        }

        public override void Free()
        {
            Instance = null;
        }
    }
}
