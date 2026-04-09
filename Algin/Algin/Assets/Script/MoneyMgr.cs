using Script.Library;

namespace Script
{
    public class MoneyManager : IScript
    {
        // =========================================================
        // UI SETTINGS
        // =========================================================
        [SerializeField] public Object moneyTextObject; 

        // =========================================================
        // SINGLETON LOGIC
        // =========================================================
        private static MoneyManager instance;
        private float MoneyCount = 0.0f;

        public static MoneyManager getInstance()
        {
            return instance;
        }

        public MoneyManager() : base("") { }

        public override void Awake()
        {
            instance = this;
        }

        public override void Start()
        {
            RefreshUI();
        }

        public override void Update(double dt)
        {
            // NEW: Check if any UI is open using the global GameState
            bool shouldShow = !GameState.IsUIOpen;

            // Show text if no UI is open (alpha 1), otherwise hide it (alpha 0)
            SetAlpha(moneyTextObject, shouldShow ? 1.0f : 0.0f);
        }

        // =========================================================
        // MONEY LOGIC
        // =========================================================
        public void addMoney(float amount)
        {
            MoneyCount += amount;
            RefreshUI(); 
        }

        public void deductMoney(float amount)
        {
            MoneyCount -= amount;
            RefreshUI(); 
        }

        public float getMoneyAmount()
        {
            return MoneyCount;
        }

        public bool canAfford(float amount)
        {
            return MoneyCount >= amount;
        }

        // =========================================================
        // UI HELPERS
        // =========================================================
        private void RefreshUI()
        {
            if (moneyTextObject == null) return;

            var txt = moneyTextObject.getComponent<TextMeshUIComponent>();
            if (txt != null)
            {   
                //float moneyToGet = GameStateManager.getInstance().magicWinningNumber;
                var d = txt.Data;
                d.text = "$" + MoneyCount.ToString("F0"); 
                txt.Data = d;
            }
        }

        private void SetAlpha(Object obj, float a)
        {
            if (obj == null) return;
            
            // Hide Text
            var txt = obj.getComponent<TextMeshUIComponent>();
            if (txt != null) 
            { 
                var d = txt.Data; 
                d.color.a = a; 
                txt.Data = d; 
            }

            // Hide Background Image (if you added one to the money object)
            var img = obj.getComponent<Image2DComponent>();
            if (img != null) 
            { 
                var d = img.Data; 
                d.color.a = a; 
                img.Data = d; 
            }
        }
        
        public override void LateUpdate(double dt) { }
        public override void Free() { instance = null; }
    }
}