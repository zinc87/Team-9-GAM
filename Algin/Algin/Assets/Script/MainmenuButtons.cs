using Script.Library;
using PrivateAPI;
using System;

namespace Script
{
    public class MainMenuButtons : IScript
    {
        // ======================
        // MAIN MENU TEXTURES
        // ======================
        [SerializeField] public Image2D startNormal;
        [SerializeField] public Image2D startHover;

        [SerializeField] public Image2D settingsNormal;
        [SerializeField] public Image2D settingsHover;

        [SerializeField] public Image2D quitNormal;
        [SerializeField] public Image2D quitHover;

        // ======================
        // MAIN MENU UI OBJECTS
        // ======================
        [SerializeField] public Script.Library.Object startButton;
        [SerializeField] public Script.Library.Object settingsButton;
        [SerializeField] public Script.Library.Object quitButton;

        public override void Start()
        {
            // Force the engine to unlock the mouse cursor when the menu loads
            GameState.SetUIState(true);

            ApplyTexture(startButton, startNormal);
            ApplyTexture(settingsButton, settingsNormal);
            ApplyTexture(quitButton, quitNormal);
        }

        public override void Update(double dt)
        {
            // Do NOT allow clicking/hovering main menu buttons if settings are open
            if (SettingsPanelMgr.Instance != null && SettingsPanelMgr.Instance.IsOpen)
                return;

            Vector2D mp;
            Input.getCurMosusePos(out mp);
            bool leftClicked = Input.isMousePressed(Input.Mouse.kMOUSE_BUTTON_LEFT);

            HandleHover(startButton, mp, startNormal, startHover);
            HandleHover(settingsButton, mp, settingsNormal, settingsHover);
            HandleHover(quitButton, mp, quitNormal, quitHover);

            if (leftClicked)
            {
                if (IsOver(startButton, mp))
                {
                    Scene.loadScene("Assets\\Scenes\\Video Player.scene");
                    return;
                }
                if (IsOver(settingsButton, mp))
                {
                    // Trigger the Settings script to open!
                    if (SettingsPanelMgr.Instance != null)
                    {
                        SettingsPanelMgr.Instance.OpenSettings();
                    }
                    return;
                }
                if (IsOver(quitButton, mp))
                {
                    Environment.Exit(0);
                    return;
                }
            }
        }

        // --- ENGINE COMPONENT HELPERS ---
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

        public override void Awake() {}
        public override void LateUpdate(double dt) {}
        public override void Free() {}
    }
}