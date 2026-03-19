using Script.Library;
using System.Collections.Generic;

namespace Script
{
    public class DiagnosePanelMgr : IScript
    {
        [SerializeField] public Image2D BGImage;
        [SerializeField] public Image2D textImage;
        [SerializeField] public Image2D panelImage;
        [SerializeField] public Image2D patientImage; // Default/Fallback image

        // --- NORMAL TEXTURES (Illnesses) ---
        [SerializeField] public Image2D mildflu;
        [SerializeField] public Image2D noIllness;
        [SerializeField] public Image2D gastro;
        [SerializeField] public Image2D uti;
        [SerializeField] public Image2D bloodinfection;
        [SerializeField] public Image2D severeallergy;
        [SerializeField] public Image2D neurological;
        [SerializeField] public Image2D druginduced;

        // --- SELECTED TEXTURES ---
        [SerializeField] public Image2D mildfluSelected;
        [SerializeField] public Image2D noIllnessSelected;
        [SerializeField] public Image2D gastroSelected;
        [SerializeField] public Image2D utiSelected;
        [SerializeField] public Image2D bloodinfectionSelected;
        [SerializeField] public Image2D severeallergySelected;
        [SerializeField] public Image2D neurologicalSelected;
        [SerializeField] public Image2D druginducedSelected;

        // --- NORMAL TEXTURES (Buttons) ---
        [SerializeField] public Image2D diagnose;
        [SerializeField] public Image2D back;

        // --- HOVER TEXTURES (Buttons) ---
        [SerializeField] public Image2D diagnoseHover;
        [SerializeField] public Image2D backHover;

        // --- HINT BUTTON ---
        [SerializeField] public Object hintButton;
        // --- HOVER TEXTURES (Illness Buttons) ---
        [SerializeField] public Image2D mildfluHover;
        [SerializeField] public Image2D noIllnessHover;
        [SerializeField] public Image2D gastroHover;
        [SerializeField] public Image2D utiHover;
        [SerializeField] public Image2D bloodinfectionHover;
        [SerializeField] public Image2D severeallergyHover;
        [SerializeField] public Image2D neurologicalHover;
        [SerializeField] public Image2D druginducedHover;

        // --- UI OBJECTS ---
        [SerializeField] public Object diagnoseBG;
        [SerializeField] public Object diagnoseText;
        [SerializeField] public Object diagnosePanel;
        [SerializeField] public Object patientIcon;

        [SerializeField] public Object diagnoseButton;
        [SerializeField] public Object backButton;

        [SerializeField] public Object mildfluButton;
        [SerializeField] public Object noIllnessButton;
        [SerializeField] public Object gastroButton;
        [SerializeField] public Object utiButton;
        [SerializeField] public Object bloodinfectionButton;
        [SerializeField] public Object severeallergyButton;
        [SerializeField] public Object neurologicalButton;
        [SerializeField] public Object druginducedButton;

        [SerializeField] public Object patientNameText;
        [SerializeField] public Object patientAgeText;
        [SerializeField] public Object patientGenderText;
        [SerializeField] public Object patientSymptomsText;

        private bool visible = false;
        public bool IsVisible { get { return visible; } }

        private string currentStudent = null;
        public static string pickedIllness = null;

        // --- HINT STATE ---
        private bool hintUsed = false;
        private List<string> hintedIllnesses = new List<string>();
        private const float HINT_COST = 50.0f;

        // Diagnosis tracking for Level Summary
        public static int totalDiagnosed = 0;
        public static int correctlyDiagnosed = 0;
        public static int incorrectlyDiagnosed = 0;
        public static int totalHintsUsed = 0;

        private float correctSoundTimer = 0f;

        public static void ResetCounters()
        {
            totalDiagnosed = 0;
            correctlyDiagnosed = 0;
            incorrectlyDiagnosed = 0;
            totalHintsUsed = 0;
        }

        private AudioInstance correctSound;
        private AudioInstance loseSound;
        private AudioInstance hintSound;

        private AudioInstance hoverSound;
        private AudioInstance clickSound;

        private Object _lastHoveredButton = null;

        public static DiagnosePanelMgr Instance { get; private set; }

