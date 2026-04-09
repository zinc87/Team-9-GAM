using Script.Library;
using PrivateAPI;
using System.Collections.Generic;
using System;
using System.Threading.Tasks;

namespace Script
{
    public class DiagnosePanelMgr : IScript
    {
        public static DiagnosePanelMgr Instance { get; private set; }

        [SerializeField] public Script.Library.Object diagnoseRoot;
        
        [SerializeField] public Script.Library.Object btnNoIllness;
        [SerializeField] public Script.Library.Object btnMildFlu;
        [SerializeField] public Script.Library.Object btnGastro;
        [SerializeField] public Script.Library.Object btnUti;
        [SerializeField] public Script.Library.Object btnAllergy;
        [SerializeField] public Script.Library.Object btnBlood;
        [SerializeField] public Script.Library.Object btnNeuro;
        [SerializeField] public Script.Library.Object btnDrug;

        [SerializeField] public Image2D imgNoIllnessNormal; 
        [SerializeField] public Image2D imgMildFluNormal;   
        [SerializeField] public Image2D imgGastroNormal;    
        [SerializeField] public Image2D imgUtiNormal;       
        [SerializeField] public Image2D imgAllergyNormal;   
        [SerializeField] public Image2D imgBloodNormal;     
        [SerializeField] public Image2D imgNeuroNormal;     
        [SerializeField] public Image2D imgDrugNormal;      

        [SerializeField] public Script.Library.Object illnessHighlightOverlay; 
        [SerializeField] public Image2D illnessHighlightTexture; 

        [SerializeField] public Script.Library.Object diagnoseConfirmBtn;
        [SerializeField] public Image2D diagnoseConfirmNormal;
        [SerializeField] public Image2D diagnoseConfirmHover;

        [SerializeField] public Script.Library.Object resultPopup;
        [SerializeField] public Image2D successTexture;
        [SerializeField] public Image2D failureTexture;

        [SerializeField] public Script.Library.Object hintButton;

        public static int totalDiagnosed = 0;
        public static int correctlyDiagnosed = 0;
        public static int incorrectlyDiagnosed = 0;
        public static int totalHintsUsed = 0;

        public bool IsVisible { get; private set; } = false;
        private string _patientID = null;
        private string pickedIllness = null;
        private bool isLevel1 = false; 

        private bool hintUsed = false;
        private List<string> hintedIllnesses = new List<string>();
        private const float HINT_COST = 50.0f;

        private float _popupAlpha = 0f;
        private float _popupTimer = 0f;
        private const float POPUP_HOLD = 1.5f;
        private const float POPUP_FADE = 1.0f;

        private AudioInstance hoverSound;
        private AudioInstance clickSound;
        private AudioInstance correctSound;
        private AudioInstance loseSound;
        private AudioInstance hintSound;
        private AudioInstance diagnoseCorrectSound1;
        private AudioInstance diagnoseCorrectSound2;
        private AudioInstance diagnoseWrongSound1;
        private AudioInstance diagnoseWrongSound2;
        private Random _rng = new Random();
        private AudioInstance diagnoseMeSfx1;
        private AudioInstance diagnoseMeSfx2;
        private AudioInstance vipAlert;
        private AudioInstance vipWalkingIn;
        private AudioInstance vipRibbons;

        private float correctSoundTimer = 0f;
        private Script.Library.Object _lastHoveredButton = null;

        private bool _wasMouseDown = false;

        public static void ResetCounters()
        {
            totalDiagnosed = 0;
            correctlyDiagnosed = 0;
            incorrectlyDiagnosed = 0;
            totalHintsUsed = 0;
        }

        public override void Awake() { Instance = this; }

