using Script.Library;

namespace Script
{
    public class SampleCollected : IScript
    {
        
        [SerializeField] public Object BloodSampleCollectedObj;
        [SerializeField] public Object UrineSampleCollectedObj;
        [SerializeField] public Object HairSampleCollectedObj;


        public Object objOnHand;

        private float currentAlpha = 0.0f;
        private bool mWasDown = false;

        [SerializeField] public float fadeSpeed;
        [SerializeField] public float displayDuration;
        [SerializeField] public float fadeRatio;

        //private bool visible = false;
        private float timer = 0.0f;
        private bool active = false;

        private bool syringeOnHand = false;
        private bool cupOnHand = false;
        private bool testTubeOnHand = false;


        public override void Awake() {}

        public override void Start() 
        {
            ApplyAlpha(BloodSampleCollectedObj, 0.0f);
            ApplyAlpha(UrineSampleCollectedObj, 0.0f);
            ApplyAlpha(HairSampleCollectedObj, 0.0f);
        }

        public override void Update(double dt)
        {   

            if (InventoryManager.Instance != null &&
                InventoryManager.Instance.CurrentItem == ItemType.Syringe)
            {
                syringeOnHand = true;
                cupOnHand = false;
                testTubeOnHand = false;
            }
            else if (InventoryManager.Instance != null && 
            InventoryManager.Instance.CurrentItem == ItemType.Cup)
            {
                syringeOnHand = false;
                cupOnHand = true;
                testTubeOnHand = false;
            }
            else if (InventoryManager.Instance != null &&
            InventoryManager.Instance.CurrentItem == ItemType.Test_Tube)
            {
                syringeOnHand = false;
                cupOnHand = false;
                testTubeOnHand = true;
            }

            bool mDown = Input.isKeyPressed(Input.KeyCode.kM);

            if (syringeOnHand)
                objOnHand = BloodSampleCollectedObj;
            if (cupOnHand)
                objOnHand = UrineSampleCollectedObj;
            if (testTubeOnHand)
                objOnHand = HairSampleCollectedObj;

            if (mDown && !mWasDown)
            {
                active = true;
                timer = 0.0f;

                currentAlpha = 1.0f;
                ApplyAlpha(objOnHand, 1.0f);

                if (ObjectivesMgr.Instance != null)
                    ObjectivesMgr.Instance.Show(false);
            }

            mWasDown = mDown;

            if (active)
            {
                timer += (float)dt;

                float t = timer / displayDuration; // 0 -> 1

                if (t < 1.0f - fadeRatio)
                {
                    // HOLD phase (80%)
                    currentAlpha = 1.0f;
                }
                else if (t < 1.0f)
                {
                    // FADE phase (last 20%)
                    float fadeT = (t - (1.0f - fadeRatio)) / fadeRatio;
                    currentAlpha = 1.0f - fadeT;
                }
                else
                {
                    // DONE
                    currentAlpha = 0.0f;
                    active = false;

                    if (ObjectivesMgr.Instance != null)
                        ObjectivesMgr.Instance.Show(true);
                }

                ApplyAlpha(objOnHand, currentAlpha);
            }
        }


        public override void LateUpdate(double dt) { }
        public override void Free() { }

        private void ApplyAlpha(Object o, float a) 
        { 
            if (o != null) 
            { 
                var i = o.getComponent<Image2DComponent>(); 
                if (i != null) 
                { 
                    var d = i.Data; 
                    d.color.a = a; 
                    i.Data = d; 
                } 
                var t = o.getComponent<TextMeshUIComponent>(); 
                if (t != null) 
                { 
                    var d = t.Data; 
                    d.color.a = a; 
                    t.Data = d; 
                } 
            } 
        }
    }
}