        public override void Awake()
        {
            Instance = this;
            visible = false;
            pickedIllness = null; // Clear stale state
            GameState.SetUIState(false);
            ResetCounters();
        }

        public override void Start()
        {
            ApplyTexture(diagnoseBG, BGImage);
            ApplyTexture(diagnoseText, textImage);
            ApplyTexture(diagnosePanel, panelImage);
            ApplyTexture(patientIcon, patientImage); // Sets default initially
            ApplyTexture(mildfluButton, mildflu);
            ApplyTexture(noIllnessButton, noIllness);
            ApplyTexture(gastroButton, gastro);
            ApplyTexture(utiButton, uti);
            if (bloodinfectionButton != null && bloodinfection != null) ApplyTexture(bloodinfectionButton, bloodinfection);
            if (severeallergyButton != null && severeallergy != null) ApplyTexture(severeallergyButton, severeallergy);
            if (neurologicalButton != null && neurological != null) ApplyTexture(neurologicalButton, neurological);
            if (druginducedButton != null && druginduced != null) ApplyTexture(druginducedButton, druginduced);

            SetVisible(false);
            UpdateButtonTextures();
            // Initialize audio instances
            correctSound = Audio.getAudioInstance("User Interface, Alert, Correct Answer, Positive Bell, Meme Redesign SND140174", Obj);
            loseSound = Audio.getAudioInstance("User Interface, Alert, Notification, Negative, Error, Incorrect, Insufficient, Video Game 04 SND86849", Obj);
            hoverSound = Audio.getAudioInstance("HoverButton", Obj);
            clickSound = Audio.getAudioInstance("ButtonPress_003", Obj);
            hintSound = Audio.getAudioInstance("HintSFX", Obj);
            // Debug logging for audio
            if (correctSound == null)
                Logger.log(Logger.LogLevel.Error, "[GameStateManager] winSound (CorrectSFX) is NULL!");
            else
                Logger.log(Logger.LogLevel.Info, "[GameStateManager] winSound (CorrectSFX) initialized.");

            if (loseSound == null)
                Logger.log(Logger.LogLevel.Error, "[GameStateManager] loseSound (WrongSFX) is NULL!");
            else
                Logger.log(Logger.LogLevel.Info, "[GameStateManager] loseSound (WrongSFX) initialized.");
        }

        public void ShowDiagnosePanel(string student)
        {
            currentStudent = student;
            pickedIllness = null;
            hintUsed = false;
            hintedIllnesses.Clear();
            SetVisible(true);
            UpdateButtonTextures();
            RefreshPatientInfo();
        }

        public void SetVisible(bool isVisible)
        {
            visible = isVisible;
            GameState.SetUIState(visible);

            float a = visible ? 1.0f : 0.0f;
            ApplyAlpha(diagnoseBG, a); ApplyAlpha(diagnoseText, a); ApplyAlpha(diagnosePanel, a); ApplyAlpha(patientIcon, a);
            ApplyAlpha(hintButton, a);
            ApplyAlpha(mildfluButton, a);
            ApplyAlpha(noIllnessButton, a);
            ApplyAlpha(gastroButton, a);
            ApplyAlpha(utiButton, a);
            if (bloodinfectionButton != null) ApplyAlpha(bloodinfectionButton, a);
            if (severeallergyButton != null) ApplyAlpha(severeallergyButton, a);
            if (neurologicalButton != null) ApplyAlpha(neurologicalButton, a);
            if (druginducedButton != null) ApplyAlpha(druginducedButton, a);
            ApplyAlpha(diagnoseButton, a); ApplyAlpha(backButton, a);
            ApplyAlpha(patientNameText, a); ApplyAlpha(patientAgeText, a); ApplyAlpha(patientGenderText, a); 
            ApplyAlpha(patientSymptomsText, a);

            if (ObjectivesMgr.Instance != null) ObjectivesMgr.Instance.Show(!visible);
        }

