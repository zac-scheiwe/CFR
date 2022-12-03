#include <cstdlib>
#include <iostream>
using namespace std;

float get_random_decimal() {
    return (float) rand()/RAND_MAX;
}

void print_array(float* array) {
    for (int i=0; i<3; i++) {
        cout << array[i] << " ";
    }
    cout << "\n";
}

class PSR_Trainer {
    public:
    PSR_Trainer(int seed=0) {
        srand(seed);
    }
    int PAPER = 0, SCISSORS = 1, ROCK = 2, NUM_ACTIONS = 3;

    float OPP_STRATEGY[3] = { 0.4, 0.3, 0.3 };

    // Get current mixed strategy through regret-matching
    float* get_strategy(const float* regretSum) {
        int* positive_regret_sums = new int[NUM_ACTIONS];
        int normalizing_sum = 0;

        for (int a = 0; a < NUM_ACTIONS; a++) {
            if (regretSum[a] > 0) {
                positive_regret_sums[a] = regretSum[a];
                normalizing_sum += positive_regret_sums[a];
            }
        }

        float* strategy = new float[NUM_ACTIONS];
        if (normalizing_sum > 0) {
            fill_n(strategy, NUM_ACTIONS, 1.0 / NUM_ACTIONS);
        } else {
            for (int a = 0; a < NUM_ACTIONS; a++) {
                strategy[a] = positive_regret_sums[a] / normalizing_sum;
            }
        }
        return strategy;
    }

    // Get random action according to mixed-strategy distribution
    int get_action(const float* strategy) {
        float r = get_random_decimal();
        float F = 0.0;
        for (int a=0; a<NUM_ACTIONS; a++) {
            F += strategy[a];
            if (r < F) return a;
        }
        return -1;
    }

    void train(const int iterations) {
        auto* regretSum = new int[NUM_ACTIONS];
        auto* strategySum = new float[NUM_ACTIONS];
        auto* action_utility = new short int[NUM_ACTIONS];

        for (int i = 0; i < iterations; i++) {
            // Get regret-matched mixed-strategy actions
            float* strategy = get_strategy(strategySum);
            int my_action = get_action(strategy);
            int other_action = get_action(OPP_STRATEGY);

            // Compute action utilities
            action_utility[other_action == NUM_ACTIONS - 1 ? 0 : other_action + 1] = 1;
            action_utility[other_action == 0 ? NUM_ACTIONS - 1 : other_action - 1] = -1;

            // Accumulate action regrets and strategy frequencies
            auto& my_utility = action_utility[my_action];
            for (int a = 0; a < NUM_ACTIONS; a++) {
                regretSum[a] += action_utility[a] - my_utility;
                strategySum[a] += strategy[a];
            }
        }
    }

    float* get_average_strategy(const float* strategySum) {
        auto* avg_strategy = new float[NUM_ACTIONS];
        float normalizing_sum = 0.0;
        for (int a = 0; a < NUM_ACTIONS; a++) {
            normalizing_sum += strategySum[a];
        }
        for (int a = 0; a < NUM_ACTIONS; a++) {
            if (normalizing_sum > 0) {
                avg_strategy[a] = strategySum[a] / normalizing_sum;
            } else {
                avg_strategy[a] = 1.0 / NUM_ACTIONS;
            }
        }
        return avg_strategy;
    }

};
