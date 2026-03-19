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
        [SerializeField] public Script.Library.Object scoreText;         // TextMeshUIComponent - "Score: earned / max (ratio%)"
        [SerializeField] public Script.Library.Object resultText;        // TextMeshUIComponent - "PASSED" / "FAILED"
        [SerializeField] public Script.Library.Object moneyEarnedText;   // TextMeshUIComponent - "Money Earned: $X"
        [SerializeField] public Script.Library.Object hintsUsedText;     // TextMeshUIComponent - "Hints Used: N"
        [SerializeField] public Script.Library.Object timeTakenText;     // TextMeshUIComponent - "Time: MM:SS"
        [SerializeField] public Script.Library.Object continueButton;    // Image2DComponent + RectTransformComponent
        [SerializeField] public Script.Library.Object continueButtonText;// TextMeshUIComponent - "Next Level" / "Retry"
        [SerializeField] public Script.Library.Object moreDetailsButton;
        [SerializeField] public Script.Library.Object moreDetailsButtonText;
        public string mainMenuScene = "Assets/Scenes/MainMenu.scene";
        public string retryScene = "Assets/Scenes/TempMap - Copy.scene";

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
                if (IsOver(continueButton, mp) || IsOver(continueButtonText, mp))
                {
                    Logger.log(Logger.LogLevel.Info, "[LevelSummaryScreen] CONTINUE CLICKED!");
                    if (clickSound != null) Audio.playAudio(clickSound);
                    
                    DiagnosePanelMgr.ResetCounters();
                    SetVisible(false);

                    if (levelPassed)
                    {
                        GameStateManager.getInstance()?.TriggerWin(true);
                    }
                    else
                    {
                        GameStateManager.getInstance()?.TriggerLose(true);
                    }
                    return;
                }

                bool overMoreDetails = IsOver(moreDetailsButton, mp) || IsOver(moreDetailsButtonText, mp);
                if (overMoreDetails)
                {
                    Logger.log(Logger.LogLevel.Info, "[LevelSummaryScreen] More Details clicked");
                    return;
                }

                // If we get here, they clicked but missed EVERYTHING
                Logger.log(Logger.LogLevel.Info, $"[LevelSummaryScreen] CLICK MISSED. MousePos=({mp.x}, {mp.y})");
                if (continueButton != null)
                {
                    var rt = continueButton.getComponent<RectTransformComponent>();
                    if (rt != null)
                    {
                        var d = rt.Data;
                        Logger.log(Logger.LogLevel.Info, $"[LevelSummaryScreen] ContinueButton Pos=({d.position.x}, {d.position.y}) Scale=({d.scale.x}, {d.scale.y})");
                    }
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
            string moneyLine = string.Format("Money Earned: ${0:0}", moneyTotal);

            // Hints used
            int hints = DiagnosePanelMgr.totalHintsUsed;
            string hintsLine = string.Format("Hints Used: {0}", hints);

            // Time taken (initial - remaining)
            string timeLine = "Time: --:--";
            var timer = TimerManager.getInstance();
            if (timer != null)
            {
                float elapsed = timer.initialTime - timer.GetTimeLeft();
                if (elapsed < 0f) elapsed = 0f;
                int mins = (int)elapsed / 60;
                int secs = (int)elapsed % 60;
                timeLine = string.Format("Time Taken: {0:00}:{1:00}", mins, secs);
            }

            // Set text BEFORE making visible so content is ready
            SetText(titleText,             string.Format("Level {0} Summary", LevelScoreManager.CurrentLevel));
            SetText(patientsDiagnosed,       string.Format("Patients Diagnosed: {0}/{1}", diagnosed, totalPatients));
            SetText(patientsCorrDiagnosed,  string.Format("Patients Correctly Diagnosed: {0}/{1}", correct, totalPatients));
            //SetText(scoreText,             resultLine);
            SetText(resultText,            ""); // reserved for future use
            SetText(moneyEarnedText,       moneyLine);
            SetText(hintsUsedText,         hintsLine);
            SetText(timeTakenText,         timeLine);
            SetText(continueButtonText,    levelPassed ? "Next Level" : "Retry");
            SetText(moreDetailsButtonText, "More Details");

            SetVisible(true);
            clickCooldown = 0.5f;

            GameState.SetGameOver(true);

            if (timer != null)
                timer.Pause();
        }

        // =========================================================
        // UI HELPERS
        // =========================================================

        private void SetVisible(bool isVisible)
        {
            if (visible == isVisible) return;
            visible = isVisible;
            float a = visible ? 1.0f : 0.0f;

            // Only touch Image2DComponent alpha — never read/write
            // TextMeshUIComponentData for visibility, to avoid
            // corrupting the font hash through marshalling.
            HideImage(summaryBG, a);
            HideImage(continueButton, a);
            HideImage(moreDetailsButton, a);

            // For text objects: clear text to "" when hiding.
            // ShowSummary() sets the real text before calling SetVisible(true).
            if (!isVisible)
            {
                SetText(titleText, "");
                SetText(patientsDiagnosed, "");
                SetText(patientsCorrDiagnosed, "");
                SetText(scoreText, "");
                SetText(resultText, "");
                SetText(moneyEarnedText, "");
                SetText(hintsUsedText, "");
                SetText(timeTakenText, "");
                SetText(continueButtonText, "");
                SetText(moreDetailsButtonText, "");
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
    }
}
