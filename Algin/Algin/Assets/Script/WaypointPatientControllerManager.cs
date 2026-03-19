// ============================================================================
// WaypointPatientControllerManager.cs - Static manager for patient controllers
// ============================================================================
// PURPOSE:
// Since the engine doesn't automatically call Update() on manually instantiated
// scripts, this manager provides a centralized place to register controllers
// and propagate Update() calls from an in-scene script.
// 
// USAGE:
// 1. Attach WaypointPatientSpawner to a game object in the scene
// 2. When spawning patients, the spawner registers controllers with this manager
// 3. The spawner's Update() calls WaypointPatientControllerManager.UpdateAll()
//    to propagate updates to all registered controllers
//
// Alternatively, if your engine supports automatic script updates on spawned
// objects, you can bypass this manager.
// ============================================================================

using Script.Library;
using System.Collections.Generic;

namespace Script
{
    /// <summary>
    /// Static manager that maintains references to all spawned patient controllers
    /// and provides a way to call their Update methods continuously.
    /// </summary>
    public static class WaypointPatientControllerManager
    {
        #region State
        
        public static int CurrentQueueSize = 0;

        /// <summary>
        /// List of all registered patient controllers.
        /// </summary>
        private static List<WaypointPatientController> _controllers = new List<WaypointPatientController>();

        #endregion

        #region Registration

        /// <summary>
        /// Register a new patient controller with the manager.
        /// </summary>
        /// <param name="controller">The controller to register.</param>
        public static void RegisterController(WaypointPatientController controller)
        {
            if (controller != null && !_controllers.Contains(controller))
            {
                _controllers.Add(controller);
                Logger.log(Logger.LogLevel.Info, 
                    $"WaypointPatientControllerManager: Registered controller (total: {_controllers.Count})");
            }
        }

        /// <summary>
        /// Unregister a patient controller from the manager.
        /// </summary>
        /// <param name="controller">The controller to unregister.</param>
        public static void UnregisterController(WaypointPatientController controller)
        {
            if (_controllers.Remove(controller))
            {
                Logger.log(Logger.LogLevel.Info, 
                    $"WaypointPatientControllerManager: Unregistered controller (remaining: {_controllers.Count})");
            }
        }

        /// <summary>
        /// Clear all registered controllers.
        /// </summary>
        public static void ClearAll()
        {
            _controllers.Clear();
            CurrentQueueSize = 0;
            Logger.log(Logger.LogLevel.Info, 
                "WaypointPatientControllerManager: Cleared all controllers");
        }

        #endregion

        #region Updates

        /// <summary>
        /// Call Update on all registered controllers.
        /// This should be called from a scene-attached script's Update method.
        /// </summary>
        /// <param name="dt">Delta time in seconds.</param>
        public static void UpdateAll(double dt)
        {
            foreach (var controller in _controllers)
            {
                controller.Update(dt);
            }
        }

        #endregion

        #region Queue Management
        
        /// <summary>
        /// Call to advance the queue position for all remaining patients.
        /// </summary>
        public static void AdvanceAllQueue()
        {
            if (CurrentQueueSize > 0)
            {
                CurrentQueueSize--;
            }

            foreach (var controller in _controllers)
            {
                if (controller != null && !controller.IsMarkedForRemoval)
                {
                    controller.AdvanceQueue();
                }
            }
        }

        /// <summary>
        /// Gets the next queue index and increments the queue size tracker.
        /// </summary>
        public static int GetNextQueueIndex()
        {
            int index = CurrentQueueSize;
            CurrentQueueSize++;
            return index;
        }

        #endregion

        #region Queries

        /// <summary>
        /// Get the number of registered controllers.
        /// </summary>
        public static int GetControllerCount()
        {
            return _controllers.Count;
        }

        /// <summary>
        /// Get a read-only copy of all controllers.
        /// </summary>
        public static IReadOnlyList<WaypointPatientController> GetAllControllers()
        {
            return _controllers.AsReadOnly();
        }

        #endregion
    }
}
