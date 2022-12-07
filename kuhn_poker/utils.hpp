#include <cstdlib>
#include <iostream>
#include <iomanip>
#include <vector>
#include <string>
#include <algorithm>
using namespace std;

float get_random_decimal() {
    return (float) rand()/RAND_MAX;
}

template <typename T>
void print_vector(T& vec) {
    for (int i=0; i<vec.size(); i++) {
        // cout << vec[i] << " ";
        cout << setw(6) << vec[i] << " ";
    }
    cout << "\n";
}

template <typename T>
T sum_arrays(T& a, T& b) {
    T result(a.size());
    for (int i=0; i<a.size(); i++) {
        result[i] = a[i] + b[i];
    }
    return result;
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
    int SOLDIERS, BATTLEFIELDS, NUM_ACTIONS;

    vector<string> layouts;
    vector<vector<int>> utility_table;

    vector<int> myRegretSum, otherRegretSum;
    vector<float> myStrategySum, otherStrategySum;
    vector<float> myAvgStrategy, otherAvgStrategy;

    vector<string> sorted_layouts;
    vector<float> sorted_myAvgStrategy;
    vector<float> sorted_otherAvgStrategy;

    Battlefield_Trainer(const int& S, const int& N, int seed=0) {
        srand(seed);
        SOLDIERS = S;
        BATTLEFIELDS = N;
        NUM_ACTIONS = nChoosek(SOLDIERS + BATTLEFIELDS - 1, BATTLEFIELDS - 1);
        
        myRegretSum.resize(NUM_ACTIONS);
        otherRegretSum.resize(NUM_ACTIONS);
        myStrategySum.resize(NUM_ACTIONS);
        otherStrategySum.resize(NUM_ACTIONS);
        myAvgStrategy.resize(NUM_ACTIONS);
        otherAvgStrategy.resize(NUM_ACTIONS);
        sorted_layouts.resize(NUM_ACTIONS);
        sorted_myAvgStrategy.resize(NUM_ACTIONS);
        sorted_otherAvgStrategy.resize(NUM_ACTIONS);
    }


    // Get current mixed strategy through regret-matching
    vector<float> get_strategy(const vector<int>& regretSum) {
        vector<int> positive_regret_sums(NUM_ACTIONS);
        int normalizing_sum = 0;

        for (int a = 0; a < NUM_ACTIONS; a++) {
            if (regretSum[a] > 0) {
                positive_regret_sums[a] = regretSum[a];
                normalizing_sum += positive_regret_sums[a];
            }
        }

        vector<float> strategy(NUM_ACTIONS);
        if (normalizing_sum > 0) {
            for (int a = 0; a < NUM_ACTIONS; a++) {
                strategy[a] = (float) positive_regret_sums[a] / (float) normalizing_sum;
            }
        } else {
            float uniform_probability = 1.0 / NUM_ACTIONS;
            fill(strategy.begin(), strategy.end(), uniform_probability);
        }
        return strategy;
    }

    // Get random action according to mixed-strategy distribution
    int get_action(const vector<float>& strategy) {
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
        }
    }

    // Generate 1D array of soldier layouts
    vector<string> get_layouts() {
        string layout;
        recursive_fill(layouts, layout, BATTLEFIELDS, SOLDIERS);
        return layouts;
    }

    int calculate_utility(const vector<string>& layouts, const int& my_action, const int& other_action) {
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


    vector<vector<int>> get_utility_table(const vector<string>& layouts) {
        // utility_table.reserve(NUM_ACTIONS);
        for (int j = 0; j < NUM_ACTIONS; j++) {
            vector<int> row(NUM_ACTIONS);
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

    vector<int> get_regrets(const vector<vector<int>>& utility_table, const int& my_action, const int& other_action) {
        vector<int> regrets(NUM_ACTIONS);
        vector<int> possible_utilities = utility_table[other_action];

        int my_utility = possible_utilities[my_action];
        for (int a = 0; a < NUM_ACTIONS; a++) {
            regrets[a] = possible_utilities[a] - my_utility;
        }
        return regrets;
    }

    vector<float> get_average_strategy(const vector<float>& strategySum) {
        vector<float> avg_strategy(NUM_ACTIONS);
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

    void post_train() {
        auto order = get_reverse_ordering(myAvgStrategy);

        sorted_layouts = sort_from_ref(layouts, order);
        sorted_myAvgStrategy = sort_from_ref(myAvgStrategy, order);
        sorted_otherAvgStrategy = sort_from_ref(otherAvgStrategy, order);
    }

    void train(const int& iterations) {
        problem_setup();

        vector<float> my_strategy(NUM_ACTIONS), other_strategy(NUM_ACTIONS);
        int my_action, other_action;
        vector<int> my_regrets(NUM_ACTIONS), other_regrets(NUM_ACTIONS); 

        for (int i = 0; i < iterations; i++) {
            my_strategy = get_strategy(myRegretSum);
            other_strategy = get_strategy(otherRegretSum);

            myStrategySum = sum_arrays(myStrategySum, my_strategy);
            otherStrategySum = sum_arrays(otherStrategySum, other_strategy);

            my_action = get_action(my_strategy);
            other_action = get_action(other_strategy);

            my_regrets = get_regrets(utility_table, my_action, other_action);
            other_regrets = get_regrets(utility_table, other_action, my_action);

            myRegretSum = sum_arrays(myRegretSum, my_regrets);
            otherRegretSum = sum_arrays(otherRegretSum, other_regrets);

            myAvgStrategy = get_average_strategy(myStrategySum);
            otherAvgStrategy = get_average_strategy(otherStrategySum);
        }

        post_train();
    }
    typedef vector<float>::const_iterator myiter;
    vector<pair<size_t, myiter>> get_reverse_ordering(const vector<float>& index) {
        vector<pair<size_t, myiter>> order(index.size());
        size_t n = 0;
        for (myiter it = index.begin(); it != index.end(); ++it, ++n) {
            order[n] = make_pair(n, it);
        }
        struct ordering {
            bool operator ()(pair<size_t, myiter> const& a, pair<size_t, myiter> const& b) {
                return *(a.second) > *(b.second);
            }
        };
        sort(order.begin(), order.end(), ordering());
        return order;
    }

    template <typename T>
    vector<T> sort_from_ref(vector<T> const& in, vector<pair<size_t, myiter>> const& reference) {
        vector<T> ret(in.size());
        size_t const size = in.size();
        for (size_t i = 0; i < size; ++i) {
            ret[i] = in[reference[i].first];
        }
        return ret;
    }

    void print_stragies() {
        cout.precision(2);
        cout << "                ";
        print_vector(sorted_layouts);
        cout << "My strategy:    ";
        print_vector(sorted_myAvgStrategy);
        cout << "Other strategy: ";
        print_vector(sorted_otherAvgStrategy);
    }
};