        private void RefreshPatientInfo()
        {
            if (currentStudent == null) 
            { 
                SetText(patientNameText, ""); 
                SetText(patientAgeText, ""); 
                SetText(patientGenderText, ""); 
                SetText(patientSymptomsText, ""); 
                return; 
            }

            if (PatientManager.TryGetPatientData(currentStudent, out PatientData data)) 
            { 
                SetText(patientNameText, data.PatientName); 
                SetText(patientAgeText, data.Age.ToString()); 
                SetText(patientGenderText, data.Gender.ToString()); 
                SetText(patientSymptomsText, "\"" + data.Symptoms.MainComplaint + "\""); 

                // Match the assigned icon from the Profile Panel ---
                if (PatientUIPanel.Instance != null)
                {
                    // Ask PatientPanelMgr for the EXACT face assigned to this ID
                    Image2D matchedFace = PatientUIPanel.Instance.GetSavedPatientImage(currentStudent);
                    if (matchedFace != null)
                    {
                        ApplyTexture(patientIcon, matchedFace);
                    }
                }
                // -----------------------------------------------------------------
            }
            else 
            { 
                SetText(patientNameText, "Error Loading Patient"); 
            }
        }

        public override void Update(double dt)
        {
            if (correctSoundTimer > 0f)
            {
                correctSoundTimer -= (float)dt;
                if (correctSoundTimer <= 0f)
                {
                    correctSoundTimer = 0f;
                    if (correctSound != null) Audio.stopAudio(correctSound);
                }
            }
            
            if (!visible) return;

            Vector2D mp;
            Input.getCurMosusePos(out mp);

            // HOVER
            Object _currentHovered = null;

            if (IsOver(diagnoseButton, mp)) { ApplyTexture(diagnoseButton, diagnoseHover); _currentHovered = diagnoseButton; }
            else { ApplyTexture(diagnoseButton, diagnose); }

            if (IsOver(backButton, mp)) { ApplyTexture(backButton, backHover); _currentHovered = backButton; }
            else { ApplyTexture(backButton, back); }

            // Illness buttons hover tracking
            if (IsOver(mildfluButton, mp)) _currentHovered = mildfluButton;
            else if (IsOver(noIllnessButton, mp)) _currentHovered = noIllnessButton;
            else if (IsOver(gastroButton, mp)) _currentHovered = gastroButton;
            else if (IsOver(utiButton, mp)) _currentHovered = utiButton;
            else if (IsOver(bloodinfectionButton, mp)) _currentHovered = bloodinfectionButton;
            else if (IsOver(severeallergyButton, mp)) _currentHovered = severeallergyButton;
            else if (IsOver(neurologicalButton, mp)) _currentHovered = neurologicalButton;
            else if (IsOver(druginducedButton, mp)) _currentHovered = druginducedButton;
            else if (IsOver(hintButton, mp)) _currentHovered = hintButton;

            // Play hover sound once on entry
            if (_currentHovered != _lastHoveredButton)
            {
                if (_currentHovered != null && hoverSound != null)
                    Audio.playAudio(hoverSound);
                _lastHoveredButton = _currentHovered;
            }

            // HINT BUTTON: dim if can't afford or already used (skip when paused)
            if (PauseMenu.Instance == null || !PauseMenu.Instance.IsPaused)
            {
                MoneyManager moneyCheck = MoneyManager.getInstance();
                bool hintAvailable = !hintUsed && moneyCheck != null && moneyCheck.canAfford(HINT_COST);
                ApplyAlpha(hintButton, hintAvailable ? 1.0f : 0.3f);
            }
            // CLICK
            if (Input.isMouseClicked(Input.Mouse.kMOUSE_BUTTON_LEFT))
            {
                if (_currentHovered != null && clickSound != null)
                    Audio.playAudio(clickSound); 
                
                bool anyPick = false;

                //TEXTURES NOT UPDATED YET
                //if (IsOver(pneumoniaButton, mp)) { pickedIllness = "Pneumonia"; anyPick = true; }
                if (IsOver(utiButton, mp)) { pickedIllness = "Urinary tract infection"; anyPick = true; }
                else if (IsOver(mildfluButton, mp)) { pickedIllness = "Mild Flu"; anyPick = true; }
                else if (IsOver(gastroButton, mp)) { pickedIllness = "Gastroenteritis"; anyPick = true; }
                else if (IsOver(noIllnessButton, mp)) { pickedIllness = "No Illness"; anyPick = true; }
                else if (IsOver(bloodinfectionButton, mp)) { pickedIllness = "Blood Infection"; anyPick = true; }
                else if (IsOver(severeallergyButton, mp)) { pickedIllness = "Severe Allergy"; anyPick = true; }
                else if (IsOver(neurologicalButton, mp)) { pickedIllness = "Toxic-Induced Neurological Distress"; anyPick = true; }
                else if (IsOver(druginducedButton, mp)) { pickedIllness = "Drug-Induced Cardiac Stress"; anyPick = true; }

                else if (IsOver(diagnoseButton, mp))
                {
                    if (PatientManager.TryGetIllness(currentStudent, out Illness data))
                    {   
                        if(pickedIllness == null) return;

                        // Check if this patient is VIP (for scoring)
                        bool isVIP = PatientManager.IsPatientVIP(currentStudent);

                        if(pickedIllness == data.Name){
                            MoneyManager.getInstance().addMoney(100.0f);
                            correctlyDiagnosed++;
                            pickedIllness = null;
                            // Play win sound
                            if (correctSound != null)
                            {
                                Logger.log(Logger.LogLevel.Info, "[GameStateManager] Playing winSound...");
                                Audio.playAudio(correctSound);
                                correctSoundTimer = 1.0f;
                            }
                            else
                            {
                                Logger.log(Logger.LogLevel.Error, "[GameStateManager] winSound is NULL, cannot play!");
                            }
                            Logger.log(Logger.LogLevel.Info, $"Actual Illness: {data.Name}");
                            Logger.log(Logger.LogLevel.Info, $"Picked Illness: {pickedIllness}");
                            Logger.log(Logger.LogLevel.Info, "CORRECT!!!!!!");

                            // Record correct diagnosis in score manager
                            LevelScoreManager.RecordDiagnosis(true, isVIP);
                            
                        } else {
                            MoneyManager.getInstance().deductMoney(100.0f);
                            if (loseSound != null)
                            {
                                Logger.log(Logger.LogLevel.Info, "[GameStateManager] Playing loseSound...");
                                Audio.playAudio(loseSound);
                            }
                            else
                            {
                                Logger.log(Logger.LogLevel.Error, "[GameStateManager] loseSound is NULL, cannot play!");
                            }
                            
                               
                            Logger.log(Logger.LogLevel.Info, $"Actual Illness: {data.Name}");
                            Logger.log(Logger.LogLevel.Info, $"Picked Illness: {pickedIllness}");
                            Logger.log(Logger.LogLevel.Info, "WRONGGG!!!!!!");
                            incorrectlyDiagnosed++;

                            // Record wrong diagnosis in score manager & penalty bar
                            LevelScoreManager.RecordDiagnosis(false, isVIP);
                            if (PenaltyBarMgr.Instance != null)
                                PenaltyBarMgr.Instance.RecordWrongDiagnosis();
                        }
                        totalDiagnosed++;
                        
                        WaypointPatientController controller = null;
                        foreach (var c in WaypointPatientControllerManager.GetAllControllers())
                        {
                            if (c.ObjectId == currentStudent)
                            {
                                controller = c;
                                break;
                            }
                        }

                        if (controller != null)
                        {
                            controller.MoveToExit();
                        }
                        
                        PatientManager.RemovePatient(currentStudent);
                        WaypointPatientControllerManager.AdvanceAllQueue();
                        pickedIllness = null;
                        SetVisible(false);

                        if (PatientUIPanel.Instance != null) PatientUIPanel.Instance.CloseAll();
                    }

                }
                else if (IsOver(backButton, mp))
                {
                    SetVisible(false);
                    if (currentStudent != null)
                    {
                        if (PatientUIPanel.Instance != null)
                        {
                            PatientUIPanel.Instance.OpenDetailsFor(currentStudent);
                        }
                        else if (PatientPanelMgr.Instance != null)
                        {
                            PatientPanelMgr.Instance.OpenDetailsFromDiagnose(currentStudent);
                            Logger.log(Logger.LogLevel.Info, $"From Diagnose Selected Patient: {currentStudent} ");
                        }
                    }
                }
                else if (IsOver(hintButton, mp) && !hintUsed)
                {
                    // Hint button clicked
                    MoneyManager moneyMgr = MoneyManager.getInstance();
                    if (moneyMgr != null && moneyMgr.canAfford(HINT_COST))
                    {
                        // Check illness FIRST before deducting money
                        if (PatientManager.TryGetIllness(currentStudent, out Illness illness))
                        {
                            // Build the list of all active button illnesses dynamically
                            List<string> allButtonIllnesses = new List<string>
                            {
                                "Urinary tract infection",
                                "Mild Flu",
                                "No Illness",
                                "Gastroenteritis"
                            };
                            if (bloodinfectionButton != null) allButtonIllnesses.Add("Blood Infection");
                            if (severeallergyButton != null) allButtonIllnesses.Add("Severe Allergy");
                            if (neurologicalButton != null) allButtonIllnesses.Add("Toxic-Induced Neurological Distress");
                            if (druginducedButton != null) allButtonIllnesses.Add("Drug-Induced Cardiac Stress");

                            var hintList = DiagnosisSystem.GetHintList(illness.Name, allButtonIllnesses, 3);

                            // Only charge if the illness is actually in the button list
                            if (hintList.Count > 0)
                            {
                                moneyMgr.deductMoney(HINT_COST);
                                hintUsed = true;
                                totalHintsUsed++;
                                hintedIllnesses = hintList;

                                string joinedIllnesses = string.Join(", ", hintedIllnesses);
                                Logger.log(Logger.LogLevel.Info, $"[Hint] Patient illness: {illness.Name}");
                                Logger.log(Logger.LogLevel.Info, $"[Hint] Highlighted: {joinedIllnesses}");

                                // Play hint sound
                                if (hintSound != null) Audio.playAudio(hintSound);

                                UpdateButtonTextures();
                                ApplyHintDimming();
                            }
                            else
                            {
                                Logger.log(Logger.LogLevel.Warning, $"[Hint] Illness '{illness.Name}' not in button list, hint not applied.");
                            }
                        }
                        else
                        {
                            Logger.log(Logger.LogLevel.Warning, "[Hint] Could not find illness for this patient!");
                        }
                    }
                    else
                    {
                        Logger.log(Logger.LogLevel.Info, "[Hint] Not enough money for hint!");
                    }
                }
                if (anyPick) UpdateButtonTextures(); 
            }
            ApplyTexture(mildfluButton, (pickedIllness == "Mild Flu") ? (mildfluSelected ?? mildflu) : IsOver(mildfluButton, mp) ? (mildfluHover ?? mildflu) : mildflu);
            ApplyTexture(noIllnessButton, (pickedIllness == "No Illness") ? (noIllnessSelected ?? noIllness) : IsOver(noIllnessButton, mp) ? (noIllnessHover ?? noIllness) : noIllness);
            ApplyTexture(gastroButton, (pickedIllness == "Gastroenteritis") ? (gastroSelected ?? gastro) : IsOver(gastroButton, mp) ? (gastroHover ?? gastro) : gastro);
            ApplyTexture(utiButton, (pickedIllness == "Urinary tract infection") ? (utiSelected ?? uti) : IsOver(utiButton, mp) ? (utiHover ?? uti) : uti);
            if (bloodinfectionButton != null) ApplyTexture(bloodinfectionButton, (pickedIllness == "Blood Infection") ? (bloodinfectionSelected ?? bloodinfection) : IsOver(bloodinfectionButton, mp) ? (bloodinfectionHover ?? bloodinfection) : bloodinfection);
            if (severeallergyButton != null) ApplyTexture(severeallergyButton, (pickedIllness == "Severe Allergy") ? (severeallergySelected ?? severeallergy) : IsOver(severeallergyButton, mp) ? (severeallergyHover ?? severeallergy) : severeallergy);
            if (neurologicalButton != null) ApplyTexture(neurologicalButton, (pickedIllness == "Toxic-Induced Neurological Distress") ? (neurologicalSelected ?? neurological) : IsOver(neurologicalButton, mp) ? (neurologicalHover ?? neurological) : neurological);
            if (druginducedButton != null) ApplyTexture(druginducedButton, (pickedIllness == "Drug-Induced Cardiac Stress") ? (druginducedSelected ?? druginduced) : IsOver(druginducedButton, mp) ? (druginducedHover ?? druginduced) : druginduced);
        }

        
        //TEXTURES NOT UPDATED YET
        private bool IsHinted(string illnessName)
        {
            return hintUsed && hintedIllnesses.Contains(illnessName);
        }

