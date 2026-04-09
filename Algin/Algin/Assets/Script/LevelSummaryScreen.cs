using Script.Library;

namespace Script
{
    public class LevelSummaryScreen : IScript
    {
        // =========================================================
        // SINGLETON
        // =========================================================
        public static LevelSummaryScreen Instance { get; private set; }

        // =========================================================
        // SERIALIZED FIELDS (wire these in the engine editor)
        // =========================================================
        [SerializeField] public Script.Library.Object summaryBG;         // Image2DComponent - BlueSummaryBackground.png
        [SerializeField] public Script.Library.Object titleText;         // TextMeshUIComponent - "Level Summary"
        [SerializeField] public Script.Library.Object patientsDiagnosed;    // TextMeshUIComponent - "Patients Diagnosed: X/N"
        [SerializeField] public Script.Library.Object patientsCorrDiagnosed; // TextMeshUIComponent - "Patients Correctly Diagnosed: X/N"
        [SerializeField] public Script.Library.Object moneyEarnedText;   // TextMeshUIComponent - "Money Earned: $X"
        [SerializeField] public Script.Library.Object hintsUsedText;     // TextMeshUIComponent - "Hints Used: N"
        [SerializeField] public Script.Library.Object timeTakenText;     // TextMeshUIComponent - "Time: MM:SS"
        [SerializeField] public Script.Library.Object retryButton;        // Image2DComponent + RectTransformComponent
        [SerializeField] public Script.Library.Object mainMenuButton;      // Image2DComponent
        [SerializeField] public Script.Library.Object nextLevelButton;      // Image2DComponent
        [SerializeField] public Image2D nextLevelButtonDefault;  // Original button texture (auto-captured in Start)
        [SerializeField] public Image2D nextLevelButtonDisabled;  // Image2D - grayscale/blackout texture for failed state
        public string mainMenuScene = "Assets/Scenes/MainMenu.scene";
        public string retryScene = "Assets/Scenes/TempMap - Copy.scene";
        public string level1Cutscene = "Assets/Scenes/Level1_end_cutscene.scene";
        public string level2Scene = "Assets/Scenes/Level_2.scene";
        public string endCutscene = "Assets/Scenes/End_Cutscene.scene";

        [SerializeField] public Script.Library.Object starRatingImage;  // Image2DComponent 
        [SerializeField] public Image2D starState0;  // 0 stars image
        [SerializeField] public Image2D starState1;  // 1 star image
        [SerializeField] public Image2D starState2;  // 2 stars image
        [SerializeField] public Image2D starState3;  // 3 stars image

        [SerializeField] public Script.Library.Object resultImage;  // Image2DComponent
        [SerializeField] public Image2D resultWinImage;
        [SerializeField] public Image2D resultLoseImage;

        // =========================================================
        // STATE
        // =========================================================
        // Start as true so that SetVisible(false) in Start() actually runs
        private bool visible = true;
        private float clickCooldown = 0.0f;
        private bool levelPassed = false;

        private AudioInstance clickSound;

        public LevelSummaryScreen() : base("") { }

        public override void Awake()
        {
            Instance = this;
            DiagnosePanelMgr.ResetCounters();
        }

        public override void Start()
        {
            // Hide all summary UI at game start
            SetVisible(false);

            clickSound = Audio.getAudioInstance("ButtonPress_003", Obj);
        }

        public override void Update(double dt)
        {
            if (!visible) return;

            if (clickCooldown > 0.0f)
            {
                clickCooldown -= (float)dt;
                return;
            }

            Vector2D mp;
            Input.getCurMosusePos(out mp);

            if (Input.isMousePressed(Input.Mouse.kMOUSE_BUTTON_LEFT))
            {
                if (IsOver(retryButton, mp))
                {
                    Logger.log(Logger.LogLevel.Info, "[LevelSummaryScreen] RETRY CLICKED!");
                    if (clickSound != null) Audio.playAudio(clickSound);
                    
                    DiagnosePanelMgr.ResetCounters();
                    LevelScoreManager.ResetLevelCounters();
                    SetVisible(false);

                    // Pick the correct scene based on current level
                    string sceneToRetry;
                    if (LevelScoreManager.CurrentLevel == 2)
                        sceneToRetry = level2Scene;
                    else
                        sceneToRetry = retryScene;  // defaults to Level 1

                    SceneFader.StartFadeOut(sceneToRetry);
                    return;
                }

                if (IsOver(mainMenuButton, mp))
                {
                    Logger.log(Logger.LogLevel.Info, "[LevelSummaryScreen] MAIN MENU CLICKED!");
                    if (clickSound != null) Audio.playAudio(clickSound);
                    
                    DiagnosePanelMgr.ResetCounters();
                    LevelScoreManager.Reset();
                    SetVisible(false);
                    SceneFader.StartFadeOut(mainMenuScene);
                    return;
                }

                if (IsOver(nextLevelButton, mp) && levelPassed)
                {
                    Logger.log(Logger.LogLevel.Info, "[LevelSummaryScreen] NEXT LEVEL CLICKED!");
                    if (clickSound != null) Audio.playAudio(clickSound);
                    
                    DiagnosePanelMgr.ResetCounters();
                    SetVisible(false);
                    
                    if (LevelScoreManager.CurrentLevel == 1)
                    {
                        LevelScoreManager.AdvanceLevel();
                        SceneFader.StartFadeOut(level1Cutscene);
                    }
                    else if (LevelScoreManager.CurrentLevel == 2)
                    {
                        //LevelScoreManager.AdvanceLevel();
                        //SceneFader.StartFadeOut(level2Scene);
                        LevelScoreManager.AdvanceLevel();
                        SceneFader.StartFadeOut(endCutscene);
                    }
                    else
                    {
                        LevelScoreManager.AdvanceLevel();
                        SceneFader.StartFadeOut(endCutscene);
                    }
                    return;
                }
            }
        }

