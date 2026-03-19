using System;
using System.Collections.Generic;
using Script.Library;

namespace Script
{
    public class ConfettiSpawner : Script.Library.IScript
    {
        public static ConfettiSpawner Instance { get; private set; }

        private class ConfettiPiece
        {
            public Script.Library.Object Obj;
            public Vector3D Velocity;
            public Vector3D RotationSpeed;
            public float Lifetime;
            public float Age;          // time alive, for flutter phase
            public float DriftPhase;   // random offset for sinusoidal sway
            public float DriftAmplitude; // how wide the side-to-side sway is
            public Vector3D BaseScale;
        }

        private List<ConfettiPiece> _confettiPieces = new List<ConfettiPiece>();
        
        private bool _isActive = false;
        private float _delayTimer = 0f;
        private float _durationTimer = 0f;
        private float _spawnTimer = 0f;

        private float _duration = 6.0f;
        private float _delay = 3.0f;
        private float _spawnRate = 0.03f; // ~33 pieces per second (balanced quantity/lag)

        private Random _rng = new Random();
        private const int MAX_CONFETTI = 100;

        // 3D parameters
        private Vector3D _spawnOrigin;

        public override void Awake()
        {   
            Instance = this;
        }

        public override void Start()
        {
            // Fallback origin if camera/player isn't found immediately
            _spawnOrigin = new Vector3D(0, 10, 0);
        }

        public void TriggerConfetti(float duration, float delay = 0f)
        {
            _duration = duration;
            _delay = delay;
            _delayTimer = delay;
            _durationTimer = duration;
            _isActive = true;

            // Try to spawn above the walkway (Waypoints 9 and 10)
            try 
            {
                Vector3D w9 = WaypointPatientSpawner.Waypoint9;
                Vector3D w10 = WaypointPatientSpawner.Waypoint10;
                
                // Average the two waypoints for a central spawn origin, then add height
                _spawnOrigin = new Vector3D(
                    (w9.x + w10.x) * 0.5f,
                    Math.Max(w9.y, w10.y) + 12.0f, // 12 units above the walkway
                    (w9.z + w10.z) * 0.5f
                );
            }
            catch (Exception)
            {
                // Fallback to player position
                if (PlayerController.Instance != null && PlayerController.Instance.Obj != null)
                {
                    var pcTransform = PlayerController.Instance.Obj.getComponent<TransformComponent>();
                    if (pcTransform != null)
                    {
                        _spawnOrigin = pcTransform.Transformation.position + new Vector3D(0, 8.0f, 0);
                    }
                }
            }
        }

        public override void Update(double dt)
        {
            float fdt = (float)dt;

            // Handle spawning logic
            if (_isActive)
            {
                if (_delayTimer > 0f)
                {
                    _delayTimer -= fdt;
                }
                else if (_durationTimer > 0f)
                {
                    _durationTimer -= fdt;
                    _spawnTimer -= fdt;

                    while (_spawnTimer <= 0f)
                    {
                        SpawnConfettiPiece();
                        _spawnTimer += _spawnRate;
                    }
                }
                else
                {
                    _isActive = false; // Done spawning
                }
            }

            // Update existing pieces (falling, spinning, shrinking/fading, and hiding/pooling)
            for (int i = _confettiPieces.Count - 1; i >= 0; i--)
            {
                var piece = _confettiPieces[i];
                piece.Lifetime -= fdt;

                if (piece.Lifetime <= 0f || piece.Obj == null)
                {
                    if (piece.Obj != null)
                    {
                        piece.Obj.setActive(false);
                        _inactivePool.Push(piece.Obj);
                    }
                    _confettiPieces.RemoveAt(i);
                    continue;
                }

                piece.Age += fdt;

                // Gentle gravity (confetti is light, falls slowly)
                piece.Velocity.y -= 3.5f * fdt;

                // Air drag — slows pieces down over time for a floaty feel
                float drag = 1.0f - 1.8f * fdt;
                if (drag < 0f) drag = 0f;
                piece.Velocity.x *= drag;
                piece.Velocity.z *= drag;
                // Less drag on Y so they still fall, but cap terminal velocity
                if (piece.Velocity.y < -3.0f) piece.Velocity.y = -3.0f;

                // Sinusoidal side-to-side flutter (like real confetti drifting)
                float flutter = piece.DriftAmplitude * (float)Math.Sin(piece.Age + piece.DriftPhase);

                // Update Transform
                var comp = piece.Obj.getComponent<TransformComponent>();
                if (comp != null)
                {
                    var trf = comp.Transformation;
                    trf.position += piece.Velocity * fdt;
                    trf.position.x += flutter * fdt;
                    trf.Rotation += piece.RotationSpeed * fdt;

                    // Shrink as it dies
                    if (piece.Lifetime < 2.0f)
                    {
                        float s = Math.Max(piece.Lifetime / 2.0f, 0f);
                        trf.Scale = piece.BaseScale * s;
                    }

                    comp.Transformation = trf;
                }
            }
        }

