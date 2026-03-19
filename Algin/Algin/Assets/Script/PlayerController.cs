using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Script.Library;

namespace Script
{
    public class PlayerController : Script.Library.IScript
    {
        [SerializeField] public float moveSpeed;
        [SerializeField] public float lookSpeed;

        [SerializeField] public Script.Library.Object pressEtoInteract;
        [SerializeField] public Script.Library.Object invalidSampleobj;
        [SerializeField] public Script.Library.Object SampleDepositedObj;
        [SerializeField] public Script.Library.Object BloodSampleCollectedObj;
        [SerializeField] public Script.Library.Object UrineSampleCollectedObj;
        [SerializeField] public Script.Library.Object HairSampleCollectedObj;

        public Script.Library.Object Sample;

        private float timer = 0.0f;
        private bool active = false;
        private float currentAlpha = 0.0f;
        private bool wasMoving = false;
        private float stepTimer = 0f;
        private float stepInterval = 0.5f;

        public AudioInstance invalidDepositSound;
        public AudioInstance validDepositSound;
        public AudioInstance sampleCollectSound;

        private float _invalidSoundTimer = -1f;
        private float _validSoundTimer = -1f;
        private float _collectSoundTimer = -1f;

        [SerializeField] public float fadeSpeed;
        [SerializeField] public float displayDuration;
        [SerializeField] public float fadeRatio;

        // VIP Overlay Variables
        [SerializeField] public Script.Library.Object vipOverlayObj;
        private bool _isVipOverlayActive = false;
        private float _vipOverlayTimer = 0f;
        private float _vipOverlayCurrentAlpha = 0f;
        [SerializeField] public float vipOverlayDisplayDuration = 4.0f;
        public bool HasTriggeredVipThisLevel = false;

        public AudioInstance footsteps;

        public bool eWasDown = false;

        public string currentPatientNPC = "";

        public static PlayerController Instance { get; private set; }

        // --- SHAKE EFFECT VARIABLES ---
        private float _shakeDuration = 0f;
        private float _shakeMagnitude = 0f;
        private float _shakeDelay = 0f;
        private float _shakeOffsetX = 0f;
        private float _shakeOffsetY = 0f;
        private Random _rng = new Random();

        public override void Awake()
        {
            Instance = this;
        }

        public override void Start()
        {
            // Reset all game state when game scene loads
            GameState.ResetAll();
            HasTriggeredVipThisLevel = false;

            footsteps = Audio.getAudioInstance("Footsteps-Carpet-11", Obj);
            invalidDepositSound = Audio.getAudioInstance("User Interface, Alert, Notification, Negative, Error, Incorrect, Insufficient, Video Game 04 SND86849", Obj);
            validDepositSound = Audio.getAudioInstance("User Interface, Alert, Alert, System SND30974 1", Obj);
            sampleCollectSound = Audio.getAudioInstance("User Interface, Alert, Alert Tone, Light SND37234", Obj);
            stepTimer = stepInterval;
            ApplyAlpha(pressEtoInteract, 0.0f);
            ApplyAlpha(invalidSampleobj, 0.0f);
            ApplyAlpha(SampleDepositedObj, 0.0f);
            ApplyAlpha(BloodSampleCollectedObj, 0.0f);
            ApplyAlpha(UrineSampleCollectedObj, 0.0f);
            ApplyAlpha(HairSampleCollectedObj, 0.0f);
            ApplyAlpha(vipOverlayObj, 0.0f);
        }

        public override void Update(double dt)
        {
            var transform = Obj.getComponent<TransformComponent>();

            // --- Reverse previous shake before we take input ---
            if (transform != null)
            {
                var trf = transform.Transformation;
                trf.Rotation.x -= _shakeOffsetX;
                trf.Rotation.y -= _shakeOffsetY;
                transform.Transformation = trf;
            }

            // Proceed with normal updates only if input is allowed
            if (GameState.IsInputAllowed)
            {
                UpdatePatientRay();

                // --- Movement & Look ---
                if (transform != null)
                {
                    updateMouseLook(transform);
                    updateMovement(transform, (float)dt);
                }

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

                        if (Sample != BloodSampleCollectedObj)
                            ApplyAlpha(BloodSampleCollectedObj, 0.0f);
                        if (Sample != UrineSampleCollectedObj)
                            ApplyAlpha(UrineSampleCollectedObj, 0.0f);
                        if (Sample != HairSampleCollectedObj)
                            ApplyAlpha(HairSampleCollectedObj, 0.0f);

                        if (ObjectivesMgr.Instance != null)
                            ObjectivesMgr.Instance.Show(true);
                    }

                    ApplyAlpha(Sample, currentAlpha);
                }
            }

