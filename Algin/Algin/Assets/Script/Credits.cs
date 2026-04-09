using Script.Library;
using System;

namespace Script
{
    public class Credits : IScript
    {
        // Added slot for the 5th image
        [SerializeField] public Script.Library.Object image1;
        [SerializeField] public Script.Library.Object image2;
        [SerializeField] public Script.Library.Object image3;
        [SerializeField] public Script.Library.Object image4;
        [SerializeField] public Script.Library.Object image5;

        public AudioInstance creditsBGM;

        private double timer = 0.0;
        private const double SHOW_TIME = 3.0; 
        private const double FADE_TIME = 1.5; 

        // Updated for 5 images: 5 show periods + 5 fade periods
        private double TotalSequenceTime => (5 * SHOW_TIME) + (5 * FADE_TIME);

        public Credits() : base("") { }

        public override void Awake() { }

        public override void Start() 
        {
            GameState.ResetAll();
            Input.enableMouse();

            creditsBGM = Audio.getAudioInstance("MainMenuBGM", Obj);
            if (creditsBGM != null)
            {
                Audio.playAudio(creditsBGM);
            }
        }

        public override void Update(double dt)
        {
            timer += dt;

            if (image1 != null && image2 != null && image3 != null && image4 != null && image5 != null)
            {
                UpdateFades();
            }

            if (timer >= 22 || Input.isMousePressed(Input.Mouse.kMOUSE_BUTTON_LEFT))
            {
                if (!SceneFader.IsActive)
                {
                    SceneFader.StartFadeOut("Assets/Scenes/MainMenu.scene");
                }
            }
            SceneFader.Update(dt);
        }

        private void UpdateFades()
        {
            float a1 = 0, a2 = 0, a3 = 0, a4 = 0, a5 = 0;

            // Image 1 -> 2
            if (timer <= SHOW_TIME) { a1 = 1.0f; }
            else if (timer <= SHOW_TIME + FADE_TIME) {
                float t = (float)((timer - SHOW_TIME) / FADE_TIME);
                a1 = 1.0f - t; a2 = t;
            }
            // Image 2 -> 3
            else if (timer <= (2 * SHOW_TIME) + FADE_TIME) { a2 = 1.0f; }
            else if (timer <= (2 * SHOW_TIME) + (2 * FADE_TIME)) {
                float t = (float)((timer - ((2 * SHOW_TIME) + FADE_TIME)) / FADE_TIME);
                a2 = 1.0f - t; a3 = t;
            }
            // Image 3 -> 4
            else if (timer <= (3 * SHOW_TIME) + (2 * FADE_TIME)) { a3 = 1.0f; }
            else if (timer <= (3 * SHOW_TIME) + (3 * FADE_TIME)) {
                float t = (float)((timer - ((3 * SHOW_TIME) + (2 * FADE_TIME))) / FADE_TIME);
                a3 = 1.0f - t; a4 = t;
            }
            // Image 4 -> 5
            else if (timer <= (4 * SHOW_TIME) + (3 * FADE_TIME)) { a4 = 1.0f; }
            else if (timer <= (4 * SHOW_TIME) + (4 * FADE_TIME)) {
                float t = (float)((timer - ((4 * SHOW_TIME) + (3 * FADE_TIME))) / FADE_TIME);
                a4 = 1.0f - t; a5 = t;
            }
            // Image 5 Solid -> Final Fade Away
            else if (timer <= (5 * SHOW_TIME) + (4 * FADE_TIME)) { a5 = 1.0f; }
            else {
                float t = (float)((timer - ((5 * SHOW_TIME) + (4 * FADE_TIME))) / FADE_TIME);
                a5 = 1.0f - t;
                if (a5 < 0f) a5 = 0f;
            }

            SetAlpha(image1, a1);
            SetAlpha(image2, a2);
            SetAlpha(image3, a3);
            SetAlpha(image4, a4);
            SetAlpha(image5, a5);
        }

        private void SetAlpha(Script.Library.Object obj, float alpha)
        {
            var comp = obj.getComponent<Image2DComponent>();
            if (comp != null)
            {
                var data = comp.Data;
                data.color.a = alpha;
                comp.Data = data;
            }
        }

        public override void LateUpdate(double dt) { }

        public override void Free() 
        { 
            if (creditsBGM != null)
            {
                Audio.stopAudio(creditsBGM);
            }
        }
    }
}