        public override void Start()
        {
            string sceneName = Scene.getSceneName();
            isLevel1 = sceneName.Contains("TempMap");

            ResetCounters();

            hoverSound = Audio.getAudioInstance("HoverButton", Obj);
            clickSound = Audio.getAudioInstance("ButtonPress_003", Obj);
            correctSound = Audio.getAudioInstance("User Interface, Alert, Correct Answer, Positive Bell, Meme Redesign SND140174", Obj);
            loseSound = Audio.getAudioInstance("User Interface, Alert, Notification, Negative, Error, Incorrect, Insufficient, Video Game 04 SND86849", Obj);
            hintSound = Audio.getAudioInstance("HintSFX", Obj);
            diagnoseMeSfx1 = Audio.getAudioInstance("Diagnose me properly", Obj);
            diagnoseMeSfx2 = Audio.getAudioInstance("Doctor wats wrong with me", Obj);

            diagnoseCorrectSound1 = Audio.getAudioInstance("Thanks for taking care of me", Obj);
            diagnoseCorrectSound2 = Audio.getAudioInstance("Thank you I feel better", Obj);
            diagnoseWrongSound1 = Audio.getAudioInstance("Ugh I still feel sick", Obj);
            diagnoseWrongSound2 = Audio.getAudioInstance("I still feel unwell but thanks anyways2", Obj);
            vipAlert = Audio.getAudioInstance("Alert short short", Obj);
            vipWalkingIn = Audio.getAudioInstance("VIP walking in", Obj);
            vipRibbons = Audio.getAudioInstance("when ribbons appear", Obj);

            SetAlpha(resultPopup, 0f);
            Hide();
        }

        public void PlayRandomDiagnoseSfx()
        {
            if (diagnoseMeSfx1 == null)
                diagnoseMeSfx1 = Audio.getAudioInstance("Diagnose me properly", Obj);
            if (diagnoseMeSfx2 == null)
                diagnoseMeSfx2 = Audio.getAudioInstance("Doctor wats wrong with me", Obj);

            var sfx = _rng.Next(2) == 0 ? diagnoseMeSfx1 : diagnoseMeSfx2;
            Logger.log(Logger.LogLevel.Warning, $"Audio Playing");
            Audio.playAudio(sfx);
        }


        private void PlayRandomSfxCorrect()
        {
            var sfx = _rng.Next(2) == 0 ? diagnoseCorrectSound1 : diagnoseCorrectSound2;
            Audio.playAudio(sfx);
        }

        private void PlayRandomSfxWrong()
        {
            var sfx = _rng.Next(2) == 0 ? diagnoseWrongSound1 : diagnoseWrongSound2;
            Audio.playAudio(sfx);
        }

        public async Task PlayVipAudioSequenceAsync()
        {
            Audio.playAudio(vipAlert);
            await Task.Delay(7000);

            //Logger.log(Logger.LogLevel.Warning, "Audio 2 Playing");
            //Audio.playAudio(vipWalkingIn);
            //await Task.Delay(1500);

            
            if(vipRibbons == null)
                Logger.log(Logger.LogLevel.Warning, "Audio 3 NUll");
            Audio.playAudio(vipRibbons);
        }

