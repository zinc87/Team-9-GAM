using Script.Library;
using System;
//
namespace Script
{
    public class GameStateManager : IScript
    {
        // =========================================================
        // SINGLETON LOGIC
        // =========================================================
        private static GameStateManager instance;
        public static GameStateManager getInstance()
        {
            return instance;
        }

        // =========================================================
        // STATE TRACKING
        // =========================================================
        private bool hasEnded = false; // Prevents multiple triggers
        private bool gameLoss = false;
        private bool gameWon = false;
        private bool hasGameStarted = false;
        public float magicWinningNumber = 50.0f;

        // =========================================================
        // AUDIO
        // =========================================================
        private AudioInstance winSound;
        private AudioInstance loseSound;

        // =========================================================
        // UI BACKGROUNDS
        // =========================================================
        [SerializeField] public Script.Library.Object loseScreenBG;
        [SerializeField] public Script.Library.Object winScreenBG;

        // =========================================================
        // BUTTONS
        // =========================================================
        [SerializeField] public Script.Library.Object retryButton;
        [SerializeField] public Script.Library.Object mainMenuButton;
        [SerializeField] public Script.Library.Object nextLevelButton;

        // ======================
        // FADE OVERLAY
        // ======================
        [SerializeField] public Script.Library.Object fadeOverlay; // Full-screen black Image2DComponent

        // =========================================================
        // AUDIO TIMER
        // =========================================================
        private double loseAudioTimer = 0.0;
        private bool loseAudioPlaying = false;
        private const double LOSE_AUDIO_DURATION = 6.5;

        public string retryScene = "Assets/Scenes/TempMap - Copy.scene";
        public string mainMenuScene = "Assets/Scenes/MainMenu.scene";

        public GameStateManager() : base("") { }

        public override void Awake()
        {
            instance = this;
        }

        public override void Start()
        {
            // Initialize audio instances
            winSound = Audio.getAudioInstance("AccordionCome CTE04_10.3", Obj);
            loseSound = Audio.getAudioInstance("Cartoon, Musical, Orchestral, Trombone, Vintage, Failure, Sad SND11701", Obj);

            // Debug logging for audio
            if (winSound == null)
                Logger.log(Logger.LogLevel.Error, "[GameStateManager] winSound (CorrectSFX) is NULL!");
            else
                Logger.log(Logger.LogLevel.Info, "[GameStateManager] winSound (CorrectSFX) initialized.");

            if (loseSound == null)
                Logger.log(Logger.LogLevel.Error, "[GameStateManager] loseSound (WrongSFX) is NULL!");
            else
                Logger.log(Logger.LogLevel.Info, "[GameStateManager] loseSound (WrongSFX) initialized.");

            // Subscribe to timer end event for lose condition
            var timer = TimerManager.getInstance();
            if (timer != null)
                timer.OnTimerEnd += OnTimerReachedZero;

            HideAllEndUI();

            // Initialize fade overlay (starts transparent since we're already in the level)
            if (fadeOverlay != null)
            {
                var imgComp = fadeOverlay.getComponent<Image2DComponent>();
                if (imgComp != null)
                {
                    SceneFader.Init(imgComp);
                    // Set overlay to transparent immediately (no fade-in needed here)
                    var dd = imgComp.Data;
                    dd.color.a = 0.0f;
                    imgComp.Data = dd;
                }
            }
        }

        private void HideAllEndUI()
        {
            loseScreenBG?.setActive(false);
            winScreenBG?.setActive(false);

            retryButton?.setActive(false);
            nextLevelButton?.setActive(false);
            mainMenuButton?.setActive(false);
        }

