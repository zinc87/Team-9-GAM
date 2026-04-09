using Script.Library;
using System.Collections.Generic;
using System.ComponentModel;
using System.Net;

namespace Script
{
    public class TutorialTextMgrScript : IScript
    {

        [SerializeField] public Image2D page1;
        [SerializeField] public Image2D page2;
        [SerializeField] public Image2D page3;
        [SerializeField] public Image2D page4;
        [SerializeField] public Image2D page5;
        [SerializeField] public Image2D page6;
        [SerializeField] public Image2D page7;
        [SerializeField] public Image2D page8;
        [SerializeField] public Image2D page9;
        [SerializeField] public Image2D page10;
        [SerializeField] public Image2D page11;
        [SerializeField] public Image2D page12;

        public override void Awake()
        {
            TutorialTextMgr.Instance.initialize(TutorialTextMgr.promptState.Navigate_To_Computer, page1);
            TutorialTextMgr.Instance.initialize(TutorialTextMgr.promptState.Interact_With_Computer, page2);
            //TutorialTextMgr.Instance.initialize(TutorialTextMgr.promptState.Select_Patient, page3);
            TutorialTextMgr.Instance.initialize(TutorialTextMgr.promptState.Furthur_Testing, page4);
            TutorialTextMgr.Instance.initialize(TutorialTextMgr.promptState.Change_To_Scanner, page5);
            TutorialTextMgr.Instance.initialize(TutorialTextMgr.promptState.Scan_Patient, page6);
            TutorialTextMgr.Instance.initialize(TutorialTextMgr.promptState.Inspect_Organ, page7);
            TutorialTextMgr.Instance.initialize(TutorialTextMgr.promptState.Change_To_Cup, page8);
            TutorialTextMgr.Instance.initialize(TutorialTextMgr.promptState.Navigate_To_Lab_Rm, page9);
            TutorialTextMgr.Instance.initialize(TutorialTextMgr.promptState.Sample_To_Machine, page10);
            TutorialTextMgr.Instance.initialize(TutorialTextMgr.promptState.Navigate_Back_To_Computer, page11);
            TutorialTextMgr.Instance.initialize(TutorialTextMgr.promptState.Diagnosis, page12);
        }
        public override void Start()
        {
            TutorialTextMgr.Instance.reset();
        }

        public override void Update(double dt)
        {
            showText();
        }

        public override void LateUpdate(double dt) {}
        public override void Free() {}

        public void showText()
        {
            TutorialTextMgr.promptState currentState = TutorialTextMgr.Instance.currentState;
            Dictionary<TutorialTextMgr.promptState, Image2D> stateToImage = TutorialTextMgr.Instance.stateToImage;

            if (currentState == TutorialTextMgr.promptState.End)
            {
                Utility.setAlpha(this.Obj, 0.0f);
            }
            else
            {
                Utility.setAlpha(this.Obj, 1f);
            
                Image2DComponent img = this.Obj.getComponent<Image2DComponent>();
                if (img != null)
                {
                    Image2DComponentData d = img.Data;
                    if (stateToImage[currentState].hashedID == 0)
                    {
                        Utility.setAlpha(this.Obj, 0.0f);
                    }
                    else if (d.hashed != stateToImage[currentState].hashedID) 
                    { 
                        d.hashed = stateToImage[currentState].hashedID;
                        img.Data = d; 
                    }
                }
            }
        }
    }

    public class TutorialTextMgr
    {
        public void initialize(promptState state, Image2D image)
        {
            stateToImage[state] = image;
        }
        
        public void triggar(promptState nextState)
        {   
            if (currentState == (promptState)((int)nextState - 1))
            {
                currentState = nextState;
            }
        }

        public void reset()
        {
            currentState = promptState.Navigate_To_Computer;
        }

        public enum promptState
        {
            Navigate_To_Computer, //Default
            Interact_With_Computer, //When look at the computer (PlayerController.cs)
            //Select_Patient, // When interacted with the computer(PlayerController.cs) (Not Active)
            Furthur_Testing, //When Select patient (PatientUIPanel.cs)
            Change_To_Scanner, //When pressed further testing btn(PatientPanelMgr.cs)
            Scan_Patient, //When choose the scanner (InventoryManager.cs)
            Inspect_Organ, //When Using the scanner (InventoryManager.cs)
            Change_To_Cup, //When reaching page 3 of info (TutorialMgr.cs)
            Navigate_To_Lab_Rm, //When Urine sample collected (PlayerController.cs)
            Sample_To_Machine, //When reaching the machine (PlayerController.cs)
            Navigate_Back_To_Computer, //When sending sample into the machine (PlayerController.cs)
            Diagnosis,
            End
        }
        public promptState currentState {get; private set;}
        public Dictionary<promptState, Image2D> stateToImage {get; private set;}

         
        private static TutorialTextMgr instance;
        private TutorialTextMgr() { 
            currentState = promptState.Navigate_To_Computer;
            stateToImage = new Dictionary<promptState, Image2D>();
        }
        public static TutorialTextMgr Instance
        {
            get
            {
                if (instance == null)
                    instance = new TutorialTextMgr();

                return instance;
            }
        }
    }

}
