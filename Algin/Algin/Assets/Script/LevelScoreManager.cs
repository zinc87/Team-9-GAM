// ============================================================================
// LevelScoreManager.cs - Scoring logic with VIP weighting & level advancement
// ============================================================================
// Non-VIP correct = 10 pts, VIP correct = 30 pts (3x multiplier).
// Pass threshold = level-based (L1 60%, L2 50%, default 55%).
// Auto-fail: if penalty bar is maxed (all non-VIP wrong), guaranteed loss
// regardless of VIP result - but game is NOT ended abruptly.
// ============================================================================

using Script.Library;

namespace Script
{
    public static class LevelScoreManager
    {
        // =========================================================
        // SCORING CONSTANTS
        // =========================================================
        public const int NON_VIP_POINTS = 10;
        public const int VIP_POINTS     = 30; // Reduced from 50 to 30

        // Level-based pass thresholds (ratio of earned/max score)
        // Level 1: 6 patients, 4 illnesses -> reasonable target 60%
        // Level 2: 8 patients, 8 illnesses -> more difficult, target 50%
        public const float PASS_THRESHOLD_LEVEL1 = 0.60f;
        public const float PASS_THRESHOLD_LEVEL2 = 0.50f;
        public const float PASS_THRESHOLD_DEFAULT = 0.55f;

        // Optional override for debugging or tuning; set < 0 to use level defaults
        public static float PASS_THRESHOLD_OVERRIDE = -1.0f;

        // =========================================================
        // STATE  (all reset per level)
        // =========================================================
        public static bool TimeRanOut    { get; set; } = false;

        public static int CurrentLevel   { get; private set; } = 1;

        public static int NonVipCorrect  { get; private set; } = 0;
        public static int NonVipTotal    { get; private set; } = 0;

        public static int VipCorrect     { get; private set; } = 0;
        public static int VipTotal       { get; private set; } = 0;

        // =========================================================
        // PUBLIC API
        // =========================================================

        /// <summary>
        /// Record one diagnosis result.  Called from DiagnosePanelMgr
        /// after the player clicks the Diagnose button.
        /// </summary>
        public static void RecordDiagnosis(bool isCorrect, bool isVIP)
        {
            if (isVIP)
            {
                VipTotal++;
                if (isCorrect) VipCorrect++;
            }
            else
            {
                NonVipTotal++;
                if (isCorrect) NonVipCorrect++;
            }

            Logger.log(Logger.LogLevel.Info,
                $"[LevelScoreManager] Recorded: correct={isCorrect}, VIP={isVIP} " +
                $"| NonVIP {NonVipCorrect}/{NonVipTotal}  VIP {VipCorrect}/{VipTotal}");
        }

        /// <summary>
        /// Compute the current level score.
        /// </summary>
        public static void ComputeScore(out int earned, out int max, out float ratio)
        {
            earned = NonVipCorrect * NON_VIP_POINTS + VipCorrect * VIP_POINTS;
            max    = NonVipTotal   * NON_VIP_POINTS + VipTotal   * VIP_POINTS;
            ratio  = (max > 0) ? (float)earned / max : 0f;
        }

        /// <summary>
        /// Determines whether the player has passed the level.
        /// Returns false if:
        ///   1. The score ratio is below the pass threshold, OR
        ///   2. The penalty bar is maxed out (guaranteed loss), OR
        ///   3. Time ran out before diagnosing all patients.
        /// </summary>
        public static bool HasPassed()
        {
            ComputeScore(out int earned, out int max, out float ratio);

            float threshold = GetPassThreshold();

            // Presentation/debug override: if threshold is 0 or less, always pass.
            if (threshold <= 0f)
            {
                Logger.log(Logger.LogLevel.Info,
                    $"[LevelScoreManager] Score {earned}/{max} ({ratio:P0}) - PASSED (threshold override)");
                return true;
            }

            if (TimeRanOut)
            {
                Logger.log(Logger.LogLevel.Info,
                    "[LevelScoreManager] FAILED - time ran out");
                return false;
            }

            // Guaranteed loss if penalty bar is full
            if (PenaltyBarMgr.Instance != null && PenaltyBarMgr.Instance.IsMaxedOut())
            {
                Logger.log(Logger.LogLevel.Info,
                    "[LevelScoreManager] FAILED - penalty bar maxed out");
                return false;
            }

            bool passed = ratio >= threshold;
            Logger.log(Logger.LogLevel.Info,
                $"[LevelScoreManager] Score {earned}/{max} ({ratio:P0}) vs {threshold:P0} - " +
                (passed ? "PASSED" : "FAILED"));
            return passed;
        }

        /// <summary>
        /// Advance to the next level.  Resets per-level counters and penalty bar.
        /// </summary>
        public static void AdvanceLevel()
        {
            CurrentLevel++;
            ResetLevelCounters();
            Logger.log(Logger.LogLevel.Info,
                $"[LevelScoreManager] Advanced to Level {CurrentLevel}");
        }

        /// <summary>
        /// Full reset (new game / return to menu).
        /// </summary>
        public static void Reset()
        {
            CurrentLevel = 1;
            ResetLevelCounters();
        }

        // =========================================================
        // INTERNAL
        // =========================================================
        private static float GetPassThreshold()
        {
            if (PASS_THRESHOLD_OVERRIDE >= 0f)
                return PASS_THRESHOLD_OVERRIDE;

            if (CurrentLevel == 1)
                return PASS_THRESHOLD_LEVEL1;
            if (CurrentLevel == 2)
                return PASS_THRESHOLD_LEVEL2;

            return PASS_THRESHOLD_DEFAULT;
        }

        public static void ResetLevelCounters()
        {
            TimeRanOut    = false;
            NonVipCorrect = 0;
            NonVipTotal   = 0;
            VipCorrect    = 0;
            VipTotal      = 0;

            if (PenaltyBarMgr.Instance != null)
                PenaltyBarMgr.Instance.Reset();
        }
    }
}
