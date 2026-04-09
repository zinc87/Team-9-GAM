// ============================================================================
// PenaltyBarMgr.cs - Visual penalty threshold bar for wrong diagnoses
// ============================================================================
// Uses TWO scene objects set up in the editor:
//   barBG   → dark background frame (static size, positioned by you)
//   barFill → red fill bar (script controls its width via scale.x)
// Once maxed out, the player is guaranteed to lose at end-of-level.
// ============================================================================

using Script.Library;

namespace Script
{
    public class PenaltyBarMgr : IScript
    {
        // =========================================================
        // SINGLETON
        // =========================================================
        public static PenaltyBarMgr Instance { get; private set; }

        // =========================================================
        // SERIALIZED FIELDS
        // =========================================================
        /// <summary>Background bar object (Image2D + RectTransform).
        /// Set color to dark grey in the editor. This never changes size.</summary>
        [SerializeField] public Object barBG;

        /// <summary>Fill bar object (Image2D + RectTransform).
        /// Set color to red in the editor. Place at same position as barBG.
        /// The script will shrink its width to 0 at start and grow it on wrong diagnoses.</summary>
        [SerializeField] public Object barFill;

        // =========================================================
        // CONFIGURATION
        // =========================================================
        /// <summary>Maximum wrong diagnoses before the bar is full.</summary>
        public int maxWrongAllowed = 3;

        // =========================================================
        // STATE
        // =========================================================
        private int wrongCount = 0;
        private float fullWidth = 1.0f;   // captured from barFill's original scale.x
        private float fillOriginX = 0.0f; // captured from barFill's original position.x

        public PenaltyBarMgr() : base("") { }

        // =========================================================
        // LIFECYCLE
        // =========================================================
        public override void Awake()
        {
            Instance = this;
        }

        public override void Start()
        {
            wrongCount = 0;

            // Capture the fill bar's full width and center position from the editor
            if (barFill != null)
            {
                var rt = barFill.getComponent<RectTransformComponent>();
                if (rt != null)
                {
                    fullWidth = rt.Data.scale.x;
                    fillOriginX = rt.Data.position.x;
                }
            }

            // Start empty
            RefreshBar();
        }

        public override void Update(double dt)
        {
            // Threshold bar hidden for now
            if (barBG != null)   barBG.setActive(false);
            if (barFill != null) barFill.setActive(false);
        }

        public override void LateUpdate(double dt) { }

        public override void Free()
        {
            Instance = null;
        }

        // =========================================================
        // PUBLIC API
        // =========================================================

        /// <summary>
        /// Call this every time the player diagnoses a patient incorrectly.
        /// The bar grows; once it reaches the cap it stays there.
        /// </summary>
        public void RecordWrongDiagnosis()
        {
            if (wrongCount < maxWrongAllowed)
            {
                wrongCount++;
                Logger.log(Logger.LogLevel.Info,
                    $"[PenaltyBarMgr] Wrong diagnosis #{wrongCount}/{maxWrongAllowed}");
            }
            RefreshBar();
        }

        /// <summary>
        /// Returns true when the bar is completely full.
        /// </summary>
        public bool IsMaxedOut()
        {
            return wrongCount >= maxWrongAllowed;
        }

        /// <summary>Current wrong count.</summary>
        public int GetWrongCount() { return wrongCount; }

        /// <summary>Reset for a new level.</summary>
        public void Reset()
        {
            wrongCount = 0;
            RefreshBar();
        }

        // =========================================================
        // INTERNAL
        // =========================================================

        /// <summary>
        /// Sets the fill bar's width and position so it grows LEFT-TO-RIGHT
        /// with its left edge pinned to the BG's left edge.
        /// </summary>
        private void RefreshBar()
        {
            if (barFill == null) return;

            float ratio = (maxWrongAllowed > 0)
                ? (float)wrongCount / maxWrongAllowed
                : 0f;
            if (ratio > 1f) ratio = 1f;

            float newWidth = fullWidth * ratio;

            // Pin left edge: the left edge of the full bar is at
            //   originX - fullWidth/2
            // The new center must be at:
            //   leftEdge + newWidth/2 = (originX - fullWidth/2) + newWidth/2
            float leftEdge = fillOriginX - fullWidth * 0.5f;
            float newCenterX = leftEdge + newWidth * 0.5f;

            var rt = barFill.getComponent<RectTransformComponent>();
            if (rt != null)
            {
                var d = rt.Data;
                d.scale.x = newWidth;
                d.position.x = newCenterX;
                rt.Data = d;
            }
        }
    }
}
