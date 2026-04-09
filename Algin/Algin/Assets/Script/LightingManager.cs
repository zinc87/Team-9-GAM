using Script.Library;
using System;
using System.Collections.Generic;

namespace Script
{
    public class LightingManager : Script.Library.IScript
    {
        public static LightingManager Instance { get; private set; }

        [SerializeField] public Script.Library.Object Light1;
        [SerializeField] public Script.Library.Object Light2;
        [SerializeField] public Script.Library.Object Light3;
        [SerializeField] public Script.Library.Object SkyboxEntity;
        
        private float _spotlightDuration = 0f;
        private float _spotlightDelay = 0f;
        private bool _isSpotlightActive = false;

        private float _baseLight1 = -1f;
        private float _baseLight2 = -1f;
        private float _baseLight3 = -1f;
        private float _baseSkybox = -1f;

        public override void Awake()
        {
            Instance = this;
        }

        public override void Start() { }

        public override void Update(double dt)
        {
            if (_spotlightDelay > 0f)
            {
                _spotlightDelay -= (float)dt;
                if (_spotlightDelay <= 0f)
                {
                    _spotlightDelay = 0f;
                    ActivateSpotlight();
                }
            }
            else if (_isSpotlightActive && _spotlightDuration > 0f)
            {
                _spotlightDuration -= (float)dt;
                if (_spotlightDuration <= 0f)
                {
                    _spotlightDuration = 0f;
                    RestoreLighting();
                }
            }
        }

        public override void LateUpdate(double dt) { }

        public override void Free()
        {
            Instance = null;
        }

        public void TriggerSpotlight(float duration, float delay = 0f)
        {
            _spotlightDuration = duration;
            _spotlightDelay = delay;

            if (delay <= 0f)
            {
                ActivateSpotlight();
            }
        }

        private void DimLight(Script.Library.Object lightObj, ref float baseIntensityCache)
        {
            if (lightObj != null && lightObj.ObjectID != "0000" && lightObj.hasComponent<LightComponent>())
            {
                var lightCmp = lightObj.getComponent<LightComponent>();
                if (lightCmp != null)
                {
                    var data = lightCmp.Data;
                    // Cache the original baseline once
                    if (baseIntensityCache < 0f) baseIntensityCache = data.intensity;
                    
                    data.intensity = 0.0f; // Drop to flat zero for darkness override
                    lightCmp.Data = data;
                }
            }
        }

        private void RestoreLight(Script.Library.Object lightObj, float baseIntensityCache)
        {
            if (lightObj != null && lightObj.ObjectID != "0000" && baseIntensityCache >= 0f && lightObj.hasComponent<LightComponent>())
            {
                var lightCmp = lightObj.getComponent<LightComponent>();
                if (lightCmp != null)
                {
                    var data = lightCmp.Data;
                    data.intensity = baseIntensityCache; // Restore
                    lightCmp.Data = data;
                }
            }
        }

        private void ActivateSpotlight()
        {
            _isSpotlightActive = true;

            DimLight(Light1, ref _baseLight1);
            DimLight(Light2, ref _baseLight2);
            DimLight(Light3, ref _baseLight3);

            if (SkyboxEntity != null && SkyboxEntity.ObjectID != "0000" && SkyboxEntity.hasComponent<SkyBoxComponent>())
            {
                var skyCmp = SkyboxEntity.getComponent<SkyBoxComponent>();
                if (skyCmp != null)
                {
                    var data = skyCmp.Data;
                    if (_baseSkybox < 0f) _baseSkybox = data.exposure;
                    data.exposure = _baseSkybox * 0.05f; // Drop skybox to 5%
                    skyCmp.Data = data;
                }
            }
            
            Logger.log(Logger.LogLevel.Info, "[LightingManager] Spotlight active! Environment dimmed.");
        }

        private void RestoreLighting()
        {
            _isSpotlightActive = false;

            RestoreLight(Light1, _baseLight1);
            RestoreLight(Light2, _baseLight2);
            RestoreLight(Light3, _baseLight3);

            if (SkyboxEntity != null && SkyboxEntity.ObjectID != "0000" && _baseSkybox >= 0f && SkyboxEntity.hasComponent<SkyBoxComponent>())
            {
                var skyCmp = SkyboxEntity.getComponent<SkyBoxComponent>();
                if (skyCmp != null)
                {
                    var data = skyCmp.Data;
                    data.exposure = _baseSkybox; // Restore
                    skyCmp.Data = data;
                }
            }

            Logger.log(Logger.LogLevel.Info, "[LightingManager] Spotlight ended. Environment restored.");
        }
    }
}
