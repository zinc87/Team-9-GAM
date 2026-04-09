// ============================================================================
// LevelScoreManager.cs - Scoring logic with VIP weighting & level advancement
// ============================================================================
// Non-VIP correct = 10 pts, VIP correct = 50 pts (5x multiplier).
// Pass threshold = 60% of max possible score.
// Auto-fail: if penalty bar is maxed (all non-VIP wrong), guaranteed loss
// regardless of VIP result — but game is NOT ended abruptly.
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
        public const float PASS_THRESHOLD = 0.6f;   // 60 %

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
            if (TimeRanOut)
            {
                Logger.log(Logger.LogLevel.Info,
                    "[LevelScoreManager] FAILED — time ran out");
                return false;
            }

            ComputeScore(out int earned, out int max, out float ratio);

            // Guaranteed loss if penalty bar is full
            if (PenaltyBarMgr.Instance != null && PenaltyBarMgr.Instance.IsMaxedOut())
            {
                Logger.log(Logger.LogLevel.Info,
                    "[LevelScoreManager] FAILED — penalty bar maxed out");
                return false;
            }

            bool passed = ratio >= PASS_THRESHOLD;
            Logger.log(Logger.LogLevel.Info,
                $"[LevelScoreManager] Score {earned}/{max} ({ratio:P0}) — " +
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
