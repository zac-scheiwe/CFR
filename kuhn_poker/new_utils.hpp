#include <vector>
#include <map>
#include <string>
#include <iostream>
#include <iomanip>
using namespace std;

float get_random_decimal() {
    return (float) rand()/RAND_MAX;
}

int get_random_integer(const int& exclusive_max, const int& inclusive_min=0) {
    return rand() % exclusive_max; 
}

template <typename T>
void print_vector(T& vec) {
    for (int i=0; i<vec.size(); i++) {
        // cout << vec[i] << " ";
        cout << setw(6) << vec[i] << " ";
    }
    cout << "\n";
}

class Kuhn_Trainer {
    public:
    // Kuhn Poker definitions
    int PASS = 0, BET = 1;
    static const int NUM_ACTIONS = 2;
    float random = get_random_decimal();
    // Information set node class definition
    class Node {
        public:
        // Kuhn node definitions
        string infoSet;
        vector<float> regretSum, strategy, strategySum;
        Node() {
            this->regretSum.resize(NUM_ACTIONS);
            this->strategy.resize(NUM_ACTIONS);
            this->strategySum.resize(NUM_ACTIONS);
        }
        // Get current information set mixed strategy through regret-matching
        vector<float> getStrategy(float realizationWeight) {
            float normalizingSum = 0.0;
            for (int a = 0; a < NUM_ACTIONS; a++) {
                strategy[a] = regretSum[a] > 0 ? regretSum[a] : 0;
                normalizingSum += strategy[a];
            }
            for (int a = 0; a < NUM_ACTIONS; a++) {
                if (normalizingSum > 0) {
                    strategy[a] /= normalizingSum;
                } else {
                    strategy[a] = 1.0 / NUM_ACTIONS;
                }
                strategySum[a] += realizationWeight * strategy[a];
            }
            return strategy;
        }
        // Get average information set mixed strategy across all training iterations
        vector<float> getAverageStrategy() {
            vector<float> avgStrategy;
            float normalizingSum = 0;
            for (int a = 0; a < NUM_ACTIONS; a++) {
                normalizingSum += strategySum[a];
            }
            for (int a = 0; a < NUM_ACTIONS; a++) {
                if (normalizingSum > 0) {
                    avgStrategy[a] = strategySum[a] / normalizingSum;
                } else {
                    avgStrategy[a] = 1.0 / NUM_ACTIONS;
                }
            }
            return avgStrategy;
        }
        // // Get information set string representation
        // string toString() {
        //     return string.format("%4s: %s", infoSet, Arrays.toString(getAverageStrategy()));
        // }
    };

    map<string, Node*> nodeMap;
    // Train Kuhn
    void train(int iterations) {
        vector<int> cards = {1, 2, 3};
        float util = 0;
        for (int i = 0; i < iterations; i++) {
            // Shuffle cards
            int c2, tmp;
            for (int c1 = cards.size() - 1; c1 > 0; c1--) {
                c2 = get_random_integer(c1 + 1);
                tmp = cards[c1];
                cards[c1] = cards[c2];
                cards[c2] = tmp;
            }
            util += cfr(cards, "", 1, 1);
        }
        post_train();
    }

    void post_train() {
    }

    void print_strategies() {
        // cout << "Average game value: " << util / iterations;
        for(std::map<string, Node*>::iterator it = nodeMap.begin(); it != nodeMap.end(); ++it) {
            cout << it->first << ": ";
            print_vector(it->second->regretSum);
        }
    }
    
    // Counterfactual regret minimization iteration
    float cfr(vector<int> cards, string history, float p0, float p1) {
        int plays = history.length();
        int player = plays % 2;
        int opponent = 1 - player;
        // Return payoff for terminal states
        if (plays > 1) {
            bool terminalPass = (history[plays - 1] == 'p');
            bool floatBet = history.substr(plays - 2, plays) == "bb";
            bool isPlayerCardHigher = cards[player] > cards[opponent];
            if (terminalPass) {
                if (history == "pp") {
                    return isPlayerCardHigher ? 1 : -1;
                } else {
                    return 1;
                }
            } else if (floatBet) {
                return isPlayerCardHigher ? 2 : -2;
            }
        }
        string infoSet = to_string(cards[player]) + history;
        // Get information set node or create it if nonexistant
        Node* node;
        if (nodeMap.find(infoSet) == nodeMap.end()) {  // not found
            node = new Node();
            node->infoSet = infoSet;
            nodeMap[infoSet] = node;
        } else {  // found
            node = nodeMap[infoSet];
        }
        // For each action, recursively call cfr with additional history and probability
        vector<float> strategy = node->getStrategy(player == 0 ? p0 : p1);
        vector<float> util;
        float nodeUtil = 0.0;
        for (int a = 0; a < NUM_ACTIONS; a++) {
            string nextHistory = history + (a == 0 ? "p" : "b");
            util.push_back(player == 0
            ? - cfr(cards, nextHistory, p0 * strategy[a], p1)
            : - cfr(cards, nextHistory, p0, p1 * strategy[a]));
            nodeUtil += strategy[a] * util[a];
        }
        // For each action, compute and accumulate counterfactual regret
        for (int a = 0; a < NUM_ACTIONS; a++) {
            float regret = util[a] - nodeUtil;
            node->regretSum[a] += (player == 0 ? p1 : p0) * regret;
        }
        return nodeUtil;
    }
};