            // --- Sound duration limiter (2 seconds) ---
            if (_invalidSoundTimer >= 0f)
            {
                _invalidSoundTimer += (float)dt;
                if (_invalidSoundTimer >= 2f)
                {
                    Audio.stopAudio(invalidDepositSound);
                    _invalidSoundTimer = -1f;
                }
            }
            if (_validSoundTimer >= 0f)
            {
                _validSoundTimer += (float)dt;
                if (_validSoundTimer >= 2f)
                {
                    Audio.stopAudio(validDepositSound);
                    _validSoundTimer = -1f;
                }
            }

            if (_collectSoundTimer >= 0f)
            {
                _collectSoundTimer += (float)dt;
                if (_collectSoundTimer >= 2f)
                {
                    Audio.stopAudio(sampleCollectSound);
                    _collectSoundTimer = -1f;
                }
            }

            // --- Apply new shake at the very end of the frame ---
            if (transform != null)
            {
                UpdateShake(transform, dt);
            }

            // --- VIP Overlay Fade Logic ---
            if (_isVipOverlayActive)
            {
                _vipOverlayTimer += (float)dt;

                float waitTime = 1.0f;
                float fadeInTime = 0.5f;

                if (_vipOverlayTimer < waitTime)
                {
                    _vipOverlayCurrentAlpha = 0.0f;
                }
                else if (_vipOverlayTimer < waitTime + fadeInTime)
                {
                    float fadeT = (_vipOverlayTimer - waitTime) / fadeInTime;
                    _vipOverlayCurrentAlpha = fadeT;
                }
                else
                {
                    float timeAfterFadeIn = _vipOverlayTimer - (waitTime + fadeInTime);
                    float remainingDuration = vipOverlayDisplayDuration - (waitTime + fadeInTime);
                    if (remainingDuration <= 0) remainingDuration = 1.0f;

                    float t = timeAfterFadeIn / remainingDuration; // 0 -> 1

                    if (t < 1.0f - fadeRatio)
                    {
                        // HOLD phase
                        _vipOverlayCurrentAlpha = 1.0f;
                    }
                    else if (t < 1.0f)
                    {
                        // FADE phase
                        float fadeT = (t - (1.0f - fadeRatio)) / fadeRatio;
                        _vipOverlayCurrentAlpha = 1.0f - fadeT;
                    }
                    else
                    {
                        // DONE
                        _vipOverlayCurrentAlpha = 0.0f;
                        _isVipOverlayActive = false;
                    }
                }
                ApplyAlpha(vipOverlayObj, _vipOverlayCurrentAlpha);
            }
        }

        public override void LateUpdate(double dt) { }
        public override void Free()
        {
            Instance = null;
        }

        private bool _hasShaken = false;

        public void TriggerShake(float duration, float magnitude, float delay = 0f)
        {
            if (_hasShaken) return;
            _hasShaken = true;

            _shakeDuration = duration;
            _shakeMagnitude = magnitude * 0.5f;
            _shakeDelay = delay;
        }

        public void TriggerVipOverlay()
        {
            HasTriggeredVipThisLevel = true;
            _isVipOverlayActive = true;
            _vipOverlayTimer = 0.0f;
            _vipOverlayCurrentAlpha = 0.0f;
            ApplyAlpha(vipOverlayObj, _vipOverlayCurrentAlpha);
        }

        private void UpdateShake(TransformComponent xformCmp, double dt)
        {
            if (_shakeDelay > 0f)
            {
                _shakeDelay -= (float)dt;
                _shakeOffsetX = 0f;
                _shakeOffsetY = 0f;
            }
            else if (_shakeDuration > 0f)
            {
                _shakeDuration -= (float)dt;

                _shakeOffsetX = (float)(_rng.NextDouble() * 2.0 - 1.0) * _shakeMagnitude;
                _shakeOffsetY = (float)(_rng.NextDouble() * 2.0 - 1.0) * _shakeMagnitude;

                _shakeMagnitude = Math.Max(0f, _shakeMagnitude - ((float)dt * 0.2f));
            }
            else
            {
                _shakeDuration = 0f;
                _shakeOffsetX = 0f;
                _shakeOffsetY = 0f;
            }

            var trf = xformCmp.Transformation;
            trf.Rotation.x += _shakeOffsetX;
            trf.Rotation.y += _shakeOffsetY;
            xformCmp.Transformation = trf;
        }

        // --- NEW: Immediately hides the previous sample and starts the new one ---
        private void StartNewSample(Script.Library.Object newSample)
        {
            if (Sample != null && Sample != newSample)
            {
                ApplyAlpha(Sample, 0.0f);
            }

            Sample = newSample;
            active = true;
            timer = 0.0f;
            currentAlpha = 1.0f;
            ApplyAlpha(Sample, 1.0f);
        }

        public void updateMovement(TransformComponent xformCmp, float dt)
        {
            Vector3D dir = new Vector3D(0, 0, 0);
            var trf = xformCmp.Transformation;

            if (Input.isKeyPressed(Input.KeyCode.kW)) dir += trf.getFlatForward();
            if (Input.isKeyPressed(Input.KeyCode.kS)) dir += trf.getFlatBackward();
            if (Input.isKeyPressed(Input.KeyCode.kD)) dir += trf.getFlatRight();
            if (Input.isKeyPressed(Input.KeyCode.kA)) dir += trf.getFlatLeft();

            bool isMoving = dir.magnitude() > 0.0f;
            if (isMoving)
            {
                dir = dir.normalize();
            }

            var rbCmp = Obj.getComponent<RigidBodyComponent>();
            if (rbCmp == null) return;

            var rb = rbCmp.Data;

            if (isMoving)
            {
                rb.velocity.x = dir.x * moveSpeed;
                rb.velocity.y = 0.0f;
                rb.velocity.z = dir.z * moveSpeed;
            }
            else
            {
                rb.velocity = new Vector3D(0, 0, 0);
            }

            if (footsteps != null)
            {
                if (isMoving)
                {
                    stepTimer += dt;
                    if (stepTimer >= stepInterval)
                    {
                        Audio.stopAudio(footsteps);
                        Audio.playAudio(footsteps);
                        stepTimer = 0f;
                    }
                }
                else
                {
                    stepTimer = 0f;
                }
            }

            wasMoving = isMoving;

            rbCmp.Data = rb;
        }

        public void updateMouseLook(TransformComponent xformCmp)
        {
            Input.getMouseDelta(out Vector2D mouseDelta);
            var trf = xformCmp.Transformation;

            trf.Rotation.y += mouseDelta.x * lookSpeed;
            trf.Rotation.x += mouseDelta.y * lookSpeed;
            trf.Rotation.x = MathPlus.Clamp(trf.Rotation.x, -89f, 89f);

            xformCmp.Transformation = trf;
        }

        private void UpdatePatientRay()
        {
            bool eDown = Input.isKeyPressed(Input.KeyCode.kE);
            bool ePressedOnce = eDown && !eWasDown;
            eWasDown = eDown;
            GameState.SetCanOpenPatientUI(false);

            CameraRay ray = Camera.getCameraRay(this.Obj);

            DebugRenderer.drawLine(ray.origin, ray.origin + ray.direction * 100.0f);

            bool hitSomething = Physics.Raycast(ray.origin, ray.direction, out RaycastHitData hit);

            if (hitSomething && hit.layer == CollisionLayers.Layer_Computer)
            {
                ApplyAlpha(pressEtoInteract, 1.0f);
                GameState.SetCanOpenPatientUI(true);
                TutorialTextMgr.Instance.triggar(TutorialTextMgr.promptState.Interact_With_Computer);
                TutorialTextMgr.Instance.triggar(TutorialTextMgr.promptState.Diagnosis);

                if (ePressedOnce)
                {
                    TutorialTextMgr.Instance.triggar(TutorialTextMgr.promptState.Select_Patient);
                    if (PatientUIPanel.Instance.IsOpen)
                    {
                        PatientUIPanel.Instance.CloseAll();
                        GameState.SetCanOpenPatientUI(false);
                    }
                    else
                    {
                        PatientUIPanel.Instance.OpenList();
                        GameState.SetCanOpenPatientUI(true);
                    }
                }
            }
            else if (hitSomething && hit.layer == CollisionLayers.Layer_NPC)
            {
                ApplyAlpha(pressEtoInteract, 1.0f);
                if (ePressedOnce)
                {
                    currentPatientNPC = hit.obj.ObjectID;
                    PatientManager.TryGetPatientData(currentPatientNPC, out PatientData Pdata);
                    Logger.log(Logger.LogLevel.Warning, $"Interacted With {Pdata.PatientName}");

                    if (InventoryManager.Instance.CurrentItem == ItemType.Syringe)
                    {
                        Logger.log(Logger.LogLevel.Warning, $"{Pdata.PatientName}'s Blood Sample Collected");
                        var labs = Pdata.Labs;
                        labs.bloodSampleCollected = true;
                        Pdata.Labs = labs;
                        PatientManager.SetPatientData(currentPatientNPC, Pdata);

                        StartNewSample(BloodSampleCollectedObj);
                        PlayCollectSound();
                    }
                    else if (InventoryManager.Instance.CurrentItem == ItemType.Cup)
                    {
                        Logger.log(Logger.LogLevel.Warning, $"{Pdata.PatientName}'s Urine Sample Collected");
                        TutorialTextMgr.Instance.triggar(TutorialTextMgr.promptState.Navigate_To_Machine_Rm);

                        var labs = Pdata.Labs;
                        labs.urineSampleCollected = true;
                        Pdata.Labs = labs;
                        PatientManager.SetPatientData(currentPatientNPC, Pdata);

                        StartNewSample(UrineSampleCollectedObj);
                        PlayCollectSound();
                    }
                    else if (InventoryManager.Instance.CurrentItem == ItemType.Test_Tube)
                    {
                        Logger.log(Logger.LogLevel.Warning, $"{Pdata.PatientName}'s Hair Sample Collected");
                        var labs = Pdata.Labs;
                        labs.hairSampleCollected = true;
                        Pdata.Labs = labs;
                        PatientManager.SetPatientData(currentPatientNPC, Pdata);

                        StartNewSample(HairSampleCollectedObj);
                        PlayCollectSound();
                    }
                }
            }
            else if (hitSomething && hit.layer == CollisionLayers.Layer_Centrifuge)
            {
                ApplyAlpha(pressEtoInteract, 1.0f);
                if (InventoryManager.Instance.CurrentItem == ItemType.Cup ||
                    InventoryManager.Instance.CurrentItem == ItemType.Test_Tube)
                {
                    if (ePressedOnce)
                    {
                        StartNewSample(invalidSampleobj);
                        PlayInvalidSound();
                    }
                }
                if (InventoryManager.Instance.CurrentItem == ItemType.Syringe)
                {
                    if (ePressedOnce)
                    {
                        if (currentPatientNPC == "")
                        {
                            Logger.log(Logger.LogLevel.Warning, "No Blood Sample Collected");
                            return;
                        }
                        PatientManager.TryGetPatientData(currentPatientNPC, out PatientData Pdata);

                        if (Pdata.Labs.bloodSampleCollected)
                        {
                            StartNewSample(SampleDepositedObj);
                            PlayValidSound();

                            var labs = Pdata.Labs;
                            labs.bloodSampleDeposited = true;
                            Pdata.Labs = labs;
                            PatientManager.SetPatientData(currentPatientNPC, Pdata);
                        }
                        else
                        {
                            Logger.log(Logger.LogLevel.Warning, $"{Pdata.PatientName}'s Blood Sample Not Collected");
                        }
                    }
                }
            }
            else if (hitSomething && hit.layer == CollisionLayers.Layer_SkinTest)
            {
                ApplyAlpha(pressEtoInteract, 1.0f);
                if (InventoryManager.Instance.CurrentItem == ItemType.Cup ||
                    InventoryManager.Instance.CurrentItem == ItemType.Syringe)
                {
                    if (ePressedOnce)
                    {
                        StartNewSample(invalidSampleobj);
                        PlayInvalidSound();
                    }
                }
                if (InventoryManager.Instance.CurrentItem == ItemType.Test_Tube)
                {
                    if (ePressedOnce)
                    {
                        if (currentPatientNPC == "")
                        {
                            Logger.log(Logger.LogLevel.Warning, "No Hair Sample Collected");
                            return;
                        }
                        PatientManager.TryGetPatientData(currentPatientNPC, out PatientData Pdata);

                        if (Pdata.Labs.hairSampleCollected)
                        {
                            StartNewSample(SampleDepositedObj);
                            PlayValidSound();

                            var labs = Pdata.Labs;
                            labs.hairSampleDeposited = true;
                            Pdata.Labs = labs;
                            PatientManager.SetPatientData(currentPatientNPC, Pdata);
                        }
                        else
                        {
                            Logger.log(Logger.LogLevel.Warning, $"{Pdata.PatientName}'s Hair Sample Not Collected");
                        }
                    }
                }
            }
            else if (hitSomething && hit.layer == CollisionLayers.Layer_UrineTest)
            {
                TutorialTextMgr.Instance.triggar(TutorialTextMgr.promptState.Sample_To_Machine);
                ApplyAlpha(pressEtoInteract, 1.0f);
                if (InventoryManager.Instance.CurrentItem == ItemType.Syringe ||
                    InventoryManager.Instance.CurrentItem == ItemType.Test_Tube)
                {
                    if (ePressedOnce)
                    {
                        StartNewSample(invalidSampleobj);
                        PlayInvalidSound();
                    }
                }
                if (InventoryManager.Instance.CurrentItem == ItemType.Cup)
                {
                    if (ePressedOnce)
                    {
                        if (currentPatientNPC == "")
                        {
                            Logger.log(Logger.LogLevel.Warning, "No Urine Sample Collected");
                            return;
                        }
                        PatientManager.TryGetPatientData(currentPatientNPC, out PatientData Pdata);

                        if (Pdata.Labs.urineSampleCollected)
                        {
                            TutorialTextMgr.Instance.triggar(TutorialTextMgr.promptState.Navigate_Back_To_Computer);
                            StartNewSample(SampleDepositedObj);
                            PlayValidSound();

                            var labs = Pdata.Labs;
                            labs.urineSampleDeposited = true;
                            Pdata.Labs = labs;
                            PatientManager.SetPatientData(currentPatientNPC, Pdata);
                        }
                        else
                        {
                            Logger.log(Logger.LogLevel.Warning, $"{Pdata.PatientName}'s Urine Sample Not Collected");
                        }
                    }
                }
            }
            else
            {
                ApplyAlpha(pressEtoInteract, 0.0f);
                GameState.SetCanOpenPatientUI(false);
            }

            if (PatientUIPanel.Instance.IsOpen)
            {
                ApplyAlpha(pressEtoInteract, 0.0f);
            }
        }

        private void PlayInvalidSound()
        {
            if (invalidDepositSound == null) return;
            Audio.stopAudio(invalidDepositSound);
            Audio.playAudio(invalidDepositSound);
            _invalidSoundTimer = 0f;
        }

        private void PlayValidSound()
        {
            if (validDepositSound == null) return;
            Audio.stopAudio(validDepositSound);
            Audio.playAudio(validDepositSound);
            _validSoundTimer = 0f;
        }

        private void PlayCollectSound()
        {
            if (sampleCollectSound == null) return;
            Audio.stopAudio(sampleCollectSound);
            Audio.playAudio(sampleCollectSound);
            _collectSoundTimer = 0f;
        }

        private void ApplyAlpha(Script.Library.Object o, float a)
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