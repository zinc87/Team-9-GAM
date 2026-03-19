using Script.Library;

namespace Script
{
    // This static class is visible to ALL your scripts automatically.
    public static class GameState
    {
        // 1. Tracks if a generic UI (like Patient Panel) is open
        public static bool IsUIOpen { get; private set; } = false;

        // 2. Tracks if the game has ended (Win/Lose)
        public static bool IsGameOver { get; private set; } = false;

        // 3. The Master Check: Player/Camera only needs to check this ONE property
        public static bool IsInputAllowed
        {
            get 
            { 
                // Input is allowed ONLY if UI is closed AND Game is not over
                return !IsUIOpen && !IsGameOver; 
            }
        }

        public static bool CanOpenPatientUI { get; private set; }

        public static void SetCanOpenPatientUI(bool canOpen)
        {
            CanOpenPatientUI = canOpen;
        }

        // --- Functions to change state ---

        public static void SetUIState(bool isOpen)
        {
            IsUIOpen = isOpen;

            // --- THIS WAS MISSING IN YOUR FILE ---
            if (isOpen)
            {
                Input.enableMouse(); // Show Cursor when UI opens
            }
            else
            {
                // Only hide cursor if the game is still running
                if (!IsGameOver) 
                {
                    Input.disableMouse(); 
                }
            }
        }

        public static void SetGameOver(bool isOver)
        {
            IsGameOver = isOver;
            
            // Always show cursor on Game Over screen
            if (isOver)
            {
                Input.enableMouse();
            }
        }

        /// <summary>
        /// Resets all game state for a fresh game start.
        /// Call this when loading the game scene.
        /// </summary>
        public static void ResetAll()
        {
           // Logger.log(Logger.LogLevel.Info, "[GameState] ResetAll called - resetting all game state");
            IsUIOpen = false;
            IsGameOver = false;
            CanOpenPatientUI = false;
            Input.disableMouse();  // Lock cursor for first-person gameplay
           // Logger.log(Logger.LogLevel.Info, $"[GameState] After reset: IsUIOpen={IsUIOpen}, IsGameOver={IsGameOver}, IsInputAllowed={IsInputAllowed}");
        }
    }
}