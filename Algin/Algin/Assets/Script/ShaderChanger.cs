using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Script.Library;


namespace Script
{
    public class ShaderChanger : Script.Library.IScript
    {
        
        [SerializeField] public Script.Library.Object NPC1;
        [SerializeField] public Script.Library.Object NPC2;




        public static void DiscardShader(Script.Library.Object NPC_Object)
        {
            SkinnedMeshRenderer skinned = NPC_Object.getComponent<SkinnedMeshRenderer>();
            if (skinned != null)
            {
                skinned.ChangeShaderExcept("SkinnedShader_Discard", "Organ_");
            }
        }

        public static void NormalShader(Script.Library.Object NPC_Object)
        {
            SkinnedMeshRenderer skinned = NPC_Object.getComponent<SkinnedMeshRenderer>();
            if (skinned != null)
            {
                skinned.ChangeShaderExcept("SkinnedShader", "Organ_");
            }


        }
    
        public override void Awake() {}
        public override void LateUpdate(double dt) 
        {

            if (Input.isKeyPressed(Input.KeyCode.kG))
            {
                if ( NPC1 != null)
                {
                    Library.Logger.log(Library.Logger.LogLevel.Info, "Discard: NPC1");
                    DiscardShader(NPC1);
                }
                if ( NPC2 != null)
                {
                    Library.Logger.log(Library.Logger.LogLevel.Info, "Discard: NPC2");
                    DiscardShader(NPC2);
                }
            }

            if (Input.isKeyPressed(Input.KeyCode.kH))
            {
                if ( NPC1 != null)
                {
                    Library.Logger.log(Library.Logger.LogLevel.Info, "Normal: NPC1");
                    NormalShader(NPC1);
                }
                if ( NPC2 != null)
                {
                    Library.Logger.log(Library.Logger.LogLevel.Info, "Normal: NPC2");
                    NormalShader(NPC2);
                }
            }


        }
        public override void Free() { }


        public override void Start() { }

        public override void Update(double dt) {}
    }

}