        private void UpdateButtonTextures()
        {
            ApplyTexture(utiButton, (pickedIllness == "Urinary tract infection") ? (utiSelected ?? uti) : uti);
            ApplyTexture(mildfluButton, (pickedIllness == "Mild Flu") ? (mildfluSelected ?? mildflu) : mildflu);
            ApplyTexture(noIllnessButton, (pickedIllness == "No Illness") ? (noIllnessSelected ?? noIllness) : noIllness);
            ApplyTexture(gastroButton, (pickedIllness == "Gastroenteritis") ? (gastroSelected ?? gastro) : gastro);
            if (bloodinfectionButton != null && bloodinfection != null) ApplyTexture(bloodinfectionButton, (pickedIllness == "Blood Infection") ? (bloodinfectionSelected ?? bloodinfection) : bloodinfection);
            if (severeallergyButton != null && severeallergy != null) ApplyTexture(severeallergyButton, (pickedIllness == "Severe Allergy") ? (severeallergySelected ?? severeallergy) : severeallergy);
            if (neurologicalButton != null && neurological != null) ApplyTexture(neurologicalButton, (pickedIllness == "Toxic-Induced Neurological Distress") ? (neurologicalSelected ?? neurological) : neurological);
            if (druginducedButton != null && druginduced != null) ApplyTexture(druginducedButton, (pickedIllness == "Drug-Induced Cardiac Stress") ? (druginducedSelected ?? druginduced) : druginduced);
        }


