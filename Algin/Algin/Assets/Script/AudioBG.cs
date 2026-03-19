using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Script.Library;

namespace Script
{
    public class AudioBackGround : Script.Library.IScript
    {

        public AudioInstance AmbienceBg;

        public override void Awake()
        {
        }
        public override void Start()
        {
            AmbienceBg = Audio.getAudioInstance("Ambience_Office_Loop", Obj);            
            Audio.playAudio(AmbienceBg);
            Audio.setAudioInstanceVolume(AmbienceBg, 0.2f); 
        }
        public override void Update(double dt)
        {
           
        }
        public override void LateUpdate(double dt) { }
        public override void Free() {
            Audio.stopAudio(AmbienceBg);
        }
    }
}
