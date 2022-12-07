#include "utils.hpp"
#include <vector>
#include <map>
#include <string>

using namespace std;

int main() {
    auto* model = new Kuhn_Trainer();
    model->train(1e6);
    // model->print_stragies();
}


class Kuhn_Trainer {
    // Kuhn Poker definitions
    int PASS = 0, BET = 1, NUM_ACTIONS = 2;
    float random = get_random_decimal();
    map<string, Node> nodeMap;
    // Information set node class definition
    class Node {
        // Kuhn node definitions
        string infoSet;
        vector<float> regretSum;
        vector<float> strategy;
        vector<float> strategySum;
        // Get current information set mixed strategy through regret-matching
        vector<float> getStrategy(float realizationWeight) {
            float normalizingSum = 0;
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
        // Get information set string representation
        string toString() {
            return string.format("%4s: %s", infoSet, Arrays.toString(getAverageStrategy()));
        }
    };
    // Train Kuhn
    void train(int iterations) {
        vector<int> cards = {1, 2, 3};
        float util = 0;
        for (int i = 0; i < iterations; i++) {
            // Shuffle cards
            for (int c1 = cards.size() - 1; c1 > 0; c1--) {
                int c2 = random.nextInt(c1 + 1);
                int tmp = cards[c1];
                cards[c1] = cards[c2];
                cards[c2] = tmp;
            }
            util += cfr(cards, "", 1, 1);
        }
        cout << "Average game value: " << util / iterations;
        for (Node n : nodeMap.values()) {
            cout << n;
        }
    }
    // Counterfactual regret minimization iteration
    float cfr(vector<int> cards, string history, float p0, float p1) {
        int plays = history.length();
        int player = plays % 2;
        int opponent = 1 - player;
        // Return payoff for terminal states
        if (plays > 1) {
            bool terminalPass = history.charAt(plays - 1) == 'p';
            bool floatBet = history.substring(plays - 2, plays).equals("bb");
            bool isPlayerCardHigher = cards[player] > cards[opponent];
            if (terminalPass)
                if (history.equals("pp"))
                    return isPlayerCardHigher ? 1 : -1;
                else
                    return 1;
            else if (floatBet)
                return isPlayerCardHigher ? 2 : -2;
        }
        string infoSet = cards[player] + history;
        // Get information set node or create it if nonexistant
        Node node = nodeMap.get(infoSet);
        if (node == null) {
        node = new Node();
        node.infoSet = infoSet;
        nodeMap.put(infoSet, node);
        }
        // For each action, recursively call cfr with additional history and probability
        vector<float> strategy = node.getStrategy(player == 0 ? p0 : p1);
        vector<float> util;
        float nodeUtil = 0;
        for (int a = 0; a < NUM_ACTIONS; a++) {
        string nextHistory = history + (a == 0 ? "p" : "b");
        util[a] = player == 0
        ? - cfr(cards, nextHistory, p0 * strategy[a], p1)
        : - cfr(cards, nextHistory, p0, p1 * strategy[a]);
        nodeUtil += strategy[a] * util[a];
        }
        // For each action, compute and accumulate counterfactual regret
        for (int a = 0; a < NUM_ACTIONS; a++) {
        float regret = util[a] - nodeUtil;
        node.regretSum[a] += (player == 0 ? p1 : p0) * regret;
        }
        return nodeUtil;
    }
};