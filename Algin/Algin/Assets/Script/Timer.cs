using System;
using Script.Library;

namespace Script
{
    public class TimerManager : IScript
    {
        // =========================================================
        // UI SETTINGS
        // =========================================================
        [SerializeField] public Script.Library.Object timerTextObject;

        [SerializeField] public bool startOnAwake = false;
        [SerializeField] public float initialTime = 420.0f; // seconds

        // =========================================================
        // SINGLETON LOGIC
        // =========================================================
        private static TimerManager instance;
        public static TimerManager getInstance()
        {
            return instance;
        }

        // =========================================================
        // TIMER STATE
        // =========================================================
        private float timeLeft = 0.0f;    // seconds remaining
        private bool running = false;
        private bool paused = false;

        public Action OnTimerEnd;

        public TimerManager() : base("") { }

        private bool prevJDown = false;

        // =========================================================
        // TICKING SOUND
        // =========================================================
        private AudioInstance tickingSound;
        [SerializeField] public float tickingThreshold = 10.0f; // seconds before end to start ticking
        private bool tickingStarted = false;

        public override void Awake()
        {
            instance = this;
        }

        public override void Start()
        {
            tickingSound = Audio.getAudioInstance("ClockTicking_Loop", Obj);

            if (startOnAwake)
            {
                StartTimer(initialTime);
            }
            else
            {
                timeLeft = initialTime;
                RefreshUI();
            }
        }

        public override void Update(double dt)
        {
            if (!running || paused) return;

            bool jDown = Input.isKeyPressed(Input.KeyCode.kJ);

            if (jDown && !prevJDown)
            {
                float changedTime = 3.0f;
                StartTimer(changedTime);
                timeLeft = changedTime;
            }

            prevJDown = jDown;

            // Subtract elapsed time
            timeLeft -= (float)dt;

            // Clamp
            if (timeLeft <= 0.0f)
            {
                timeLeft = 0.0f;
                running = false;
                paused = false;

                // Stop ticking when timer ends
                if (tickingSound != null && tickingStarted)
                {
                    Audio.stopAudio(tickingSound);
                    tickingStarted = false;
                }

                RefreshUI();

                if (OnTimerEnd != null)
                {
                    try
                    {
                        OnTimerEnd.Invoke();
                    }
                    catch (Exception)
                    {
                    }
                }
                return;
            }

            // Start ticking sound when time is low
            if (tickingSound != null)
            {
                if (timeLeft <= tickingThreshold && timeLeft > 0.0f && !tickingStarted)
                {
                    Audio.playAudio(tickingSound);
                    tickingStarted = true;
                }
            }

            // Update visual each frame 
            RefreshUI();
        }

        public override void LateUpdate(double dt) { }

        // =========================================================
        // PUBLIC API
        // =========================================================

        // Start (or restart) the timer with a given duration (seconds)
        public void StartTimer(float durationSeconds)
        {
            timeLeft = Math.Max(0.0f, durationSeconds);
            running = true;
            paused = false;
            tickingStarted = false;
            if (tickingSound != null) Audio.stopAudio(tickingSound);
            RefreshUI();
        }

        // Start timer using the stored initialTime
        public void StartTimer()
        {
            StartTimer(initialTime);
        }

        // Pause the timer (keeps timeLeft unchanged)
        public void Pause()
        {
            if (!running) return;
            paused = true;
        }

        // Resume if paused
        public void Resume()
        {
            if (!running) return;
            paused = false;
        }

        // Stop timer and reset to zero (or to initialTime via overload)
        public void Stop(bool resetToInitial = false)
        {
            running = false;
            paused = false;
            timeLeft = resetToInitial ? initialTime : 0.0f;

            if (tickingSound != null && tickingStarted)
            {
                Audio.stopAudio(tickingSound);
                tickingStarted = false;
            }

            RefreshUI();
        }

        // Add or subtract time while running or stopped
        public void AddTime(float seconds)
        {
            timeLeft += seconds;
            if (timeLeft < 0.0f) timeLeft = 0.0f;
            RefreshUI();
        }

        // Get remaining time in seconds
        public float GetTimeLeft()
        {
            return timeLeft;
        }

        // Is the timer currently counting down?
        public bool IsRunning()
        {
            return running && !paused;
        }

        // =========================================================
        // UI HELPER
        // =========================================================
        private void RefreshUI()
        {
            if (timerTextObject == null) return;

            var txt = timerTextObject.getComponent<TextMeshUIComponent>();
            if (txt != null)
            {
                var d = txt.Data;
                d.text = FormatTime(timeLeft);
                txt.Data = d;
            }
        }

        // Formats seconds as "MM:SS"
        private string FormatTime(float seconds)
        {
            if (seconds < 0) seconds = 0;
            int whole = (int)Math.Floor(seconds);
            int mins = whole / 60;
            int secs = whole % 60;

            return string.Format("{0:00}:{1:00}", mins, secs);
        }

        public override void Free()
        {
            instance = null;
            OnTimerEnd = null;
        }
    }
}