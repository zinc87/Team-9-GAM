using Script.Library;
using System;

namespace Script
{
    /// <summary>
    /// Static helper that drives a full-screen black overlay to produce
    /// fade-in (black → transparent) and fade-out (transparent → black)
    /// transitions between scenes.
    ///
    /// Usage:
    ///   1. Place a full-screen black Image2DComponent in the scene.
    ///   2. In your script's Awake/Start, call SceneFader.Init(overlay)
    ///      followed by SceneFader.StartFadeIn().
    ///   3. Every frame, call SceneFader.Update(dt).
    ///   4. Before loading a new scene, call SceneFader.StartFadeOut("scene path")
    ///      — the scene will load automatically once the fade completes.
    /// </summary>
    public static class SceneFader
    {
        // ── Configuration ──────────────────────────────────────────
        public static float FadeDuration = 1.0f;   // seconds

        // ── Internal state ─────────────────────────────────────────
        private static Image2DComponent overlay;
        private static float alpha;        // current overlay alpha
        private static float timer;        // elapsed time in current fade
        private static bool  fadingIn;     // true = going transparent
        private static bool  fadingOut;    // true = going opaque
        private static string pendingScene;
        private static Action pendingCallback;

        /// <summary>True while any fade animation is running.</summary>
        public static bool IsActive => fadingIn || fadingOut;

        // ── Public API ─────────────────────────────────────────────

        /// <summary>
        /// Bind the overlay and set it to fully opaque (black screen).
        /// Call this in Awake() or Start() before StartFadeIn().
        /// </summary>
        public static void Init(Image2DComponent img)
        {
            overlay = img;
            alpha   = 1.0f;      // start fully black
            timer   = 0.0f;
            fadingIn  = false;
            fadingOut = false;
            pendingScene    = null;
            pendingCallback = null;
            ApplyAlpha(1.0f);
        }

        /// <summary>Begin fading FROM black TO transparent.</summary>
        public static void StartFadeIn()
        {
            if (overlay == null) return;
            alpha     = 1.0f;
            timer     = 0.0f;
            fadingIn  = true;
            fadingOut = false;
            ApplyAlpha(1.0f);
        }

        /// <summary>
        /// Begin fading FROM transparent TO black, then load the target scene.
        /// </summary>
        public static void StartFadeOut(string targetScene)
        {
            if (overlay == null)
            {
                // No overlay — just load immediately
                Scene.loadScene(targetScene);
                return;
            }
            alpha          = 0.0f;
            timer          = 0.0f;
            fadingOut       = true;
            fadingIn        = false;
            pendingScene    = targetScene;
            pendingCallback = null;
            ApplyAlpha(0.0f);
        }

        /// <summary>
        /// Begin fading FROM transparent TO black, then invoke a callback.
        /// </summary>
        public static void StartFadeOut(Action onComplete)
        {
            if (overlay == null)
            {
                // No overlay — invoke immediately
                onComplete?.Invoke();
                return;
            }
            alpha          = 0.0f;
            timer          = 0.0f;
            fadingOut       = true;
            fadingIn        = false;
            pendingScene    = null;
            pendingCallback = onComplete;
            ApplyAlpha(0.0f);
        }

        /// <summary>
        /// Call this every frame from the hosting script's Update().
        /// </summary>
        public static void Update(double dt)
        {
            if (overlay == null) return;

            if (fadingIn)
            {
                timer += (float)dt;
                alpha  = 1.0f - Math.Min(timer / FadeDuration, 1.0f);
                ApplyAlpha(alpha);

                if (alpha <= 0.0f)
                {
                    fadingIn = false;
                    ApplyAlpha(0.0f);
                }
            }
            else if (fadingOut)
            {
                timer += (float)dt;
                alpha  = Math.Min(timer / FadeDuration, 1.0f);
                ApplyAlpha(alpha);

                if (alpha >= 1.0f)
                {
                    fadingOut = false;
                    ApplyAlpha(1.0f);

                    // Perform the deferred action
                    if (pendingScene != null)
                    {
                        string scene = pendingScene;
                        pendingScene = null;
                        Scene.loadScene(scene);
                    }
                    else if (pendingCallback != null)
                    {
                        Action cb = pendingCallback;
                        pendingCallback = null;
                        cb.Invoke();
                    }
                }
            }
        }

        // ── Internals ──────────────────────────────────────────────

        private static void ApplyAlpha(float a)
        {
            if (overlay == null) return;
            var d = overlay.Data;
            d.color.a = a;
            overlay.Data = d;
        }
    }
}
