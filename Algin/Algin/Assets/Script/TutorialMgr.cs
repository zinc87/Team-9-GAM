using Script.Library;

namespace Script
{
    public class TutorialMgr : IScript
    {
        // =========================================================
        // UI OBJECTS
        // =========================================================
        [SerializeField] public Object infoButton; 
        [SerializeField] public Image2D infoNormal; 
        [SerializeField] public Image2D infoHover; 

        [SerializeField] public Object nextButton;
        [SerializeField] public Image2D nextNormal;
        [SerializeField] public Image2D nextHover;

        [SerializeField] public Object backButton;
        [SerializeField] public Image2D backNormal;
        [SerializeField] public Image2D backHover;

        [SerializeField] public Object closeButton;
        [SerializeField] public Image2D closeNormal;
        [SerializeField] public Image2D closeHover;

        [SerializeField] public Object doneButton; 
        [SerializeField] public Image2D doneNormal;
        [SerializeField] public Image2D doneHover;

        [SerializeField] public Object page1;
        [SerializeField] public Object page2;
        [SerializeField] public Object page3;
        [SerializeField] public Object page4;
        [SerializeField] public Object page5;
        [SerializeField] public Object page6;
        [SerializeField] public Object page7;
        [SerializeField] public Object page8;
        [SerializeField] public Object page9;
        [SerializeField] public Object page10;
        [SerializeField] public Object page11;

        private Object[] pagesArray;
        private int currentPageIndex = 0;
        private bool isTutorialOpen = false;
        private bool iWasDown = false;
        private bool mouseWasDown = false; 
        private const int TOTAL_PAGES = 11;

        public static TutorialMgr Instance { get; private set; }

        public TutorialMgr() : base("") { }
        public override void Awake() { Instance = this; }

        public override void Start()
        {
            // array to include all 11 pages
            pagesArray = new Object[] { page1, page2, page3, page4, page5, page6, page7, page8, page9, page10, page11 };
            CloseTutorial();
            Input.disableMouse(); 
        }

        public override void Update(double dt)
        {
            if (GameState.IsGameOver) return;

            Vector2D mp;
            Input.getCurMosusePos(out mp);

            // HOVER LOGIC
            if (!isTutorialOpen)
            {
                UpdateHover(infoButton, infoNormal, infoHover, mp);
            }
            else
            {
                UpdateHover(nextButton, nextNormal, nextHover, mp);
                UpdateHover(backButton, backNormal, backHover, mp);
                UpdateHover(closeButton, closeNormal, closeHover, mp);
                UpdateHover(doneButton, doneNormal, doneHover, mp);
            }

            // KEYBOARD INPUT (I Key)
            bool iDown = Input.isKeyPressed(Input.KeyCode.kI);
            if (iDown && !iWasDown) 
            { 
                if (isTutorialOpen) 
                {
                    CloseTutorial(); 
                }
                else if (GameState.IsInputAllowed || (PauseMenu.Instance != null && PauseMenu.Instance.IsPaused)) 
                {
                    OpenTutorial(); 
                }
            }
            iWasDown = iDown;

            // MOUSE INPUT
            bool isMouseDown = Input.isMousePressed(Input.Mouse.kMOUSE_BUTTON_LEFT);
            if (isMouseDown && !mouseWasDown) 
            {
                HandleMouseClick(mp);
            }
            mouseWasDown = isMouseDown;

            // =========================================================
            //  VISIBILITY LOGIC
            // =========================================================
            bool isOtherUIOpen = false;

            // 1. Check the details panel
            if (PatientPanelMgr.Instance != null && PatientPanelMgr.Instance.state != PatientPanelMgr.ViewState.Closed) 
                isOtherUIOpen = true;

            // 2. Check the Manager (PatientUIPanel)
            if (PatientUIPanel.Instance != null && PatientUIPanel.Instance.IsOpen)
                isOtherUIOpen = true;

            // 3. Check Diagnose Panel
            if (DiagnosePanelMgr.Instance != null && DiagnosePanelMgr.Instance.IsVisible) 
                isOtherUIOpen = true;

            // 4. Check Test Panel
            if (PatientTestPanel.Instance != null && PatientTestPanel.Instance.IsVisible) 
                isOtherUIOpen = true;

            // Logic: 
            // - If Tutorial is open -> Hide 'i'
            // - If ANY Other UI is open -> Hide 'i'
            // - Else -> Show 'i' (This includes Pause Menu, so it stays visible there)
            if (isTutorialOpen || isOtherUIOpen)
            {
                SetVisible(infoButton, false);
            }
            else
            {
                SetVisible(infoButton, true);
            }
        }