        public override void LateUpdate(double dt) { }

        public override void Free()
        {
            Instance = null;
        }

        // =========================================================
        // PUBLIC API
        // =========================================================

        public void ShowSummary(int totalPatients)
        {
            string sceneName = Scene.getSceneName();
            Logger.log(Logger.LogLevel.Info, $"WaypointPatientSpawner: Scene name = '{sceneName}'");
            if (sceneName.Contains("TempMap"))
            {
                if (totalPatients <= 0)
                    totalPatients = 6;
            }
            else if (sceneName.Contains("Level_2"))
            {
                if (totalPatients <= 0)
                    totalPatients = 8;
            }


            int diagnosed = DiagnosePanelMgr.totalDiagnosed;
            int correct   = DiagnosePanelMgr.correctlyDiagnosed;

            // Compute score from LevelScoreManager
            LevelScoreManager.ComputeScore(out int earned, out int max, out float ratio);
            levelPassed = LevelScoreManager.HasPassed();

            Logger.log(Logger.LogLevel.Info,
                $"[LevelSummaryScreen] ShowSummary: Diagnosed={diagnosed}/{totalPatients}, Correct={correct}/{totalPatients}, Score={earned}/{max} ({ratio:P0}), Passed={levelPassed}");

            // Build summary strings
            string nonVipLine = string.Format("Non-VIP: {0}/{1} correct",
                LevelScoreManager.NonVipCorrect, LevelScoreManager.NonVipTotal);
            string vipLine = string.Format("VIP: {0}/{1} correct",
                LevelScoreManager.VipCorrect, LevelScoreManager.VipTotal);
            string scoreLine = string.Format("Score: {0}/{1} ({2:0}%)", earned, max, ratio * 100f);
            string resultLine = levelPassed ? "PASSED" : "FAILED";

            // Penalty bar warning
            if (PenaltyBarMgr.Instance != null && PenaltyBarMgr.Instance.IsMaxedOut())
                resultLine = "FAILED (Penalty bar maxed!)";
            else if (LevelScoreManager.TimeRanOut)
                Logger.log(Logger.LogLevel.Info,
                    "[LevelSummaryScreen] FAILED — time ran out");
                //resultLine = "FAILED (Time Out!)";

            // --- Compute extra stats ---
            // Money earned
            float moneyTotal = 0f;
            var moneyMgr = MoneyManager.getInstance();
            if (moneyMgr != null)
                moneyTotal = moneyMgr.getMoneyAmount();
            string sign = moneyTotal >= 0 ? "+" : "-";
            float moneyAbs = moneyTotal >= 0 ? moneyTotal : -moneyTotal;
            string moneyLine = string.Format("{0}${1:0}", sign, moneyAbs);

            // Hints used
            int hints = DiagnosePanelMgr.totalHintsUsed;
            string hintsLine = string.Format("{0}", hints);

            // Time taken (initial - remaining)
            string timeLine = "";
            var timer = TimerManager.getInstance();
            if (timer != null)
            {
                float elapsed = timer.initialTime - timer.GetTimeLeft();
                if (elapsed < 0f) elapsed = 0f;
                int mins = (int)elapsed / 60;
                int secs = (int)elapsed % 60;
                timeLine = string.Format("{0:00}:{1:00}", mins, secs);
            }

            // Force-clear any lingering diagnosis result popup
            if (DiagnosePanelMgr.Instance != null)
                DiagnosePanelMgr.Instance.ForceHideResultPopup();

            // Set text BEFORE making visible so content is ready
            SetText(titleText,             string.Format("Level {0}", LevelScoreManager.CurrentLevel));
            SetText(patientsDiagnosed,       string.Format("{0}/{1}", diagnosed, totalPatients));
            SetText(patientsCorrDiagnosed,  string.Format("{0}/{1}", correct, totalPatients));
            SetText(moneyEarnedText,       moneyLine);
            SetText(hintsUsedText,         hintsLine);
            SetText(timeTakenText,         timeLine);

            SetVisible(true);
            SetStarRating(levelPassed, ratio);
            SetResultImage(levelPassed);
            SetNextLevelButtonState(levelPassed);
            clickCooldown = 0.5f;

            GameState.SetGameOver(true);

            if (timer != null)
            {
                timer.Pause();
                timer.HideUI();
            }
            if(resultLine == "PASSED")
            {
                GameStateManager.getInstance().TriggerWinSFX();
            }
            else
            {
                GameStateManager.getInstance().TriggerLoseSFX();
            }
        }

