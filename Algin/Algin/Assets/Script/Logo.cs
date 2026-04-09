using Script.Library;
using System;

namespace Script
{
    public class Logo : IScript
    {
        private double timer = 0.0;
        
        // --- TIMING CONFIGURATION ---
        private const double FADE_IN_DURATION = 15;  // Time to fade from Black to Logo
        private const double SOLID_DISPLAY_TIME = 3.0; // Time logo stays fully visible
        private const double FADE_OUT_DURATION = 3.0; // Time to fade from Logo to Black
        
        private double FadeOutStartTime => FADE_IN_DURATION + SOLID_DISPLAY_TIME;
        private double TotalSceneTime => FadeOutStartTime + FADE_OUT_DURATION;

        public Logo() : base("") { }

        public override void Awake() { }

        public override void Start() 
        {
            GameState.ResetAll();
            
            // 1. Initial State: Set Logo Alpha to 0 immediately so it can fade IN
            var img = Obj.getComponent<Image2DComponent>();
            if (img != null)
            {
                var data = img.Data;
                data.color.a = 0.0f;
                img.Data = data;
            }

            // 2. Start the global scene fader (the black screen blanket)
            SceneFader.StartFadeIn();
        }

        public override void Update(double dt)
        {
            timer += dt;
            
            var img = Obj.getComponent<Image2DComponent>();
            if (img == null) return;
            
            var data = img.Data;

            // --- PHASE 1: FADE IN ---
            if (timer <= FADE_IN_DURATION)
            {
                float alpha = (float)(timer / FADE_IN_DURATION);
                data.color.a = alpha;
            }
            // --- PHASE 2: SOLID (Do nothing, let alpha stay at 1.0) ---
            else if (timer > FADE_IN_DURATION && timer < FadeOutStartTime)
            {
                data.color.a = 1.0f;
            }
            // --- PHASE 3: FADE OUT ---
            else if (timer >= FadeOutStartTime)
            {
                double fadeOutProgress = (timer - FadeOutStartTime) / FADE_OUT_DURATION;
                float alpha = 1.0f - (float)fadeOutProgress;
                
                if (alpha < 0f) alpha = 0f;
                data.color.a = alpha;
            }

            // Apply the calculated alpha back to the component
            img.Data = data;

            // --- SCENE TRANSITION ---
            if (timer >= TotalSceneTime)
            {
                SceneFader.StartFadeOut("Assets/Scenes/MainMenu.scene");
            }
        }

        public override void LateUpdate(double dt) { }
        public override void Free() { }
    }
}