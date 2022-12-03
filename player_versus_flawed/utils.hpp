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

    float* regretSum = new float[NUM_ACTIONS];
    float* strategy = new float[NUM_ACTIONS];
    float* strategySum = new float[NUM_ACTIONS];
    float oppStrategy[3] = { 0.4, 0.3, 0.3 };

    // Get current mixed strategy through regret-matching
    float* get_strategy() {
        float normalizing_sum = 0;
        for (int a = 0; a < NUM_ACTIONS; a++) {
            strategy[a] = regretSum[a] > 0 ? regretSum[a] : 0;
            normalizing_sum += strategy[a];
        }
        for (int a = 0; a < NUM_ACTIONS; a++) {
            if (normalizing_sum > 0) {
                strategy[a] /= normalizing_sum;
            } else {
                strategy[a] = 1.0 / NUM_ACTIONS;
            }
            strategySum[a] += strategy[a];
        }
        return strategy;
    }

    // Get random action according to mixed-strategy distribution
    int get_action(float* strategy) {
        float r = get_random_decimal();
        float F = 0.0;
        for (int a=0; a<NUM_ACTIONS; a++) {
            F += strategy[a];
            if (r < F) return a;
        }
        return -1;
    }

    void train(int iterations) {
        auto* action_utility = new float[NUM_ACTIONS];
        for (int i = 0; i < iterations; i++) {
            // Get regret-matched mixed-strategy actions
            float* strategy = get_strategy();
            int my_action = get_action(strategy);
            int other_action = get_action(oppStrategy);

            // Compute action utilities
            action_utility[other_action] = 0;
            action_utility[other_action == NUM_ACTIONS - 1 ? 0 : other_action + 1] = 1;
            action_utility[other_action == 0 ? NUM_ACTIONS - 1 : other_action - 1] = -1;

            // Accumulate action regrets
            for (int a = 0; a < NUM_ACTIONS; a++) {
                regretSum[a] += action_utility[a] - action_utility[my_action];
            }
        }
    }

    float* get_average_strategy() {
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