        public override void Update(double dt)
        {
            if (GameState.IsGameOver) return;

            if (correctSoundTimer > 0f)
            {
                correctSoundTimer -= (float)dt;
                if (correctSoundTimer <= 0f) { if (correctSound != null) Audio.stopAudio(correctSound); }
            }

            if (resultPopup != null && _popupTimer > 0f)
            {
                _popupTimer -= (float)dt;
                if (_popupTimer <= POPUP_FADE) _popupAlpha = _popupTimer / POPUP_FADE;
                ApplyAlpha(resultPopup, _popupAlpha);
                if (_popupTimer <= 0f) { ApplyAlpha(resultPopup, 0f); _popupTimer = 0f; }
            }

            if (!IsVisible) return;
            if (PauseMenu.Instance != null && PauseMenu.Instance.IsPaused) return;

            Vector2D mp;
            Input.getCurMosusePos(out mp);
            
            bool isMouseDown = Input.isMousePressed(Input.Mouse.kMOUSE_BUTTON_LEFT);
            bool clicked = isMouseDown && !_wasMouseDown;
            _wasMouseDown = isMouseDown;

            Script.Library.Object currentHover = null;

            UpdateHover(diagnoseConfirmBtn, diagnoseConfirmNormal, diagnoseConfirmHover, mp, ref currentHover);

            if (IsOver(btnNoIllness, mp)) currentHover = btnNoIllness;
            else if (IsOver(btnMildFlu, mp)) currentHover = btnMildFlu;
            else if (IsOver(btnGastro, mp)) currentHover = btnGastro;
            else if (IsOver(btnUti, mp)) currentHover = btnUti;
            else if (IsOver(hintButton, mp)) currentHover = hintButton;
            
            if (!isLevel1)
            {
                if (btnAllergy != null && IsOver(btnAllergy, mp)) currentHover = btnAllergy;
                else if (btnBlood != null && IsOver(btnBlood, mp)) currentHover = btnBlood;
                else if (btnNeuro != null && IsOver(btnNeuro, mp)) currentHover = btnNeuro;
                else if (btnDrug != null && IsOver(btnDrug, mp)) currentHover = btnDrug;
            }

            MoneyManager moneyCheck = MoneyManager.getInstance();
            bool hintAvailable = !hintUsed && moneyCheck != null && moneyCheck.canAfford(HINT_COST);
            ApplyAlpha(hintButton, hintAvailable ? 1.0f : 0.3f);

            if (clicked)
            {
                bool illnessChanged = false;
                if (IsOver(btnNoIllness, mp)) { pickedIllness = "No Illness"; illnessChanged = true; }
                else if (IsOver(btnMildFlu, mp)) { pickedIllness = "Mild Flu"; illnessChanged = true; }
                else if (IsOver(btnGastro, mp)) { pickedIllness = "Gastroenteritis"; illnessChanged = true; }
                else if (IsOver(btnUti, mp)) { pickedIllness = "Urinary tract infection"; illnessChanged = true; }

                if (!isLevel1)
                {
                    if (btnAllergy != null && IsOver(btnAllergy, mp)) { pickedIllness = "Severe Allergy"; illnessChanged = true; }
                    else if (btnBlood != null && IsOver(btnBlood, mp)) { pickedIllness = "Blood Infection"; illnessChanged = true; }
                    else if (btnNeuro != null && IsOver(btnNeuro, mp)) { pickedIllness = "Toxic-Induced Neurological Distress"; illnessChanged = true; }
                    else if (btnDrug != null && IsOver(btnDrug, mp)) { pickedIllness = "Drug-Induced Cardiac Stress"; illnessChanged = true; }
                }

                if (IsOver(hintButton, mp) && !hintUsed)
                {
                    MoneyManager moneyMgr = MoneyManager.getInstance();
                    if (moneyMgr != null && moneyMgr.canAfford(HINT_COST))
                    {
                        if (PatientManager.TryGetIllness(_patientID, out Illness illness))
                        {
                            List<string> allButtonIllnesses = new List<string> { "Urinary tract infection", "Mild Flu", "No Illness", "Gastroenteritis" };
                            if (!isLevel1)
                            {
                                if (btnAllergy != null) allButtonIllnesses.Add("Severe Allergy");
                                if (btnBlood != null) allButtonIllnesses.Add("Blood Infection");
                                if (btnNeuro != null) allButtonIllnesses.Add("Toxic-Induced Neurological Distress");
                                if (btnDrug != null) allButtonIllnesses.Add("Drug-Induced Cardiac Stress");
                            }

                            var hintList = DiagnosisSystem.GetHintList(illness.Name, allButtonIllnesses, 3);
                            if (hintList.Count > 0)
                            {
                                moneyMgr.deductMoney(HINT_COST);
                                hintUsed = true;
                                totalHintsUsed++;
                                hintedIllnesses = hintList;
                                if (hintSound != null) Audio.playAudio(hintSound);
                                illnessChanged = true;
                            }
                        }
                    }
                }

                if (illnessChanged && clickSound != null) 
                { 
                    Audio.playAudio(clickSound);
                }

                if (IsOver(diagnoseConfirmBtn, mp) && pickedIllness != null)
                {
                    ConfirmDiagnosis();
                    TutorialTextMgr.Instance.triggar(TutorialTextMgr.promptState.End);
                }
            }

            if (currentHover != _lastHoveredButton)
            {
                if (currentHover != null && hoverSound != null) Audio.playAudio(hoverSound);
                _lastHoveredButton = currentHover;
            }

            // FIX: Only force the buttons to refresh and stay active IF the panel is still open!
            // This prevents the buttons from resurrecting themselves a millisecond after ConfirmDiagnosis hides them.
            if (IsVisible)
            {
                RefreshIllnessButtons();
            }
        }

