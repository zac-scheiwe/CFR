#include <cstdlib>
#include <iostream>
using namespace std;

float get_random_decimal() {
    return (float) rand()/RAND_MAX;
}

template <typename T>
void print_array(T* array, int num_elements) {
    for (int i=0; i<num_elements; i++) {
        cout << array[i] << " ";
    }
    cout << "\n";
}

template <typename T>
T* add_arrays(T* a, T* b, int num_elements) {
    T* c = new T[num_elements];
    for (int i=0; i<num_elements; i++) {
        c[i] = a[i] + b[i];
    }
    return c;
}

class PSR_Trainer {
    public:
    PSR_Trainer(int seed=0) {
        srand(seed);
    }
    // PAPER = 0, SCISSORS = 1, ROCK = 2
    const int NUM_ACTIONS = 3;

    int *my_regretSum = new int[NUM_ACTIONS], *other_regretSum = new int[NUM_ACTIONS];
    float *my_strategySum = new float[NUM_ACTIONS], *other_strategySum = new float[NUM_ACTIONS];
    float *my_avgStrategy = new float[NUM_ACTIONS], *other_avgStrategy = new float[NUM_ACTIONS];

    // Get current mixed strategy through regret-matching
    float* get_strategy(const int* regretSum) {
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
            for (int a = 0; a < NUM_ACTIONS; a++) {
                strategy[a] = (float) positive_regret_sums[a] / (float) normalizing_sum;
            }
        } else {
            fill_n(strategy, NUM_ACTIONS, 1.0 / NUM_ACTIONS);
        }

        return strategy;
    }


    // Get random action according to mixed-strategy distribution
    int get_action(const float* strategy) {
        float r = get_random_decimal();
        float F = 0.0;
        for (int a=0; a<NUM_ACTIONS; a++) {
            F += strategy[a];
            if (r <= F) return a;
        }
        return -1;
    }

    int* get_regrets(const int& my_action, const int& other_action) {
            // Compute action utilities
            int* action_utilities = new int[NUM_ACTIONS];
            action_utilities[other_action == NUM_ACTIONS - 1 ? 0 : other_action + 1] = 1;
            action_utilities[other_action == 0 ? NUM_ACTIONS - 1 : other_action - 1] = -1;

            // Accumulate action regrets and strategy frequencies
            int my_utility = action_utilities[my_action];
            for (int a = 0; a < NUM_ACTIONS; a++) {
                action_utilities[a] -= my_utility;
            }
            return action_utilities;
    }

    float* get_average_strategy(float* strategySum) {
        auto* avg_strategy = new float[NUM_ACTIONS];
        float normalizing_sum = 0.0;
        for (int a = 0; a < NUM_ACTIONS; a++) {
            normalizing_sum += strategySum[a];
        }
        for (int a = 0; a < NUM_ACTIONS; a++) {
            avg_strategy[a] = strategySum[a] / normalizing_sum;
        }
        return avg_strategy;
    }

    void train(const int& iterations) {
        float *my_strategy, *other_strategy;
        int my_action, other_action;
        int *my_regrets = new int[NUM_ACTIONS], *other_regrets = new int[NUM_ACTIONS]; 

        for (int i = 0; i < iterations; i++) {
            // Get regret-matched mixed-strategy actions
            my_strategy = get_strategy(my_regretSum);
            other_strategy = get_strategy(other_regretSum);

            my_strategySum = add_arrays(my_strategySum, my_strategy, NUM_ACTIONS);
            other_strategySum = add_arrays(other_strategySum, other_strategy, NUM_ACTIONS);

            my_action = get_action(my_strategy);
            other_action = get_action(other_strategy);

            my_regrets = get_regrets(my_action, other_action);
            other_regrets = get_regrets(other_action, my_action);

            my_regretSum = add_arrays(my_regretSum, my_regrets, NUM_ACTIONS);
            other_regretSum = add_arrays(other_regretSum, other_regrets, NUM_ACTIONS);

            my_avgStrategy = get_average_strategy(my_strategySum);
            other_avgStrategy = get_average_strategy(other_strategySum);
        }
    }

    void print_stragies() {
        cout << "My strategy:    ";
        print_array(my_avgStrategy, NUM_ACTIONS);
        cout << "Other strategy: ";
        print_array(other_avgStrategy, NUM_ACTIONS);
    }

};