        // =========================================================
        // UI HELPERS
        // =========================================================

        private void SetVisible(bool isVisible)
        {
            if (visible == isVisible) return;
            visible = isVisible;
            float a = visible ? 1.0f : 0.0f;

            HideImage(summaryBG, a);
            HideImage(retryButton, a);
            HideImage(mainMenuButton, a);
            HideImage(nextLevelButton, a);
            HideImage(starRatingImage, a);
            HideImage(resultImage, a);

            if (!isVisible)
            {
                SetText(titleText, "");
                SetText(patientsDiagnosed, "");
                SetText(patientsCorrDiagnosed, "");
                SetText(moneyEarnedText, "");
                SetText(hintsUsedText, "");
                SetText(timeTakenText, "");
            }
        }

        private void HideImage(Script.Library.Object o, float a)
        {
            if (o == null) return;
            var img = o.getComponent<Image2DComponent>();
            if (img == null) return;
            var d = img.Data;
            d.color.a = a;
            img.Data = d;
        }

        private void SetText(Script.Library.Object o, string text)
        {
            if (o == null) return;
            var t = o.getComponent<TextMeshUIComponent>();
            if (t == null) return;
            var d = t.Data;
            d.text = text;
            t.Data = d;
        }

        private bool IsOver(Script.Library.Object o, Vector2D mp)
        {
            if (o == null) return false;
            var rt = o.getComponent<RectTransformComponent>();
            if (rt == null) return false;
            var d = rt.Data;
            float hx = d.scale.x * 0.5f;
            float hy = d.scale.y * 0.5f;
            Vector2D tl = new Vector2D(d.position.x - hx, d.position.y + hy);
            Vector2D br = new Vector2D(d.position.x + hx, d.position.y - hy);
            return Utility.AABB(tl, br, mp);
        }

        private void SetStarRating(bool passed, float ratio)
        {
            if (starRatingImage == null) return;
            var img = starRatingImage.getComponent<Image2DComponent>();
            if (img == null) return;

            Image2D tex;
            if (!passed)
                tex = starState0;
            else if (ratio >= 0.80f)
                tex = starState3;
            else if (ratio >= 0.60f)
                tex = starState2;
            else if (ratio >= 0.40f)
                tex = starState1;
            else
                tex = starState0;

            if (tex == null) return;
            var d = img.Data;
            d.hashed = tex.hashedID;
            d.color.a = visible ? 1.0f : 0.0f;  // respect current visibility
            img.Data = d;
        }

        private void SetResultImage(bool passed)
        {
            if (resultImage == null) return;
            var img = resultImage.getComponent<Image2DComponent>();
            if (img == null) return;

            Image2D tex = passed ? resultWinImage : resultLoseImage;
            if (tex == null) return;

            var d = img.Data;
            d.hashed = tex.hashedID;
            d.color.a = visible ? 1.0f : 0.0f;
            img.Data = d;
        }

        private void SetNextLevelButtonState(bool passed)
        {
            if (nextLevelButton == null) return;
            var img = nextLevelButton.getComponent<Image2DComponent>();
            if (img == null) return;

            if (!passed && nextLevelButtonDisabled != null)
            {
                var d = img.Data;
                d.hashed = nextLevelButtonDisabled.hashedID;
                img.Data = d;
            }
            else if (passed && nextLevelButtonDefault != null)
            {
                var d = img.Data;
                d.hashed = nextLevelButtonDefault.hashedID;
                img.Data = d;
            }
        }
    }
}
