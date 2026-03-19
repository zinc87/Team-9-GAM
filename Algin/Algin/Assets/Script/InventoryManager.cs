using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Script.Library;

namespace Script
{

    public enum ItemType
    {
        Scanner,
        Syringe,
        Syringe_Filled,
        Cup,
        Cup_Filled,
        Test_Tube
    }

    public class InventoryManager : Script.Library.IScript
    {
        public static InventoryManager Instance { get; private set; }
        [SerializeField] public Script.Library.Object onHandGO;

        [SerializeField] public Script.Library.Object UIScanner;
        [SerializeField] public Script.Library.Object UISyringe;
        [SerializeField] public Script.Library.Object UICup;
        [SerializeField] public Script.Library.Object UITest;

        private List<ItemType> itemOrder = 
            new List<ItemType> { ItemType.Scanner, ItemType.Syringe, ItemType.Cup, ItemType.Test_Tube };
        private int currentIndex = 2;
        public ItemType CurrentItem => itemOrder[currentIndex];
        private string sceneNameAtAwake = "";

        public Illness syringe_record;
        public bool syringe_active;
        public Illness cup_record;
        public bool cup_active;

        public AudioInstance Scanning;
        private AudioInstance whooshAudio;

        public ItemType GetCurrentObject()
        {
            if (CurrentItem == ItemType.Syringe)
            {
                if (syringe_active)
                    return ItemType.Syringe_Filled;
            }
            else if (CurrentItem == ItemType.Cup)
            {
                if (cup_active)
                    return ItemType.Cup_Filled;
            }
            return CurrentItem;
        }

        public void ResetSyringe()
        {
            syringe_active = false;
        }

        public Illness GetSyringeData()
        {
            return syringe_record;
        }

        public void SetSyringeData(Illness data)
        {
            syringe_record = data;
        }

        public void ResetCup()
        {
            cup_active = false;
        }

        public Illness GetCupData()
        {
            return cup_record;
        }

        public void SetCupData(Illness data)
        {
            cup_record = data;
        }
        public void ChangeItem(int direction)
        {
            currentIndex += direction;

            if (currentIndex < 0)
                currentIndex = itemOrder.Count - 1;
            else if (currentIndex >= itemOrder.Count)
                currentIndex = 0;
            Logger.log(Logger.LogLevel.Warning, $"{CurrentItem}");
            if (whooshAudio != null)
                Audio.playAudio(whooshAudio);
            ChangeModel();
            UpdateToolUI();   
        }

        public void ChangeModel()
        {
            ItemType onHand = GetCurrentObject();
            if (onHandGO == null)
            {
                Logger.log(Logger.LogLevel.Error, "onHandGO is null!");
                return;
            }

            MeshRendererComponent mesh = onHandGO.getComponent<MeshRendererComponent>();

            if (mesh == null)
            {
                Logger.log(Logger.LogLevel.Error, "MeshRendererComponent not found!");
                return;
            }
            switch (onHand)
            {
                case ItemType.Scanner:
                    {
                        TransformComponent trfComp = onHandGO.getComponent<TransformComponent>();

                        var tr = trfComp.Transformation;
                        tr.position = new Vector3D(2.5f, -1.2f, -3f);
                        tr.Rotation = new Vector3D(0f,0f,0f);
                        tr.Scale = new Vector3D(0.05f,0.05f,0.05f);

                        trfComp.Transformation = tr;   
                        mesh.SetMesh("Scanner1.agstaticmesh");
                        mesh.SetMeshRendererSubmeshMaterial("Mesh_mat0", "Scanner_MAT.agmat");
                    }
                    break;
                case ItemType.Syringe:
                    {
                        TransformComponent trfComp = onHandGO.getComponent<TransformComponent>();

                        var tr = trfComp.Transformation;
                        tr.position = new Vector3D(1.5f, -0.6f, -2.1f);
                        tr.Rotation = new Vector3D(117f,0f,17f);
                        tr.Scale = new Vector3D(0.05f,0.05f,0.05f);

                        trfComp.Transformation = tr;   
                        mesh.SetMesh("Syringe_updated.agstaticmesh");
                        mesh.SetMeshRendererSubmeshMaterial("Syringe_mat0", "Syringe_MATTERIAL.agmat");
                    }
                    break;
                case ItemType.Cup:
                    {
                        TransformComponent trfComp = onHandGO.getComponent<TransformComponent>();

                        var tr = trfComp.Transformation;
                        tr.position = new Vector3D(1.4f, -0.7f, -1.8f);
                        tr.Rotation = new Vector3D(0f,0f,0f);
                        tr.Scale = new Vector3D(0.04f,0.04f,0.04f);

                        trfComp.Transformation = tr;   
                        mesh.SetMesh("Container.agstaticmesh");
                        mesh.SetMeshRendererSubmeshMaterial("Container_mat0", "Urine_Cup_MATTERIAL.agmat");
                    }
                    break;
                case ItemType.Test_Tube:
                    {
                        TransformComponent trfComp = onHandGO.getComponent<TransformComponent>();

                        var tr = trfComp.Transformation;
                        tr.position = new Vector3D(1.1f, -0.6f, -1.5f);
                        tr.Rotation = new Vector3D(4.4f,18f,-25.6f);
                        tr.Scale = new Vector3D(0.04f,0.04f,0.04f);

                        trfComp.Transformation = tr;   
                        mesh.SetMesh("Skin_Test_Tube.agstaticmesh");
                        mesh.SetMeshRendererSubmeshMaterial("SkinTube_mat1", "TestTubeMATTEst.agmat");
                    }
                    break;
            }

        }