        public void Show(string patientID)
        {
            _patientID = patientID;
            IsVisible = true;
            pickedIllness = null;
            hintUsed = false;
            hintedIllnesses.Clear();

            SetAlpha(diagnoseRoot, 1f);
            SetAlpha(hintButton, 1f);
            SetAlpha(btnNoIllness, 1f);
            SetAlpha(btnMildFlu, 1f);
            SetAlpha(btnGastro, 1f);
            SetAlpha(btnUti, 1f);
            SetAlpha(diagnoseConfirmBtn, 1f);

            if (!isLevel1)
            {
                if (btnAllergy != null) SetAlpha(btnAllergy, 1f);
                if (btnBlood != null) SetAlpha(btnBlood, 1f);
                if (btnNeuro != null) SetAlpha(btnNeuro, 1f);
                if (btnDrug != null) SetAlpha(btnDrug, 1f);
            }

            RefreshIllnessButtons();
        }

        public void Hide()
        {
            IsVisible = false;
            SetAlpha(diagnoseRoot, 0f);
            SetAlpha(hintButton, 0f);
            SetAlpha(btnNoIllness, 0f);
            SetAlpha(btnMildFlu, 0f);
            SetAlpha(btnGastro, 0f);
            SetAlpha(btnUti, 0f);
            SetAlpha(diagnoseConfirmBtn, 0f);

            if (btnAllergy != null) SetAlpha(btnAllergy, 0f);
            if (btnBlood != null) SetAlpha(btnBlood, 0f);
            if (btnNeuro != null) SetAlpha(btnNeuro, 0f);
            if (btnDrug != null) SetAlpha(btnDrug, 0f);
            
            SetAlpha(illnessHighlightOverlay, 0f);
        }

        private bool IsHinted(string illnessName)
        {
            return hintUsed && hintedIllnesses.Contains(illnessName);
        }

        private void RefreshIllnessButtons()
        {
            ApplyTexture(btnNoIllness, imgNoIllnessNormal);
            ApplyTexture(btnMildFlu, imgMildFluNormal);
            ApplyTexture(btnGastro, imgGastroNormal);
            ApplyTexture(btnUti, imgUtiNormal);
            
            if (!isLevel1)
            {
                if (btnAllergy != null) ApplyTexture(btnAllergy, imgAllergyNormal);
                if (btnBlood != null) ApplyTexture(btnBlood, imgBloodNormal);
                if (btnNeuro != null) ApplyTexture(btnNeuro, imgNeuroNormal);
                if (btnDrug != null) ApplyTexture(btnDrug, imgDrugNormal);
            }

            if (hintUsed)
            {
                ApplyAlpha(btnNoIllness, IsHinted("No Illness") ? 1f : 0.3f);
                ApplyAlpha(btnMildFlu, IsHinted("Mild Flu") ? 1f : 0.3f);
                ApplyAlpha(btnGastro, IsHinted("Gastroenteritis") ? 1f : 0.3f);
                ApplyAlpha(btnUti, IsHinted("Urinary tract infection") ? 1f : 0.3f);
                if (!isLevel1)
                {
                    if (btnAllergy != null) ApplyAlpha(btnAllergy, IsHinted("Severe Allergy") ? 1f : 0.3f);
                    if (btnBlood != null) ApplyAlpha(btnBlood, IsHinted("Blood Infection") ? 1f : 0.3f);
                    if (btnNeuro != null) ApplyAlpha(btnNeuro, IsHinted("Toxic-Induced Neurological Distress") ? 1f : 0.3f);
                    if (btnDrug != null) ApplyAlpha(btnDrug, IsHinted("Drug-Induced Cardiac Stress") ? 1f : 0.3f);
                }
            }
            else
            {
                ApplyAlpha(btnNoIllness, 1f); ApplyAlpha(btnMildFlu, 1f); ApplyAlpha(btnGastro, 1f); ApplyAlpha(btnUti, 1f);
                if (!isLevel1)
                {
                    if (btnAllergy != null) ApplyAlpha(btnAllergy, 1f);
                    if (btnBlood != null) ApplyAlpha(btnBlood, 1f);
                    if (btnNeuro != null) ApplyAlpha(btnNeuro, 1f);
                    if (btnDrug != null) ApplyAlpha(btnDrug, 1f);
                }
            }

            Script.Library.Object targetIllnessBtn = null;
            if (pickedIllness == "No Illness") targetIllnessBtn = btnNoIllness;
            else if (pickedIllness == "Mild Flu") targetIllnessBtn = btnMildFlu;
            else if (pickedIllness == "Gastroenteritis") targetIllnessBtn = btnGastro;
            else if (pickedIllness == "Urinary tract infection") targetIllnessBtn = btnUti;
            else if (pickedIllness == "Severe Allergy") targetIllnessBtn = btnAllergy;
            else if (pickedIllness == "Blood Infection") targetIllnessBtn = btnBlood;
            else if (pickedIllness == "Toxic-Induced Neurological Distress") targetIllnessBtn = btnNeuro;
            else if (pickedIllness == "Drug-Induced Cardiac Stress") targetIllnessBtn = btnDrug;

            if (targetIllnessBtn != null)
            {
                ApplyTexture(illnessHighlightOverlay, illnessHighlightTexture);
                SnapOverlay(targetIllnessBtn, illnessHighlightOverlay);
                SetAlpha(illnessHighlightOverlay, 1.0f);
            }
            else
            {
                SetAlpha(illnessHighlightOverlay, 0.0f);
            }
        }