        private Stack<Script.Library.Object> _inactivePool = new Stack<Script.Library.Object>();

        private void SpawnConfettiPiece()
        {
            if (_confettiPieces.Count >= MAX_CONFETTI)
                return;


            Script.Library.Object pieceObj;

            if (_inactivePool.Count > 0)
            {
                pieceObj = _inactivePool.Pop();
                pieceObj.setActive(true);
            }
            else
            {
                string newId = Script.Library.Object.createGameObject();
                pieceObj = new Script.Library.Object(newId);

                pieceObj.addComponent(ComponentTypes.Transform);
                pieceObj.addComponent(ComponentTypes.MeshRenderer);

                var meshComp = pieceObj.getComponent<MeshRendererComponent>();
                if (meshComp != null)
                {
                    meshComp.SetMesh("quad.agstaticmesh");
                }
            }

            // Randomly position within the designated confetti area
            var trfComp = pieceObj.getComponent<TransformComponent>();
            Vector3D baseScale = new Vector3D(0.012f, 0.012f, 0.012f);
            if (trfComp != null)
            {
                var trf = trfComp.Transformation;
                
                // Default Spawn within x: -14.65 to -11.2, z: -4 to 6
                float spawnX = -14.65f + (float)(_rng.NextDouble() * (-11.2 - (-14.65)));
                float spawnZ = -4.0f + (float)(_rng.NextDouble() * (6.0 - (-4.0)));
                
                if (LevelScoreManager.CurrentLevel == 2)
                {
                    spawnX = -12.542f + (float)(_rng.NextDouble() * (-7.968f - (-12.542f)));
                    spawnZ = -20.0f + (float)(_rng.NextDouble() * (-10.0f - (-20.0f)));
                }
                
                float spawnY =13.0f;
                trf.position = new Vector3D(spawnX, spawnY, spawnZ);
                trf.Rotation = new Vector3D((float)_rng.NextDouble() * 360f, (float)_rng.NextDouble() * 360f, (float)_rng.NextDouble() * 360f);

                
                trf.Scale = baseScale;

                trfComp.Transformation = trf;
            }

            // Randomize color/material each time it spawns
            var mComp = pieceObj.getComponent<MeshRendererComponent>();
            if (mComp != null)
            {
                int colorIdx = _rng.Next(3);
                string matName = "Material_Blue.agmat";
                switch(colorIdx)
                {
                    case 0: matName = "Material_Blue.agmat"; break;
                    case 1: matName = "Confetti_Yellow.agmat"; break;
                    case 2: matName = "Confetti_Green.agmat"; break;
                    //case 3: matName = "Confetti_Purple.agmat"; break;
                    //case 4: matName = "Confetti_Pink.agmat"; break;
                    //case 5: matName = "Confetti_Orange.agmat"; break;
                    //case 6: matName = "Confetti_Cyan.agmat"; break;
                    //case 7: matName = "Confetti_LimeGreen.agmat"; break;
                    //case 8: matName = "Confetti_White.agmat"; break;
                }
                
                // Set material on the cube's actual submesh
                mComp.SetMeshRendererSubmeshMaterial("Plane.001_mat0", matName);
            }

            // Burst upward and outward, then gravity + drag take over
            float angleRad = (float)(_rng.NextDouble() * Math.PI * 2.0);
            float outwardSpeed = 2.0f + (float)_rng.NextDouble() * 4.0f;
            float upwardSpeed = 4.0f + (float)_rng.NextDouble() * 5.0f;

            ConfettiPiece piece = new ConfettiPiece
            {
                Obj = pieceObj,
                BaseScale = baseScale,
                Velocity = new Vector3D(
                    (float)Math.Cos(angleRad) * outwardSpeed,
                    upwardSpeed,
                    (float)Math.Sin(angleRad) * outwardSpeed
                ),
                RotationSpeed = new Vector3D(
                    (float)(_rng.NextDouble() * 500.0 - 250.0),
                    (float)(_rng.NextDouble() * 500.0 - 250.0),
                    (float)(_rng.NextDouble() * 500.0 - 250.0)
                ),
                Lifetime = 5.0f + (float)_rng.NextDouble() * 3.0f,
                Age = 0f,
                DriftPhase = (float)(_rng.NextDouble() * Math.PI * 2.0),
                DriftAmplitude = 1.0f + (float)_rng.NextDouble() * 2.0f
            };

            _confettiPieces.Add(piece);
        }

        public override void LateUpdate(double dt) { }

        public override void Free()
        {
            _confettiPieces.Clear();
            _inactivePool.Clear();
            Instance = null;
        }
    }
}