        // =========================================================
        //  HELPER FUNCTIONS
        // =========================================================

        private void UpdateHover(Object btn, Image2D normal, Image2D hover, Vector2D mp)
        {
            if (btn == null) return;
            // Only hover if visible (Alpha > 0.1)
            if (GetAlpha(btn) < 0.1f) return;

            if (IsMouseOver(btn, mp)) ApplyTexture(btn, hover);
            else ApplyTexture(btn, normal);
        }

        private void HandleMouseClick(Vector2D mp) 
        { 
            // 1. If Tutorial is CLOSED, check for Info Button click
            if (!isTutorialOpen && IsMouseOver(infoButton, mp)) 
            { 
                // Only click if it's actually visible
                if (GetAlpha(infoButton) > 0.5f) 
                { 
                    OpenTutorial(); 
                    return; 
                } 
            } 
            
            // 2. If Tutorial is OPEN, handle navigation clicks
            if (isTutorialOpen) 
            { 
                if (IsMouseOver(closeButton, mp) || IsMouseOver(doneButton, mp)) 
                { 
                    CloseTutorial(); 
                } 
                else if (IsMouseOver(nextButton, mp)) 
                { 
                    NextPage(); 
                } 
                else if (IsMouseOver(backButton, mp)) 
                { 
                    PrevPage(); 
                } 
            } 
        }

        public void OpenTutorial() 
        { 
            // If Pause Menu is open, force close it
            if (PauseMenu.Instance != null && PauseMenu.Instance.IsPaused)
            {
                PauseMenu.Instance.ShowMenu(false);
            }

            isTutorialOpen = true; 
            currentPageIndex = 0; 
            
            GameState.SetUIState(true); 
            Input.enableMouse(); 
            
            SetVisible(closeButton, true); 
            UpdatePageVisuals(); 
        }

        public void CloseTutorial() 
        { 
            isTutorialOpen = false; 
            GameState.SetUIState(false); 
            Input.disableMouse(); 
            
            if (pagesArray != null) 
            { 
                foreach(var p in pagesArray) SetVisible(p, false); 
            } 
            
            SetVisible(nextButton, false); 
            SetVisible(backButton, false); 
            SetVisible(closeButton, false); 
            SetVisible(doneButton, false); 
            
            SetVisible(infoButton, true); 
        }

        private void NextPage() 
        { 
            if (currentPageIndex < TOTAL_PAGES - 1) 
            { 
                currentPageIndex++;

                if (currentPageIndex == 2) //The normal abnormal lungs page
                {
                    TutorialTextMgr.Instance.triggar(TutorialTextMgr.promptState.Change_To_Needle);
                }

                UpdatePageVisuals(); 
            } 
        }

        private void PrevPage() 
        { 
            if (currentPageIndex > 0) 
            { 
                currentPageIndex--; 
                UpdatePageVisuals(); 
            } 
        }

        private void UpdatePageVisuals() 
        { 
            if (pagesArray == null) return; 
            
            for (int i = 0; i < pagesArray.Length; i++) 
            { 
                SetVisible(pagesArray[i], i == currentPageIndex); 
            } 
            
            SetVisible(backButton, currentPageIndex > 0); 
            SetVisible(nextButton, currentPageIndex < TOTAL_PAGES - 1); 
            SetVisible(doneButton, currentPageIndex == TOTAL_PAGES - 1); 
        }

        private void SetVisible(Object obj, bool visible) 
        { 
            SetAlpha(obj, visible ? 1f : 0f); 
        }

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

        private float GetAlpha(Object obj) 
        { 
            if (obj == null) return 0f; 
            var img = obj.getComponent<Image2DComponent>(); 
            if (img != null) return img.Data.color.a; 
            return 0f; 
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

        private bool IsMouseOver(Object obj, Vector2D mouse) 
        { 
            if (obj == null) return false; 
            
            // Check visibility first
            if (GetAlpha(obj) < 0.1f) return false; 
            
            var rt = obj.getComponent<RectTransformComponent>(); 
            if (rt == null) return false; 
            
            var d = rt.Data; 
            float hx = d.scale.x * 0.5f; 
            float hy = d.scale.y * 0.5f; 
            
            Vector2D tl = new Vector2D(d.position.x - hx, d.position.y + hy); 
            Vector2D br = new Vector2D(d.position.x + hx, d.position.y - hy); 
            
            return Utility.AABB(tl, br, mouse); 
        }

        public override void LateUpdate(double dt) { }
        public override void Free() { Instance = null; }
    }
}