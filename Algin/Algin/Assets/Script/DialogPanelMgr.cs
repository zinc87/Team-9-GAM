using Script.Library;

namespace Script
{
    public class DialogPanelMgr : IScript
    {
        // PANEL + BUTTON TEXTURES (ASSIGN IN INSPECTOR)
        [SerializeField] public Image2D panelBGImage;

        [SerializeField] public Image2D yesButtonImage;
        [SerializeField] public Image2D yesButtonHoverImage;

        [SerializeField] public Image2D noButtonImage;
        [SerializeField] public Image2D noButtonHoverImage;

        [SerializeField] public Image2D closeButtonImage;
        [SerializeField] public Image2D closeButtonHoverImage;

        // UI OBJECTS (ASSIGN IN INSPECTOR)
        [SerializeField] public Object vapingPanel;
        [SerializeField] public Object vapingButtonDetain;   // Yes
        [SerializeField] public Object vapingButtonClear;    // No
        [SerializeField] public Object vapingButtonClose;    // X

        // INTERNAL STATE
        private bool visible = false;
        private StudentScript currentStudent = null;

        public static DialogPanelMgr Instance { get; private set; }

        //-------------------------------------------------------
        public override void Awake()
        {
            Instance = this;
        }

        //-------------------------------------------------------
        public override void Start()
        {
            // Apply initial textures
            ApplyTexture(vapingPanel, panelBGImage);
            ApplyTexture(vapingButtonDetain, yesButtonImage);
            ApplyTexture(vapingButtonClear, noButtonImage);
            ApplyTexture(vapingButtonClose, closeButtonImage);

            // Hide everything
            SetAlpha(0f);
            visible = false;
        }

        //-------------------------------------------------------
        // CALLED BY GAMECONTROLLER
        public void ShowVapingPanel(StudentScript student)
        {
            currentStudent = student;
            visible = true;

            SetAlpha(1f);
            Input.enableMouse();
        }

        //-------------------------------------------------------
        private void HidePopup()
        {
            visible = false;
            currentStudent = null;

            SetAlpha(0f);
            Input.disableMouse();
        }

        //-------------------------------------------------------
        public override void Update(double dt)
        {
            if (!visible)
                return;

            Vector2D mp;
            Input.getCurMosusePos(out mp);

            // --- HOVER EFFECTS ---
            HandleHover(vapingButtonDetain, mp, yesButtonImage, yesButtonHoverImage);
            HandleHover(vapingButtonClear, mp, noButtonImage, noButtonHoverImage);
            HandleHover(vapingButtonClose, mp, closeButtonImage, closeButtonHoverImage);

            // --- CLICK ACTIONS ---
            if (Input.isMousePressed(Input.Mouse.kMOUSE_BUTTON_LEFT))
            {
                // DETAIN (YES)
                if (IsOver(vapingButtonDetain, mp))
                {
                    if (currentStudent != null) {
                        currentStudent.SendToArrest();
                        WinLose.detainCount++;

                        if(!currentStudent.isVaping) {
                            WinLose.wrongCount++;
                        } else {
                            WinLose.correctCount++;
                        }
                    }

                    HidePopup();
                    return;
                }

                // CLEAR (NO)
                if (IsOver(vapingButtonClear, mp))
                {
                    if (currentStudent != null) {
                        currentStudent.SendToFree();
                        WinLose.detainCount++;

                        if(currentStudent.isVaping) {
                            WinLose.wrongCount++;
                        } else {
                            WinLose.correctCount++;
                        }
                    }

                    HidePopup();
                    return;
                }

                // CLOSE (X)
                if (IsOver(vapingButtonClose, mp))
                {
                    HidePopup();
                    return;
                }
            }
        }

        //-------------------------------------------------------
        // HELPERS
        private void ApplyTexture(Object obj, Image2D img)
        {
            if (obj == null || img == null) return;

            var comp = obj.getComponent<Image2DComponent>();
            if (comp == null) return;

            var d = comp.Data;
            d.hashed = img.hashedID;
            comp.Data = d;
        }

        //-------------------------------------------------------
        private void SetAlpha(float a)
        {
            ApplyAlpha(vapingPanel, a);
            ApplyAlpha(vapingButtonDetain, a);
            ApplyAlpha(vapingButtonClear, a);
            ApplyAlpha(vapingButtonClose, a);
        }

        //-------------------------------------------------------
        private void ApplyAlpha(Object obj, float a)
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

        //-------------------------------------------------------
        private void HandleHover(Object obj, Vector2D mp,
                                 Image2D normal, Image2D hover)
        {
            if (IsOver(obj, mp))
                ApplyTexture(obj, hover);
            else
                ApplyTexture(obj, normal);
        }

        //-------------------------------------------------------
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

        //-------------------------------------------------------
        public override void LateUpdate(double dt) {}
        public override void Free() { Instance = null; }
    }
}
