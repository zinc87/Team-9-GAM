using Script.Library;
using PrivateAPI;
using System;

namespace Script
{
    public class SettingsPanelMgr : IScript
    {
        // SINGLETON INSTANCE for easy access from other scripts
        public static SettingsPanelMgr Instance { get; private set; }

        // ======================
        // SETTINGS PAGE TEXTURES
        // ======================
        [SerializeField] public Image2D settingsBgTexture; 
        [SerializeField] public Image2D sliderKnobTexture; 
        [SerializeField] public Image2D sliderTrackTexture; 
        
        [SerializeField] public Image2D backNormal;
        [SerializeField] public Image2D backHover;

        // ======================
        // SETTINGS UI OBJECTS
        // ======================
        [SerializeField] public Script.Library.Object settingsPanelBg; 
        [SerializeField] public Script.Library.Object backButton; 

        // SOUND SLIDER
        [SerializeField] public Script.Library.Object soundSliderTrack; 
        [SerializeField] public Script.Library.Object soundSliderKnob;
        [SerializeField] public Script.Library.Object soundPercentText;

        // GAMMA SLIDER
        [SerializeField] public Script.Library.Object gammaSliderTrack; 
        [SerializeField] public Script.Library.Object gammaSliderKnob;
        [SerializeField] public Script.Library.Object gammaPercentText;

        public bool IsOpen { get; private set; } = false;

        // Mouse Dragging States
        private bool wasLeftHeld = false;
        private bool isDraggingSound = false;
        private bool isDraggingGamma = false;

        // STATIC MEMORY: Survives scene loads
        private static float SavedGamma = -1f;
        private static float SavedVolume = -1f;

        public override void Awake() 
        {
            Instance = this;
        }

        public override void Start()
        {
            // Apply settings textures
            ApplyTexture(settingsPanelBg, settingsBgTexture);
            ApplyTexture(soundSliderKnob, sliderKnobTexture);
            ApplyTexture(gammaSliderKnob, sliderKnobTexture);
            ApplyTexture(backButton, backNormal);
            ApplyTexture(soundSliderTrack, sliderTrackTexture);
            ApplyTexture(gammaSliderTrack, sliderTrackTexture);

            // 1. VOLUME
            if (SavedVolume < 0f) 
            {
                SavedVolume = Audio.getMasterVolume();
            }
            Audio.setMasterVolume(SavedVolume);
            SetSliderValue(soundSliderTrack, soundSliderKnob, soundPercentText, SavedVolume, true);

            // 2. GAMMA
            if (SavedGamma < 0f)
            {
                SavedGamma = Graphics.Gamma;
                
                // If it hasn't been set yet (or is basically 0), force default to 1.20
                if (SavedGamma <= 0.01f) 
                {
                    SavedGamma = 1.20f;
                }
            }
            Graphics.Gamma = SavedGamma;

            // Convert max 2.20 gamma back down to a percentage (0.0 to 1.0) for the physical slider position
            float gammaPct = SavedGamma / 2.2f;
            SetSliderValue(gammaSliderTrack, gammaSliderKnob, gammaPercentText, gammaPct, false);

            CloseSettings();
        }

        public override void Update(double dt)
        {
            // Do not process anything if closed
            if (!IsOpen) 
            {
                wasLeftHeld = Input.isMousePressed(Input.Mouse.kMOUSE_BUTTON_LEFT);
                return;
            }

            Vector2D mp;
            Input.getCurMosusePos(out mp);
            
            bool leftHeld = Input.isMousePressed(Input.Mouse.kMOUSE_BUTTON_LEFT);
            bool leftClicked = leftHeld && !wasLeftHeld; 

            HandleHover(backButton, mp, backNormal, backHover);

            if (Input.isKeyClicked(Input.KeyCode.kESCAPE) || (leftClicked && backButton != null && IsOver(backButton, mp)))
            {
                CloseSettings();
                isDraggingSound = false;
                isDraggingGamma = false;
                wasLeftHeld = leftHeld;
                return;
            }

            if (!leftHeld)
            {
                isDraggingSound = false;
                isDraggingGamma = false;
            }
            else if (leftClicked)
            {
                if (IsOver(soundSliderKnob, mp) || IsOver(soundSliderTrack, mp))
                {
                    isDraggingSound = true;
                    UpdateSlider(soundSliderTrack, soundSliderKnob, soundPercentText, mp, true);
                }
                else if (IsOver(gammaSliderKnob, mp) || IsOver(gammaSliderTrack, mp))
                {
                    isDraggingGamma = true;
                    UpdateSlider(gammaSliderTrack, gammaSliderKnob, gammaPercentText, mp, false);
                }
            }

            if (isDraggingSound) UpdateSlider(soundSliderTrack, soundSliderKnob, soundPercentText, mp, true);
            if (isDraggingGamma) UpdateSlider(gammaSliderTrack, gammaSliderKnob, gammaPercentText, mp, false);

            wasLeftHeld = leftHeld; 
        }

