#include "MasterHeader.h"

void GenerateWave(int waveNumber, const std::vector<EnemyType>& enemyPool) {
    // 1. CALCULATE BUDGET
    // Formula: Base 15 + (5 * Wave^1.5). Grows exponentially for difficulty.
    float budget = 15.0f + (5.0f * std::pow(static_cast<float>(waveNumber), 1.5f));

    std::cout << "--- Wave " << waveNumber << " Started (Budget: " << budget << ") ---\n";

    // 2. FILTER & WEIGHTING
    struct WeightedChoice { EnemyType type; float currentWeight; };
    std::vector<WeightedChoice> activeChoices;
    float totalWeight = 0.0f;

    for (const auto& e : enemyPool) {
        if (waveNumber >= e.minWave) {
            // Calculate current weight: Base + (Growth * Wave), clamped to 0
            float w = static_cast<float>(e.baseWeight + (e.growthRate * waveNumber));
            w = std::max(0.0f, w);

            activeChoices.push_back({ e, w });
            totalWeight += w;
        }
    }

    // 3. SPAWNING LOOP
    // Keep picking enemies until we can't afford the cheapest one or run out of points
    while (budget > 0 && totalWeight > 0) {
        // Roll for a weighted random enemy
        float roll = static_cast<float>(rand()) / (static_cast<float>(RAND_MAX) / totalWeight);
        float cursor = 0.0f;
        int pickedIndex = -1;

        for (int i = 0; i < activeChoices.size(); ++i) {
            cursor += activeChoices[i].currentWeight;
            if (roll <= cursor) {
                pickedIndex = i;
                break;
            }
        }

        if (pickedIndex != -1) {
            EnemyType& selected = activeChoices[pickedIndex].type;

            if (budget >= selected.cost) {
                // SUCCESS: Spawn logic goes here (e.g., using your BinaryMap for coordinates)
                // SpawnEnemy(selected.id); 
                budget -= selected.cost;
                std::cout << "Spawned Enemy ID: " << selected.id << " (Remaining Budget: " << budget << ")\n";
            }
            else {
                // If we can't afford this specific enemy, remove its weight so we don't pick it again
                totalWeight -= activeChoices[pickedIndex].currentWeight;
                activeChoices[pickedIndex].currentWeight = 0.0f;
            }
        }
    }
    std::cout << "--- Wave Generation Complete ---\n\n";
}

// --- EXAMPLE USAGE ---
/*
std::vector<EnemyType> pool = {
    { 1, 10, 100, -2, 1 }, // Grunt: Common early, becomes rarer (%)
    { 2, 35, 0, 8, 5 },    // Elite: Starts at Wave 5, becomes common
    { 3, 100, -20, 15, 10} // Boss: Rare, high cost, starts Wave 10
};

GenerateWave(1, pool);  // Mostly ID 1
GenerateWave(15, pool); // Mixture of ID 2 and 3
*/