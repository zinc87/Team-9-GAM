using System;
using System.Collections.Generic;

namespace Script
{
    // ================================================
    // Helper class: Weighted Item
    // ================================================
    [Serializable]
    public class WeightedItem
    {
        public string item;      // You can change to int/Object/etc.
        public float weight;

        public WeightedItem(string item, float weight)
        {
            this.item = item;
            this.weight = weight;
        }
    }

    // ================================================
    // Symtom Spawner Script
    // ================================================
    public class SymtomSpawner : Script.Library.IScript
    {
         // ------------------------------
        // Weighted random list
        // Appears in Inspector
        // ------------------------------
        public List<WeightedItem> weightedList = new List<WeightedItem>();

        private Random rng = new Random();
        private bool keyWasDown = false;

        public override void Awake() 
        {
            // Clear existing list
            weightedList.Clear();

            // Add weighted entries
            weightedList.Add(new WeightedItem("Symtom 1", 60f));
            weightedList.Add(new WeightedItem("Symtom 2", 25f));
            weightedList.Add(new WeightedItem("Symtom 3", 15f));
        }

        public override void Start() 
        {
        }

        public override void Update(double dt)
        {
            bool isDown = Library.Input.isKeyPressed(Library.Input.KeyCode.kT);

            // When key is first pressed (edge trigger)
            if (isDown && !keyWasDown)
            {
                string randomPick = GetWeightedRandom();
                Library.Logger.log(Library.Logger.LogLevel.Info, $"Weighted pick: {randomPick}");

                // Spawn player
                Library.Object.createGameObject();
            }

            // Update previous state
            keyWasDown = isDown;
        }

        public override void LateUpdate(double dt) { }
        public override void Free() { }

        // ================================================
        // Weighted Random Function
        // ================================================
        private string GetWeightedRandom()
        {
            if (weightedList == null || weightedList.Count == 0)
                return null;

            float totalWeight = 0f;

            // Add all weights together
            foreach (var w in weightedList)
                totalWeight += w.weight;

            // Pick a random number within the weight sum
            float randomValue = (float)(rng.NextDouble() * totalWeight);

            // Walk through list until randomValue falls inside a weight block
            foreach (var w in weightedList)
            {
                if (randomValue < w.weight)
                    return w.item;

                randomValue -= w.weight;
            }

            // Fallback
            return weightedList[weightedList.Count - 1].item;
        }
    }
}
