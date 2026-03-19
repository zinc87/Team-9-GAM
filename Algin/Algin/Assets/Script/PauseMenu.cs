using Script.Library;

namespace Script
{
    public class PauseMenu : IScript
    {
        // ============================
        // UI OBJECTS
        // ============================
        [SerializeField] public Object overlayTint;
        [SerializeField] public Object PauseMenuPanel;     

        [SerializeField] public Object btnResume;
        [SerializeField] public Object btnRestart;
        [SerializeField] public Object btnMainMenu;

        // ============================
        // TEXTURES
        // ============================
        [SerializeField] public Image2D resumeNormal;
        [SerializeField] public Image2D resumeHover;

        [SerializeField] public Image2D restartNormal;
        [SerializeField] public Image2D restartHover;

        [SerializeField] public Image2D mainMenuNormal;
        [SerializeField] public Image2D mainMenuHover;

        private bool isPaused = false;

        public bool IsPaused 
        { 
            get { return isPaused; } 
        }

        private AudioInstance hoverSound;
        private AudioInstance clickSound;
        private Object _lastHoveredButton = null;
        private bool wasLeftHeld = false;

        public static PauseMenu Instance { get; private set; }

        public PauseMenu() : base("") { }
        public PauseMenu(string id) : base(id) { }

        public override void Awake() 
        { 
            Instance = this; 
        }

        public override void Start()
        {
            ShowMenu(false);

            hoverSound = Audio.getAudioInstance("HoverButton", Obj);
            clickSound = Audio.getAudioInstance("ButtonPress_003", Obj);
        }

        public override void Update(double dt)
        {
            if (Input.isKeyClicked(Input.KeyCode.kESCAPE))
            {
                ShowMenu(!isPaused);
            }

            if (!isPaused)
            {
                wasLeftHeld = Input.isMousePressed(Input.Mouse.kMOUSE_BUTTON_LEFT);
                return;
            }

            Vector2D mp;
            Input.getCurMosusePos(out mp);
            bool leftHeld = Input.isMousePressed(Input.Mouse.kMOUSE_BUTTON_LEFT);
            bool leftClicked = leftHeld && !wasLeftHeld;

            UpdateHover(btnResume, resumeNormal, resumeHover, mp);
            UpdateHover(btnRestart, restartNormal, restartHover, mp);
            UpdateHover(btnMainMenu, mainMenuNormal, mainMenuHover, mp);

            Object _currentHovered = null;
            if (IsOver(btnResume, mp)) _currentHovered = btnResume;
            else if (IsOver(btnRestart, mp)) _currentHovered = btnRestart;
            else if (IsOver(btnMainMenu, mp)) _currentHovered = btnMainMenu;

            if (_currentHovered != _lastHoveredButton)
            {
                if (_currentHovered != null && hoverSound != null)
                    Audio.playAudio(hoverSound);
                _lastHoveredButton = _currentHovered;
            }

            if (leftClicked)
            {
                if (clickSound != null) Audio.playAudio(clickSound);

                if (IsOver(btnResume, mp))
                {
                    ShowMenu(false);
                }
                else if (IsOver(btnRestart, mp))
                {
                    ResetAllUI();
                    LevelScoreManager.ResetLevelCounters();
                    if (LevelScoreManager.CurrentLevel == 2)
                    {
                        Scene.loadScene("Assets\\Scenes\\Level_2.scene");
                    }
                    else
                    {
                        Scene.loadScene("Assets\\Scenes\\TempMap - Copy.scene");
                    }
                }
                else if (IsOver(btnMainMenu, mp))
                {
                    ResetAllUI();
                    LevelScoreManager.Reset();
                    Input.enableMouse();
                    Scene.loadScene("Assets\\Scenes\\MainMenu.scene");
                }
            }

            wasLeftHeld = leftHeld;
        }

        // -------------------------

        public void ShowMenu(bool show)
        {
            isPaused = show;
            GameState.SetUIState(show);

            SetAlpha(overlayTint, show ? 0.7f : 0f);

            float uiAlpha = show ? 1f : 0f;
            SetAlpha(PauseMenuPanel, uiAlpha);
            SetAlpha(btnResume, uiAlpha);
            SetAlpha(btnRestart, uiAlpha);
            SetAlpha(btnMainMenu, uiAlpha);
        }

        private void ResetAllUI()
        {
            if (PatientPanelMgr.Instance != null) 
                PatientPanelMgr.Instance.CloseAll();
            
            if (DiagnosePanelMgr.Instance != null) 
                DiagnosePanelMgr.Instance.SetVisible(false);
            
            if (PatientTestPanel.Instance != null) 
                PatientTestPanel.Instance.Hide();
            
            ShowMenu(false);
            GameState.SetUIState(false);
        }

        // ============================
        // HELPERS
        // ============================

        private void SetAlpha(Object obj, float a)
        {
            if (obj == null) return;
            
            var img = obj.getComponent<Image2DComponent>();
            if (img != null)
            {
                var d = img.Data;
                d.color.a = a;
                img.Data = d;
            }
            
            var txt = obj.getComponent<TextMeshUIComponent>();
            if (txt != null)
            {
                var d = txt.Data;
                d.color.a = a;
                txt.Data = d;
            }
        }

        private void UpdateHover(Object btn, Image2D normal, Image2D hover, Vector2D mp)
        {
            if (btn == null) return;
            
            if (IsOver(btn, mp)) 
                ApplyTexture(btn, hover);
            else 
                ApplyTexture(btn, normal);
        }

        private void ApplyTexture(Object obj, Image2D tex)
        {
            if (obj == null || tex == null) return;
            
            var img = obj.getComponent<Image2DComponent>();
            if (img != null)
            {
                var d = img.Data;
                if (d.hashed != tex.hashedID) 
                { 
                    d.hashed = tex.hashedID; 
                    img.Data = d; 
                }
            }
        }

        private bool IsOver(Object obj, Vector2D mp)
        {
            if (obj == null) return false;
            
            var rt = obj.getComponent<RectTransformComponent>();
            if (rt == null) return false;
            
            var d = rt.Data;
            float hx = d.scale.x * 0.5f;
            float hy = d.scale.y * 0.5f;
            
            Vector2D tl = new Vector2D(d.position.x - hx, d.position.y + hy);
            Vector2D br = new Vector2D(d.position.x + hx, d.position.y - hy);
            
            return Utility.AABB(tl, br, mp);
        }

        public override void LateUpdate(double dt) { }
        public override void Free() { Instance = null; }
    }
}
