using Script.Library;
using System.Collections.Generic;

namespace Script
{
    public class GameController : IScript
    {
        private Object queueAnchor;
        public float queueSpacing = 5.0f;

        private StudentScript current;
        private bool anchorReady = false;

        public override void Update(double dt)
        {
            // Step 1 — Load QueueAnchor only once
            if (!anchorReady)
            {
                TryResolveAnchor();
                return;
            }

            // Step 2 — Handle queue assignment
            AssignQueuePositions();

            // Step 3 — Handle check station (E/Q keys)
            HandleCheckStation();

            // Step 4 — Handle right-click for vaping popup
            HandleRightClickOnStudent();

            // Step 5 — Check if all students are done
            CheckWinLoseCondition();
        }

        private void TryResolveAnchor()
        {
            foreach (var wp in WaypointHolder.All)
            {
                if (wp.waypointType == "QueueAnchor")
                {
                    queueAnchor = new Object(wp.id);
                    anchorReady = true;
                    return;
                }
            }
        }

        private void AssignQueuePositions()
        {
            if (queueAnchor == null)
                return;

            Vector3D anchorPos = GetPosition(queueAnchor);

            var students = new List<StudentScript>(StudentScript.All);
            students.RemoveAll(s => s.CurrentState == StudentScript.State.Done);

            for (int i = 0; i < students.Count; i++)
            {
                // BUILD QUEUE FORWARD FROM ANCHOR
                Vector3D pos = new Vector3D(
                    anchorPos.x,
                    anchorPos.y,
                    anchorPos.z - (i * queueSpacing)
                );

                students[i].AssignQueuePosition(pos);
            }

            // PROMOTE FIRST STUDENT TO CHECK ZONE
            if (students.Count > 0)
            {
                StudentScript first = students[0];

                if (first.CurrentState == StudentScript.State.InQueue)
                {
                    first.CurrentState = StudentScript.State.WalkingToCheck;
                }
            }
        }

        private void HandleCheckStation()
        {
            if (current == null || current.CurrentState != StudentScript.State.Waiting)
                current = GetNextWaitingStudent();

            if (current == null)
                return;

            if (Input.isKeyPressed(Input.KeyCode.kE))
            {
                current.SendToArrest();
                current = null;
            }

            if (Input.isKeyPressed(Input.KeyCode.kQ))
            {
                current.SendToFree();
                current = null;
            }
        }

        // Right-click to show vaping dialog
        private void HandleRightClickOnStudent()
        {
            // Check if right mouse button is clicked
            if (!Input.isMouseClicked(Input.Mouse.kMOUSE_BUTTON_RIGHT))
                return;

            // Make sure DialogPanelMgr exists
            if (DialogPanelMgr.Instance == null)
            {
                Library.Logger.log(
                    Library.Logger.LogLevel.Warning,
                    "DialogPanelMgr not found in scene!"
                );
                return;
            }

            // Get the student currently at the check station
            StudentScript studentToCheck = GetNextWaitingStudent();

            if (studentToCheck != null && studentToCheck.CurrentState == StudentScript.State.Waiting)
            {
                // Show the vaping popup for this student
                DialogPanelMgr.Instance.ShowVapingPanel(studentToCheck);
                current = studentToCheck; // Set as current
            }
            else
            {
                Library.Logger.log(
                    Library.Logger.LogLevel.Info,
                    "No student at check station to question"
                );
            }
        }

        private void CheckWinLoseCondition()
        {
            // If there are no students at all, do nothing
            if (StudentScript.All.Count == 0)
                return;

            // Check if all students are done (processed)
            foreach (var s in StudentScript.All)
            {
                if (s.CurrentState != StudentScript.State.Done)
                    return; // Found someone still in queue → do nothing
            }

            // If we reach here → ALL STUDENTS ARE DONE
            WinLose.SetOpen(true);
        }

        private StudentScript GetNextWaitingStudent()
        {
            foreach (var s in StudentScript.All)
                if (s.CurrentState == StudentScript.State.Waiting)
                    return s;

            return null;
        }

        private Vector3D GetPosition(Object obj)
        {
            var tr = obj.getComponent<TransformComponent>();
            if (tr == null)
                return new Vector3D(0, 0, 0);

            return tr.Transformation.position;
        }

        public override void Start()
        {
        }
        public override void Awake() { }
        public override void LateUpdate(double dt) { }
        public override void Free() { }
    }
}