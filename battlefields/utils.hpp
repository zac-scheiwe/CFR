#include <cstdlib>
#include <iostream>
#include <vector>
#include <string>
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

const int nChoosek(int n, int k ) {
    if (k > n) return 0;
    if (k * 2 > n) k = n-k;
    if (k == 0) return 1;

    int result = n;
    for (int i = 2; i <= k; ++i) {
        result *= (n-i+1);
        result /= i;
    }
    return result;
}

class Battlefield_Trainer {
    public:
    int SOLDIERS, BATTLEFIELDS;
    Battlefield_Trainer(const int& S, const int& N, int seed=0) {
        srand(seed);
        SOLDIERS = S;
        BATTLEFIELDS = N;
        // NUM_ACTIONS = nChoosek(SOLDIERS + BATTLEFIELDS - 1, BATTLEFIELDS - 1);
    }
    const int NUM_ACTIONS = 21;

    vector<string> layouts;
    vector<int*> utility_table;

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
    
    // function to find all n–digit numbers with a sum of digits equal
    // to `target` in a bottom-up manner
    void recursive_fill(vector<string>& layouts, string layout, const int& n, const int& target) {
        const char MAX_CHAR = '0' + target;
        // if the number is less than n–digit and its sum of digits is
        // less than the given sum
        if (n && target >= 0) {
            char d = '0';
            // consider every valid digit and put it in the current
            // index and recur for the next index
            while (d <= MAX_CHAR) {
                recursive_fill(layouts, layout + d, n - 1, target - (d - '0'));
                d++;
            }
        }
        // if the number becomes n–digit and its sum of digits is
        // equal to the given sum, print it
        else if (n == 0 && target == 0) {
            layouts.push_back(layout);
            string abc = layout;
        }
    }

    // Generate 1D array of soldier layouts
    vector<string> get_layouts() {
        string layout;
        recursive_fill(layouts, layout, BATTLEFIELDS, SOLDIERS);
        return layouts;
    }

    int calculate_utility(vector<string>& layouts, int my_action, int other_action) {
        string my_layout = layouts[my_action];
        string other_layout = layouts[other_action]; 

        int utility = 0;
        int soldier_diff;
        for (int i=0; i<BATTLEFIELDS; i++) {
            soldier_diff = my_layout[i] - other_layout[i];
            if (soldier_diff > 0) utility++;
            else if (soldier_diff < 0) utility--;
        }
        if (utility > 0) return 1;
        if (utility < 0) return -1;
        else return 0;
    }


    vector<int*> get_utility_table(vector<string>& layouts) {
        // utility_table.reserve(NUM_ACTIONS);
        for (int j = 0; j < NUM_ACTIONS; j++) {
            int* row = new int[NUM_ACTIONS];
            for (int i = 0; i < j; i++) {
                row[i] = - utility_table[i][j];
            }
            for (int i = j; i < NUM_ACTIONS; i++) {
                row[i] = calculate_utility(layouts, i, j);
            }
            utility_table.push_back(row);
        }
        return utility_table;
    }

    int* get_regrets(const vector<int*>& utility_table, const int& my_action, const int& other_action) {
        int* regrets = new int[NUM_ACTIONS];
        int* possible_utilities = utility_table[other_action];

        int my_utility = possible_utilities[my_action];
        for (int a = 0; a < NUM_ACTIONS; a++) {
            regrets[a] = possible_utilities[a] - my_utility;
        }
        return regrets;
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

    void problem_setup() {
        layouts = get_layouts();
        utility_table = get_utility_table(layouts);
    }

    void train(const int& iterations) {
        problem_setup();

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

            my_regrets = get_regrets(utility_table, my_action, other_action);
            other_regrets = get_regrets(utility_table, other_action, my_action);

            my_regretSum = add_arrays(my_regretSum, my_regrets, NUM_ACTIONS);
            other_regretSum = add_arrays(other_regretSum, other_regrets, NUM_ACTIONS);

            my_avgStrategy = get_average_strategy(my_strategySum);
            other_avgStrategy = get_average_strategy(other_strategySum);
        }
    }

    void print_stragies() {
        cout.precision(2);
        cout << "My strategy:    ";
        print_array(my_avgStrategy, NUM_ACTIONS);
        cout << "Other strategy: ";
        print_array(other_avgStrategy, NUM_ACTIONS);
    }

    // void sort_results(int* index, float* values) {
    //     typedef vector<int>::const_iterator myiter;

    //     vector<pair<size_t, myiter>> order(NUM_ACTIONS);

    //     size_t n = 0;
    //     for (myiter it = index.begin(); it != Index.end(); ++it, ++n)
    //         order[n] = make_pair(n, it);

   
    // }
    };
