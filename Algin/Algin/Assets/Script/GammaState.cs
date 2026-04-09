using Script.Library;

namespace Script
{
    public static class GammaState
    {
        public const float DefaultGamma = 1.20f;
        public const float MaxGamma = 2.20f;

        private static float savedGamma = -1f;

        public static void EnsureApplied()
        {
            if (savedGamma < 0f)
            {
                savedGamma = Graphics.Gamma;
                if (savedGamma <= 0.01f)
                {
                    savedGamma = DefaultGamma;
                }
            }

            Graphics.Gamma = savedGamma;
        }

        public static float GetSliderPercent()
        {
            EnsureApplied();
            return Clamp01(savedGamma / MaxGamma);
        }

        public static void SetFromSliderPercent(float percent)
        {
            savedGamma = Clamp01(percent) * MaxGamma;
            Graphics.Gamma = savedGamma;
        }

        private static float Clamp01(float value)
        {
            if (value < 0f) return 0f;
            if (value > 1f) return 1f;
            return value;
        }
    }
}