        public void UpdateToolUI()
        {
            // Hide all
            if (UIScanner != null) UIScanner.setActive(false);
            if (UISyringe != null) UISyringe.setActive(false);
            if (UICup != null) UICup.setActive(false);
            if (UITest != null) UITest.setActive(false);

            // Show current
            switch (CurrentItem)
            {
                case ItemType.Scanner:
                    if (UIScanner != null) UIScanner.setActive(true);
                    break;
                case ItemType.Syringe:
                    if (UISyringe != null) UISyringe.setActive(true);
                    break;
                case ItemType.Cup:
                    if (UICup != null) UICup.setActive(true);
                    break;
                case ItemType.Test_Tube:
                    if (UITest != null) UITest.setActive(true);
                    break;
            }
        }

        public override void Awake()
        {
            if (Instance != null && Instance != this)
            {
                string currentScene = Scene.getSceneName();
                if (Instance.sceneNameAtAwake != currentScene)
                {
                    Logger.log(Logger.LogLevel.Warning,
                        $"[InventoryManager] Replacing stale Instance from '{Instance.sceneNameAtAwake}' with new Instance in '{currentScene}'.");
                }
                else
                {
                    Logger.log(Logger.LogLevel.Error,
                        $"DUPLICATE InventoryManager found! Keeping existing Instance={Instance.GetHashCode()}, destroying/ignoring this one={this.GetHashCode()}");

                    // Disable this script (use whatever your engine supports)
                    // If you have a way to destroy the object/script, do that instead.
                    return;
                }
            }

            Instance = this;
            sceneNameAtAwake = Scene.getSceneName();
            Scan.setScan(false);
            Logger.log(Logger.LogLevel.Info,
                $"InventoryManager Instance set: {this.GetHashCode()}");
        }

        public override void Start() { 
            Scanning = Audio.getAudioInstance("Scanning", Obj);
            if (Scanning != null) Audio.stopAudio(Scanning);
            whooshAudio = Audio.getAudioInstance("whooshCartoon CTE02_89.4", Obj);
            Audio.setAudioInstanceVolume(whooshAudio, 0.1f);

            UpdateToolUI();
        }

        public override void Update(double dt)
        {
            /* 
                holding syring, raycast to player , get blood sample
                public struct PatientRecord 
                if holding syringe and raycast npc, extract PatientRecord from PatientManager (current NPC) -> for centrifuge
            */

            // changing 
            Vector2D input;
            Input.mouseScroll(out input);
            if (input.y > 0)
            {
                ChangeItem(1);
            }
            else if (input.y < 0)
            {
                ChangeItem(-1);
            }

            // apply raycast on NPC
            if (CurrentItem == ItemType.Syringe)
            {
                /*
                    syringe_record.Name = ...
                */
            }
            else if (CurrentItem == ItemType.Cup)
            {
                /*
                    cup_record.Name = ...
                */
            }
            else if (CurrentItem == ItemType.Scanner)
            {
                if (Input.isMousePressed(Input.Mouse.kMOUSE_BUTTON_LEFT))
                {
                    Scan.setScan(true);
                    if (Scanning != null) {
                        Audio.playAudio(Scanning);
                        TutorialTextMgr.Instance.triggar(TutorialTextMgr.promptState.Inspect_Organ);
                    }
                }
                else
                {
                    Scan.setScan(false);
                    if (Scanning != null) Audio.stopAudio(Scanning);
                }
            }

            // apply ray
        }

        public override void LateUpdate(double dt) { }
        public override void Free()
        {
            if (Instance == this)
            {
                Instance = null;
            }
            if (Scanning != null)
            {
                Audio.stopAudio(Scanning);
            }
            Scan.setScan(false);
        }


    }
}