        private void ConfirmDiagnosis()
        {
            if (PatientManager.TryGetIllness(_patientID, out Illness data))
            {
                bool isVIP = PatientManager.IsPatientVIP(_patientID);

                if (pickedIllness == data.Name)
                {
                    MoneyManager.getInstance().addMoney(100.0f);
                    correctlyDiagnosed++;
                    LevelScoreManager.RecordDiagnosis(true, isVIP);
                    if (correctSound != null) { Audio.playAudio(correctSound); correctSoundTimer = 1.0f; }
                    PlayRandomSfxCorrect();

                    ApplyTexture(resultPopup, successTexture);
                    _popupAlpha = 1.0f;
                    _popupTimer = POPUP_HOLD + POPUP_FADE;
                    ApplyAlpha(resultPopup, 1f);
                }
                else
                {
                    MoneyManager.getInstance().deductMoney(100.0f);
                    incorrectlyDiagnosed++;
                    LevelScoreManager.RecordDiagnosis(false, isVIP);
                    if (PenaltyBarMgr.Instance != null) PenaltyBarMgr.Instance.RecordWrongDiagnosis();
                    if (loseSound != null) Audio.playAudio(loseSound);
                    PlayRandomSfxWrong();

                    ApplyTexture(resultPopup, failureTexture);
                    _popupAlpha = 1.0f;
                    _popupTimer = POPUP_HOLD + POPUP_FADE;
                    ApplyAlpha(resultPopup, 1f);
                }
                
                totalDiagnosed++;

                foreach (var c in WaypointPatientControllerManager.GetAllControllers()) {
                    if (c.ObjectId == _patientID) { c.MoveToExit(); break; }
                }
                
                PatientManager.RemovePatient(_patientID);
                WaypointPatientControllerManager.AdvanceAllQueue();
                
                if (PatientPanelMgr.Instance != null)
                    PatientPanelMgr.Instance.ClosePanel();
            }
        }

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

        private void ApplyTexture(Script.Library.Object obj, Image2D tex) 
        { 
            if (obj == null || tex == null) return; 
            var img = obj.getComponent<Image2DComponent>(); 
            if (img != null) { 
                var d = img.Data; 
                d.hashed = tex.hashedID; 
                img.Data = d; 
            } 
        }

        private void ApplyAlpha(Script.Library.Object obj, float a) 
        { 
            if (obj == null) return; 
            var i = obj.getComponent<Image2DComponent>(); 
            if (i != null) { 
                var d = i.Data; 
                d.color.a = a; 
                i.Data = d; 
            } 
            var t = obj.getComponent<TextMeshUIComponent>(); 
            if (t != null) { 
                var d = t.Data; 
                d.color.a = a; 
                t.Data = d; 
            } 
        }

        public void ForceHideResultPopup()
        {
            _popupTimer = 0f;
            _popupAlpha = 0f;
            if (resultPopup != null) SetAlpha(resultPopup, 0f);
        }

        private void SetAlpha(Script.Library.Object obj, float a) { ApplyAlpha(obj, a); }
        
        private bool IsOver(Script.Library.Object obj, Vector2D mp) { if (obj == null) return false; var rt = obj.getComponent<RectTransformComponent>(); if (rt == null) return false; var d = rt.Data; float hx = d.scale.x * 0.5f; float hy = d.scale.y * 0.5f; Vector2D tl = new Vector2D(d.position.x - hx, d.position.y + hy); Vector2D br = new Vector2D(d.position.x + hx, d.position.y - hy); return Utility.AABB(tl, br, mp); }
        
        public override void LateUpdate(double dt) {}
        public override void Free() { Instance = null; }
    }
}