        /// <summary>
        /// Dims non-hinted illness buttons to 30% alpha, keeps hinted ones at full brightness.
        /// 
        /// This is outdated
        /// </summary>
        private void ApplyHintDimming()
        {
            if (!hintUsed) return;

            float dim = 0.3f;
            float full = 1.0f;

            ApplyAlpha(utiButton, IsHinted("Urinary tract infection") ? full : dim);
            ApplyAlpha(mildfluButton,          IsHinted("Mild Flu")                           ? full : dim);
            ApplyAlpha(bloodinfectionButton,   IsHinted("Blood Infection")                    ? full : dim);
            ApplyAlpha(severeallergyButton,    IsHinted("Severe Allergy")                     ? full : dim);
            ApplyAlpha(neurologicalButton,     IsHinted("Toxic-Induced Neurological Distress")? full : dim);
            ApplyAlpha(druginducedButton,      IsHinted("Drug-Induced Cardiac Stress")        ? full : dim);
            //ApplyAlpha(syntheticButton,        IsHinted("Synthetic Substance Toxicity")       ? full : dim);
            ApplyAlpha(gastroButton, IsHinted("Gastroenteritis") ? full : dim);
            ApplyAlpha(noIllnessButton, IsHinted("No Illness") ? full : dim);
        }