        public override void Update(double dt)
        {
            // Drive the fader every frame
            SceneFader.Update(dt);

            Vector2D mp;
            Input.getCurMosusePos(out mp);

            if (!hasEnded)
            {
                if (PatientManager.getPatientCount() > 0)
                {
                    hasGameStarted = true;
                }

                if (hasGameStarted && PatientManager.getPatientCount() == 0)
                {
                    // For Level 1 map, ensure VIP has spawned before ending the level
                    bool isLevel1 = (LevelScoreManager.CurrentLevel == 1 || Scene.getSceneName() == "TempMap - Copy.scene");
                    if (isLevel1 && WaypointPatientSpawner.Instance != null && !WaypointPatientSpawner.Instance.VipSpawned)
                    {
                        return; // Wait for VIP to spawn!
                    }

                    if (LevelSummaryScreen.Instance != null)
                    {
                        hasEnded = true;
                        LevelSummaryScreen.Instance.ShowSummary(PatientSpawner._patientCounter);
                    }
                    else
                    {
                        var money = MoneyManager.getInstance();
                        if (money != null && money.getMoneyAmount() >= magicWinningNumber)
                        {
                            TriggerWin();
                        }
                        else
                        {
                            TriggerLose();
                        }
                    }
                }
            }

            // Stop lose audio after 6 seconds
            if (loseAudioPlaying)
            {
                loseAudioTimer += dt;
                if (loseAudioTimer >= LOSE_AUDIO_DURATION)
                {
                    Audio.stopAudio(loseSound);
                    loseAudioPlaying = false;
                }
            }

            //// Win check only if game running
            //if (!hasEnded)
            //{
            //    var money = MoneyManager.getInstance();
            //    if (money != null && money.getMoneyAmount() >= magicWinningNumber)
            //    {
            //        TriggerWin();
            //    }
            //}

            // Always allow clicks when end screen visible
            // Block clicks while fading
            if (hasEnded && !SceneFader.IsActive && Input.isMousePressed(Input.Mouse.kMOUSE_BUTTON_LEFT))
            {   

                //UI stays the same, only buttons change
                if(gameLoss)
                {
                    if (IsOver(retryButton, mp))
                    {
                        HideAllEndUI();
                        GameState.SetGameOver(false);
                        LevelScoreManager.ResetLevelCounters();
                        if (LevelScoreManager.CurrentLevel == 2)
                        {
                            SceneFader.StartFadeOut("Assets/Scenes/Level_2.scene");
                        }
                        else
                        {
                            SceneFader.StartFadeOut(retryScene);
                        }
                        return;
                    }

                    if (IsOver(mainMenuButton, mp))
                    {
                        HideAllEndUI();
                        GameState.SetGameOver(false);  // Reset game over state before returning to menu
                        LevelScoreManager.Reset();
                        SceneFader.StartFadeOut(mainMenuScene);
                        return;
                    }
                }

                if (gameWon)
                {
                    if (IsOver(nextLevelButton, mp))
                    {
                        HideAllEndUI();
                        GameState.SetGameOver(false);
                        
                        if (LevelScoreManager.CurrentLevel == 1)
                        {
                            LevelScoreManager.AdvanceLevel();
                            SceneFader.StartFadeOut("Assets/Scenes/Level1_end_cutscene.scene");
                        }
                        else
                        {
                            LevelScoreManager.AdvanceLevel();
                            if (LevelScoreManager.CurrentLevel == 2)
                            {
                                SceneFader.StartFadeOut("Assets/Scenes/Level_2.scene");
                            }
                            else if (LevelScoreManager.CurrentLevel == 3)
                            {
                                SceneFader.StartFadeOut("Assets/Scenes/End_Cutscene.scene");
                            }
                            else
                            {
                                LevelScoreManager.Reset();
                                SceneFader.StartFadeOut(mainMenuScene);
                            }
                        }
                        return;
                    }

                    if (IsOver(mainMenuButton, mp))
                    {
                        HideAllEndUI();
                        GameState.SetGameOver(false);  // Reset game over state before returning to menu
                        LevelScoreManager.Reset();
                        SceneFader.StartFadeOut(mainMenuScene);
                        return;
                    }
                }

            }
        }

        public override void LateUpdate(double dt) { }

        public override void Free()
        {
            // Unsubscribe from timer event
            var timer = TimerManager.getInstance();
            if (timer != null)
            {
                timer.OnTimerEnd -= OnTimerReachedZero;
            }
            instance = null;
        }

        // =========================================================
        // WIN / LOSE LOGIC
        // =========================================================
        private void OnTimerReachedZero()
        {
            if (hasEnded) return;

            if (PatientManager.getPatientCount() > 0)
            {
                LevelScoreManager.TimeRanOut = true;
            }

            if (LevelSummaryScreen.Instance != null)
            {
                hasEnded = true;
                LevelSummaryScreen.Instance.ShowSummary(PatientSpawner._patientCounter);
            }
            else
            {
                var money = MoneyManager.getInstance();
                if (money != null && money.getMoneyAmount() >= magicWinningNumber)
                {
                    TriggerWin();
                }
                else
                {
                    TriggerLose();
                }
            }
        }

        public void TriggerWinSFX()
        {
            // Play win sound
            if (winSound != null)
            {
                Logger.log(Logger.LogLevel.Info, "[GameStateManager] Playing winSound...");
                Audio.playAudio(winSound);
            }
        }

        public void TriggerLoseSFX()
        {
            // Play win sound
            if (loseSound != null)
            {
                Logger.log(Logger.LogLevel.Info, "[GameStateManager] Playing loseSound...");
                Audio.playAudio(loseSound);
            }
        }

        public void TriggerWin(bool fromSummary = false)
        {
            if (hasEnded && !fromSummary) return;
            hasEnded = true;
            gameWon = true;
            gameLoss = false;

            GameState.SetGameOver(true);
            Logger.log(Logger.LogLevel.Info, "You have WON the game");

            // Play win sound
            if (winSound != null)
            {
                Logger.log(Logger.LogLevel.Info, "[GameStateManager] Playing winSound...");
                Audio.playAudio(winSound);
            }
            ShowWinScreen();
        }

        public void TriggerLose(bool fromSummary = false)
        {
            if (hasEnded && !fromSummary) return;
            hasEnded = true;
            gameLoss = true;
            gameWon = false;

            GameState.SetGameOver(true);

            if (loseSound != null)
            {
                Audio.playAudio(loseSound);
                loseAudioPlaying = true;
                loseAudioTimer = 0.0;
            }

            ShowLoseScreen();
        }

        // =========================================================
        // UI
        // =========================================================
        private void ShowLoseScreen()
        {
            // Disabled - LevelSummaryScreen now handles all end-game UI
            HideAllEndUI();
        }

        private void ShowWinScreen()
        {
            // Disabled - LevelSummaryScreen now handles all end-game UI
            HideAllEndUI();
        }

        private bool IsOver(Script.Library.Object o, Vector2D mp) { if (o == null) return false; var rt = o.getComponent<RectTransformComponent>(); if (rt == null) return false; var d = rt.Data; float hx = d.scale.x * 0.5f; float hy = d.scale.y * 0.5f; Vector2D tl = new Vector2D(d.position.x - hx, d.position.y + hy); Vector2D br = new Vector2D(d.position.x + hx, d.position.y - hy); return Utility.AABB(tl, br, mp); }
    }
}
