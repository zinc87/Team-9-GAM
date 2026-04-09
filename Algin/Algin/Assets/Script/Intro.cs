using System;
using Script.Library;
using System.Collections.Generic;

namespace Script
{
    public class IntroUISequence : IScript
    {
        [SerializeField]
        public Library.Object screen1;
        [SerializeField]
        public Library.Object screen2;
        [SerializeField]
        public Library.Object screen3;

        private List<Library.Object> screens = new List<Library.Object>();
        private int currentScreen = 0;

        public override void Start()
        {
            screens.Add(screen1);
            screens.Add(screen2);
            screens.Add(screen3);

            // Hide all first
            foreach (var s in screens)
                SetAlpha(s, 0.0f);

            // Show the first screen only
            if (screens.Count > 0)
                SetAlpha(screens[0], 1.0f);
        }

        public override void Update(double dt)
        {
            // Space advances to next screen
            if (Input.isKeyClicked(Input.KeyCode.kSPACE) || Input.isMouseClicked(Input.Mouse.kMOUSE_BUTTON_LEFT))
            {
                AdvanceScreen();
            }
        }

        private void AdvanceScreen()
        {
            // If no more screens, end intro
            if (currentScreen >= screens.Count)
            {
                Library.Logger.log(
                    Library.Logger.LogLevel.Info,
                    "Intro sequence finished"
                );
                return;
            }

            // Hide current
            SetAlpha(screens[currentScreen], 0.0f);

            // Move to next
            currentScreen++;

            // Show next
            SetAlpha(screens[currentScreen], 1.0f);

            Library.Logger.log(
                Library.Logger.LogLevel.Info,
                $"Showing intro screen #{currentScreen + 1}"
            );
        }

        private void SetAlpha(Library.Object obj, float alpha)
        {
            if (obj == null) return;

            var img = obj.getComponent<Image2DComponent>();
            if (img == null) return;

            Image2DComponentData data = img.Data;
            data.color.a = alpha;
            img.Data = data;
        }

        public override void Awake() { }
        public override void LateUpdate(double dt) { }
        public override void Free() { }
    }
}
