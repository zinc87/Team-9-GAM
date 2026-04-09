using Script.Library;

namespace Script
{
    public class ObjectivesMgr : IScript
    {
        public static ObjectivesMgr Instance { get; private set; }

        // ===========================================
        // INSPECTOR VARIABLES
        // ===========================================
        [SerializeField] public Object objectivesPanel;
        [SerializeField] public Object objectivesText1; 
        [SerializeField] public Object objectivesText2;

        private string defaultText1 = "Current Objective";
        private string defaultText2 = "Search and scan the students.";

        public override void Awake()
        {
            Instance = this;
        }

        public override void Start()
        {
            // Set Text
            UpdateObjectives(defaultText1, defaultText2);
            
            // Show immediately at start
            Show(true);
        }

        public override void Update(double dt) { }

        public void UpdateObjectives(string text1, string text2)
        {
            SetText(objectivesText1, text1);
            SetText(objectivesText2, text2);
        }

        // VISIBILITY TOGGLE
        public void Show(bool visible)
        {
            float alpha = visible ? 1f : 0f;
            ApplyAlpha(objectivesPanel, alpha);
            ApplyAlpha(objectivesText1, alpha);
            ApplyAlpha(objectivesText2, alpha);
        }

        // ===========================================
        // HELPERS
        // ===========================================
        private void SetText(Object obj, string text)
        {
            if (obj == null) return;
            var t = obj.getComponent<TextMeshUIComponent>();
            if (t == null) return;
            var d = t.Data;
            d.text = text;
            t.Data = d;
        }

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

        public override void LateUpdate(double dt) { }
        public override void Free() { Instance = null; }
    }
}