        public void OpenSettings()
        {
            IsOpen = true;
            SetAlpha(settingsPanelBg, 1f);
            SetAlpha(backButton, 1f);
            SetAlpha(soundSliderTrack, 1f);
            SetAlpha(soundSliderKnob, 1f);
            SetAlpha(soundPercentText, 1f);
            SetAlpha(gammaSliderTrack, 1f);
            SetAlpha(gammaSliderKnob, 1f);
            SetAlpha(gammaPercentText, 1f);
        }

        public void CloseSettings()
        {
            IsOpen = false;
            SetAlpha(settingsPanelBg, 0f);
            SetAlpha(backButton, 0f);
            SetAlpha(soundSliderTrack, 0f);
            SetAlpha(soundSliderKnob, 0f);
            SetAlpha(soundPercentText, 0f);
            SetAlpha(gammaSliderTrack, 0f);
            SetAlpha(gammaSliderKnob, 0f);
            SetAlpha(gammaPercentText, 0f);
        }

        // --- SLIDER LOGIC ---
        private void UpdateSlider(Script.Library.Object track, Script.Library.Object knob, Script.Library.Object textObj, Vector2D mp, bool isSoundSlider)
        {
            if (track == null || knob == null) return;
            var trackRt = track.getComponent<RectTransformComponent>();
            var knobRt = knob.getComponent<RectTransformComponent>();
            if (trackRt == null || knobRt == null) return;

            float trackMinX = trackRt.Data.position.x - (trackRt.Data.scale.x * 0.5f);
            float trackMaxX = trackRt.Data.position.x + (trackRt.Data.scale.x * 0.5f);

            float newX = MathPlus.Clamp(mp.x, trackMinX, trackMaxX);

            var kData = knobRt.Data;
            kData.position.x = newX;
            knobRt.Data = kData;

            // percentage of the physical slider track (0.0 to 1.0)
            float pct = (newX - trackMinX) / (trackMaxX - trackMinX);

            // Display percentage for BOTH sliders
            int percentVal = (int)(pct * 100f);
            SetText(textObj, percentVal.ToString() + "%");

            if (isSoundSlider)
            {
                SavedVolume = pct;
                Audio.setMasterVolume(SavedVolume);
            }
            else
            {
                // Multiply percentage by 2.2 to get the actual gamma value for the engine
                SavedGamma = pct * 2.2f;
                Graphics.Gamma = SavedGamma;
            }
        }

        private void SetSliderValue(Script.Library.Object track, Script.Library.Object knob, Script.Library.Object textObj, float value, bool isSoundSlider)
        {
            if (track == null || knob == null) return;
            var trackRt = track.getComponent<RectTransformComponent>();
            var knobRt = knob.getComponent<RectTransformComponent>();
            if (trackRt == null || knobRt == null) return;

            value = MathPlus.Clamp(value, 0f, 1f);
            float trackMinX = trackRt.Data.position.x - (trackRt.Data.scale.x * 0.5f);
            float trackMaxX = trackRt.Data.position.x + (trackRt.Data.scale.x * 0.5f);
            float newX = trackMinX + (value * (trackMaxX - trackMinX));

            var kData = knobRt.Data;
            kData.position.x = newX;
            knobRt.Data = kData;

            // Display percentage for BOTH sliders
            int percentVal = (int)(value * 100f);
            SetText(textObj, percentVal.ToString() + "%");
        }

        // --- ENGINE COMPONENT HELPERS ---
        private void SetAlpha(Script.Library.Object obj, float alpha)
        {
            if (obj == null) return;
            var img = obj.getComponent<Image2DComponent>();
            if (img != null)
            {
                var d = img.Data;
                d.color.a = alpha;
                if (alpha > 0f) { d.color.r = 1f; d.color.g = 1f; d.color.b = 1f; }
                img.Data = d;
            }
            var txt = obj.getComponent<TextMeshUIComponent>();
            if (txt != null) { var d = txt.Data; d.color.a = alpha; txt.Data = d; }
        }

        private void SetText(Script.Library.Object obj, string text)
        {
            if (obj == null) return;
            var txt = obj.getComponent<TextMeshUIComponent>();
            if (txt != null) { var d = txt.Data; d.text = text; txt.Data = d; }
        }

        private void ApplyTexture(Script.Library.Object obj, Image2D img)
        {
            if (obj == null || img == null) return;
            var comp = obj.getComponent<Image2DComponent>();
            if (comp == null) return;
            var d = comp.Data;
            d.hashed = img.hashedID;
            comp.Data = d;
        }

        private void HandleHover(Script.Library.Object obj, Vector2D mp, Image2D normal, Image2D hover)
        {
            if (IsOver(obj, mp)) ApplyTexture(obj, hover);
            else ApplyTexture(obj, normal);
        }

        private bool IsOver(Script.Library.Object obj, Vector2D mp)
        {
            if (obj == null) return false;
            var rt = obj.getComponent<RectTransformComponent>();
            if (rt == null) return false;
            var d = rt.Data;
            float hx = d.scale.x * 0.5f; float hy = d.scale.y * 0.5f;
            Vector2D tl = new Vector2D(d.position.x - hx, d.position.y + hy);
            Vector2D br = new Vector2D(d.position.x + hx, d.position.y - hy);
            return Utility.AABB(tl, br, mp);
        }

        public override void LateUpdate(double dt) {}
        public override void Free() { Instance = null; }
    }
}