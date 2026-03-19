using Script.Library;
//
namespace Script
{
    public class WinLose : IScript
    {
        [SerializeField] public Object DetainText;
        [SerializeField] public Object CorrectText;
        [SerializeField] public Object WrongText;
        [SerializeField] public Object FinalText;

        public static int detainCount = 0;
        public static int correctCount = 0;
        public static int wrongCount = 0;

        [SerializeField] public Object Panel;
        [SerializeField] public Object MenuButton;
        [SerializeField] public Object RestartButton;
        [SerializeField] public Object QuitButton;

        private static bool isOpen = false;

        // STATIC reference for global call
        private static WinLose instance;

        public override void Awake()
        {
            instance = this;
        }

        public override void Start()
        {
            SetOpen(false);
        }

        // --------------------------------------------------------------------
        // OPEN / CLOSE UI PANEL
        // --------------------------------------------------------------------
        public static void SetOpen(bool state)
        {
            if (instance == null) return;

            float a = state ? 1f : 0f;

            if(state) {
                Input.enableMouse();
                isOpen = state;
            } else {
                Input.disableMouse();
                isOpen = state;
            }

            ApplyAlpha(instance.DetainText, a);
            ApplyAlpha(instance.CorrectText, a);
            ApplyAlpha(instance.WrongText, a);
            ApplyAlpha(instance.FinalText, a);

            ApplyAlpha(instance.Panel, a);
            ApplyAlpha(instance.MenuButton, a);
            //ApplyAlpha(instance.RestartButton, a);
            ApplyAlpha(instance.QuitButton, a);
        }

        // --------------------------------------------------------------------
        // SET ALPHA FOR ANY UI ELEMENT (Image or Text)
        // --------------------------------------------------------------------
        public static void ApplyAlpha(Object obj, float a)
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

        // --------------------------------------------------------------------
        // UPDATE COUNTS ON SCREEN
        // --------------------------------------------------------------------
        public override void Update(double dt)
        {
            UpdateText(DetainText, detainCount);
            UpdateText(CorrectText, correctCount);
            UpdateText(WrongText, wrongCount);
            UpdateText(FinalText, detainCount + correctCount + wrongCount); // optional
        }

        private void UpdateText(Object obj, int value)
        {
            if (obj == null) return;

            var txt = obj.getComponent<TextMeshUIComponent>();
            if (txt == null) return;

            var d = txt.Data;
            d.text = value.ToString();
            txt.Data = d;

            Vector2D mp;
            Input.getCurMosusePos(out mp);

            if (Input.isMousePressed(Input.Mouse.kMOUSE_BUTTON_LEFT) && isOpen)
            {
                if(IsOver(MenuButton, mp)) {
                    GameState.SetGameOver(false);  // Reset game over state before returning to menu
                    Scene.loadScene("Assets\\Scenes\\MainMenu.scene");
                }

                if(IsOver(QuitButton, mp)) {
                    GameState.SetGameOver(false);
                    // Advance level if we are on level 1, so the cutscene logic goes to level 2 next
                    if (LevelScoreManager.CurrentLevel == 1)
                    {
                        LevelScoreManager.AdvanceLevel();
                        Scene.loadScene("Assets/Scenes/Level1_end_cutscene.scene");
                    }
                    else if (LevelScoreManager.CurrentLevel == 2)
                    {
                        LevelScoreManager.AdvanceLevel();
                        Scene.loadScene("Assets/Scenes/End_Cutscene.scene");
                    }
                    else
                    {
                        // Fallback behavior
                        Scene.loadScene("Assets/Scenes/Level_2.scene");
                    }
                }
            }
        }

        private bool IsOver(Object obj, Vector2D mp)
        {
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
        public override void Free() { }
    }
}
