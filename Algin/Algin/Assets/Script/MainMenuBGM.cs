using Script.Library;
using PrivateAPI;
using System;

namespace Script
{
    public class MainMenuBGM : IScript
    {

        public AudioInstance menuBGM;

        public override void Start()
        {
            
            menuBGM = Audio.getAudioInstance("MainMenuBGM", Obj);
            Audio.playAudio(menuBGM);

            
        }

        public override void Update(double dt)
        {
            
        }

       
        public override void Awake() { }
        public override void LateUpdate(double dt) { }

        public override void Free()
        {
            Audio.stopAudio(menuBGM);
        }
    }
}
