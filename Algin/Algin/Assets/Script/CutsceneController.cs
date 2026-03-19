using Script.Library;
using System;

namespace Script
{
    public class CutsceneController : IScript
    {
        public VideoPlayerComponent videoPlayer; // Reference to the component
         
        public string nextScene = "Assets/Scenes/Level_2.scene"; // Modify in editor or here

        [SerializeField] public Script.Library.Object skipPromptUI; // The UI to show "Press Space to skip"
        
        private float timer = 0.0f;
        private float currentAlpha = 0.0f;
        private bool previousTonemap = true;
        //private bool uiVisible = false;

        public override void Awake()
        {
            // Disable tonemapping/gamma for video cutscenes (store previous state to restore later)
            previousTonemap = Graphics.TonemapEnabled;
            Graphics.TonemapEnabled = false;

             // Initially hide the skip prompt
            if (skipPromptUI != null)
            {
                ApplyAlpha(skipPromptUI, 0.0f);
            }
        }

        public override void Start()
        {
            // Try to get the component if attached to same object
            videoPlayer = Obj.getComponent<VideoPlayerComponent>();
            if (videoPlayer == null)
            {
                Console.WriteLine("[CutsceneController] No VideoPlayer component found on this object!");
            }

            // Initially hide the skip prompt
            if (skipPromptUI != null)
            {
                ApplyAlpha(skipPromptUI, 0.0f);
            }
        }

        public override void Update(double dt)
        {
            if (videoPlayer == null) return;

            // Timer logic for fading in UI
            timer += (float)dt;
            if (skipPromptUI != null)
            {
                if (timer <= 3.0f)
                {
                    // Force invisible during partial wait to ensure it doesn't appear
                    ApplyAlpha(skipPromptUI, 0.0f);
                }
                else if (currentAlpha < 1.0f)
                {
                    currentAlpha += (float)dt; // 1 second fade duration
                    if (currentAlpha > 1.0f) currentAlpha = 1.0f;
                    ApplyAlpha(skipPromptUI, currentAlpha);
                }
            }

            // SKIP Logic
            if (Input.isKeyPressed(Input.KeyCode.kSPACE) || Input.isKeyPressed(Input.KeyCode.kENTER))
            {
                Console.WriteLine("[CutsceneController] User skipped video.");
                LoadNextScene();
                return;
            }

            // Completion Logic
            if (videoPlayer.IsFinished())
            {
                Console.WriteLine("[CutsceneController] Video finished.");
                LoadNextScene();
            }
        }

        public override void LateUpdate(double dt) { }
        public override void Free()
        {
            // Restore previous tonemap state on cleanup
            Graphics.TonemapEnabled = previousTonemap;
        }

        void LoadNextScene()
        {
            // Ensure we don't load multiple times
            videoPlayer.Stop();
            GameState.ResetAll();  // Reset all game state for fresh start
            Graphics.TonemapEnabled = previousTonemap;

            // Branching based on Level (1 = Intro -> Level 1 Map, 2 = Level 1 -> Level 2 Map)
            if (LevelScoreManager.CurrentLevel == 1)
            {
                Scene.loadScene("Assets/Scenes/TempMap - Copy.scene");
            }
            else if (LevelScoreManager.CurrentLevel == 2)
            {
                Scene.loadScene("Assets/Scenes/Level_2.scene");
            }
            else if (LevelScoreManager.CurrentLevel == 3)
            {
                LevelScoreManager.Reset();
                Scene.loadScene("Assets/Scenes/MainMenu.scene");
            }
            else
            {
                Scene.loadScene(nextScene);
            }
        }

        private void ApplyAlpha(Script.Library.Object o, float a)
        {
            if (o != null)
            {
                var i = o.getComponent<Image2DComponent>();
                if (i != null)
                {
                    var d = i.Data;
                    d.color.a = a;
                    i.Data = d;
                }
                var t = o.getComponent<TextMeshUIComponent>();
                if (t != null)
                {
                    var d = t.Data;
                    d.color.a = a;
                    t.Data = d;
                }
            }
        }
    }
}