        private void ApplyTexture(Object o, Image2D t) { if (o != null && t != null) { var i = o.getComponent<Image2DComponent>(); if (i != null) { var d = i.Data; d.hashed = t.hashedID; i.Data = d; } } }
        private void ApplyAlpha(Object o, float a) { if (o != null) { var i = o.getComponent<Image2DComponent>(); if (i != null) { var d = i.Data; d.color.a = a; i.Data = d; } var t = o.getComponent<TextMeshUIComponent>(); if (t != null) { var d = t.Data; d.color.a = a; t.Data = d; } } }
        private void SetText(Object o, string tx) { if (o == null) return; var t = o.getComponent<TextMeshUIComponent>(); if (t != null) { var d = t.Data; d.text = tx; t.Data = d; } }
        private bool IsOver(Object o, Vector2D mp) { if (o == null) return false; var rt = o.getComponent<RectTransformComponent>(); if (rt == null) return false; var d = rt.Data; float hx = d.scale.x * 0.5f; float hy = d.scale.y * 0.5f; Vector2D tl = new Vector2D(d.position.x - hx, d.position.y + hy); Vector2D br = new Vector2D(d.position.x + hx, d.position.y - hy); return Utility.AABB(tl, br, mp); }
        public override void LateUpdate(double dt) { }
        public override void Free()
        {
            // Reset static state
            pickedIllness = null;
            Instance = null;

            // Reset UI/game state
            visible = false;
            GameState.SetUIState(false);
        }
    }
}
