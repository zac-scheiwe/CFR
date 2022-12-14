#include <iostream>
#include <vector>
#include <iomanip>

using namespace std;

// float get_random_decimal() {
//     return (float) rand()/RAND_MAX;
// }

int get_random_integer(const int& exclusive_max, const int& inclusive_min=0) {
    return rand() % (exclusive_max - inclusive_min) + inclusive_min; 
}

template <typename T>
void print_vector(T& vec) {
    cout.precision(2);
    for (int i=0; i<vec.size(); i++) {
        cout << setw(10) << vec[i] << " ";
    }
    cout << "\n";
}

class LiarDieTrainer {
    public:
    // Liar Die definitions
    static const int DOUBT = 0, ACCEPT = 1;
    int sides;

    // Liar Die player decision node
    class Node {
        public:
        // Liar Die node definitions
        vector<float> regretSum, strategy, strategySum;
        float u, pPlayer, pOpponent;

        // Liar Die node constructor
        Node(int NUM_ACTIONS) {
            this->regretSum.resize(NUM_ACTIONS);
            this->strategy.resize(NUM_ACTIONS);
            this->strategySum.resize(NUM_ACTIONS);
        }

        // Get Liar Die node current mixed strategy through regret-matching
        vector<float> getStrategy() {
            float normalizingSum = 0;
            for (int a = 0; a < strategy.size(); a++) {
                strategy[a] = max(regretSum[a], (float) 0);
                normalizingSum += strategy[a];
            }
            for (int a = 0; a < strategy.size(); a++) {
                if (normalizingSum > 0)
                    strategy[a] /= normalizingSum;
                else
                    strategy[a] = 1.0/strategy.size();
            }
            for (int a = 0; a < strategy.size(); a++)
                strategySum[a] += pPlayer * strategy[a];
            return strategy;
        }

        // Get Liar Die node average mixed strategy
        vector<float> getAverageStrategy() {
            float normalizingSum = 0;
            for (int a = 0; a < strategySum.size(); a++)
                normalizingSum += strategySum[a];
            for (int a = 0; a < strategySum.size(); a++)
                if (normalizingSum > 0)
                    strategySum[a] /= normalizingSum;
                else
                    strategySum[a] = 1.0 / strategySum.size();
            return strategySum;
        }
    };

    vector<vector<Node*>> responseNodes;
    vector<vector<Node*>> claimNodes;

    // Construct trainer and allocate player decision nodes
    LiarDieTrainer(int sides) {
        this->sides = sides;
        
        this->responseNodes.resize(sides+1);
        for (int myClaim = 0; myClaim <= sides; myClaim++) {
            this->responseNodes[myClaim].resize(sides+1);
            for (int oppClaim = myClaim + 1; oppClaim <= sides; oppClaim++)
                responseNodes[myClaim][oppClaim] = new Node((oppClaim == 0 || oppClaim == sides) ? 1 : 2);
        }

        this->claimNodes.resize(sides);
        for (int oppClaim = 0; oppClaim < sides; oppClaim++) {
            this->claimNodes[oppClaim].resize(sides+1);
            for (int roll = 1; roll <= sides; roll++)
                this->claimNodes[oppClaim][roll] = new Node(sides - oppClaim);
        }
    }

    // Train with FSICFR
    void train(int iterations) {
        vector<float> regret(sides);
        vector<int> rollAfterAcceptingClaim(sides);
        for (int iter = 0; iter < iterations; iter++) {
            // Initialize rolls and starting probabilities
            for (int i = 0; i < rollAfterAcceptingClaim.size(); i++)
                rollAfterAcceptingClaim[i] = get_random_integer(sides+1, 1);
            this->claimNodes[0][rollAfterAcceptingClaim[0]]->pPlayer = 1;
            this->claimNodes[0][rollAfterAcceptingClaim[0]]->pOpponent = 1;

            // Accumulate realization weights forward
            for (int oppClaim = 0; oppClaim <= sides; oppClaim++) {
                // Visit response nodes forward
                if (oppClaim > 0)
                    for (int myClaim = 0; myClaim < oppClaim; myClaim++) {
                        Node* node = responseNodes[myClaim][oppClaim];
                        vector<float> actionProb = node->getStrategy();
                        if (oppClaim < sides) {
                            Node* nextNode = claimNodes[oppClaim][rollAfterAcceptingClaim[oppClaim]];
                            nextNode->pPlayer += actionProb[1] * node->pPlayer;
                            nextNode->pOpponent += node->pOpponent;
                        }
                    }
            
                // Visit claim nodes forward
                if (oppClaim < sides) {
                    Node* node = claimNodes[oppClaim][rollAfterAcceptingClaim[oppClaim]];
                    vector<float> actionProb = node->getStrategy();
                    for (int myClaim = oppClaim + 1; myClaim <= sides; myClaim++) {
                        float nextClaimProb = actionProb[myClaim - oppClaim - 1];
                        if (nextClaimProb > 0) {
                            Node* nextNode = responseNodes[oppClaim][myClaim];
                            nextNode->pPlayer += node->pOpponent;
                            nextNode->pOpponent += nextClaimProb * node->pPlayer;
                        }
                    }
                }
            }

            // Backpropagate utilities, adjusting regrets and strategies
            for (int oppClaim = sides; oppClaim >= 0; oppClaim--) {
                // Visit claim nodes backward
                if (oppClaim < sides) {
                    Node* node = claimNodes[oppClaim][rollAfterAcceptingClaim[oppClaim]];
                    vector<float> actionProb = node->strategy;
                    node->u = 0.0;
                    for (int myClaim = oppClaim + 1; myClaim <= sides; myClaim++) {
                        int actionIndex = myClaim - oppClaim - 1;
                        Node* nextNode = responseNodes[oppClaim][myClaim];
                        float childUtil = - nextNode->u;
                        regret[actionIndex] = childUtil;
                        node->u += actionProb[actionIndex] * childUtil;
                    }
                    for (int a = 0; a < actionProb.size(); a++) {
                        regret[a] -= node->u;
                        node->regretSum[a] += node->pOpponent * regret[a];
                    }
                    node->pPlayer = node->pOpponent = 0;
                }

                // Visit response nodes backward
                if (oppClaim > 0)
                    for (int myClaim = 0; myClaim < oppClaim; myClaim++) {
                        Node* node = responseNodes[myClaim][oppClaim];
                        vector<float> actionProb = node->strategy;
                        node->u = 0.0;
                        float doubtUtil = (oppClaim > rollAfterAcceptingClaim[myClaim]) ? 1 : -1;
                        regret[DOUBT] = doubtUtil;
                        node->u += actionProb[DOUBT] * doubtUtil;
                        if (oppClaim < sides) {
                            Node* nextNode = claimNodes[oppClaim][rollAfterAcceptingClaim[oppClaim]];
                            regret[ACCEPT] = nextNode->u;
                            node->u += actionProb[ACCEPT] * nextNode->u;
                        }
                        for (int a = 0; a < actionProb.size(); a++) {
                            regret[a] -= node->u;
                            node->regretSum[a] += node->pOpponent * regret[a];
                        }
                        node->pPlayer = node->pOpponent = 0;
                    }
            }

            // Reset strategy sums after half of training
            if (iter == iterations / 2) {
                for (auto nodes : responseNodes)
                    for (auto node : nodes)
                        if (node != nullptr)
                            // fill_n(node->strategySum.begin(), node->strategySum.end(), 0);
                            for (int a = 0; a < node->strategySum.size(); a++)
                                node->strategySum[a] = 0;

                for (auto nodes : claimNodes)
                    for (auto node : nodes)
                        if (node != nullptr)
                            for (int a = 0; a < node->strategySum.size(); a++)
                                node->strategySum[a] = 0;
            }
        }
    }

    void print_strategies() {
        // Print resulting strategy
        for (int initialRoll = 1; initialRoll <= sides; initialRoll++) {
            printf("Initial claim policy with roll %d: ", initialRoll);
            for (auto& prob : claimNodes[0][initialRoll]->getAverageStrategy())
                printf("%.2f ", prob);
            printf("\n");
        }
        vector<string> headings = {"Old Claim", "New Claim", "Doubt", "Accept"};
        print_vector(headings);
        vector<float> avg_strategy;
        for (int myClaim = 0; myClaim <= sides; myClaim++)
            for (int oppClaim = myClaim + 1; oppClaim <= sides; oppClaim++) {   
                printf("\t%d\t%d\t", myClaim, oppClaim);
                avg_strategy = responseNodes[myClaim][oppClaim]->getAverageStrategy();
                print_vector(avg_strategy);
            }
        headings = {"Old Claim", "Roll", "Action Probablities"};
        print_vector(headings);
        printf("\nOld Claim\tRoll\tAction Probabilities\n");
        for (int oppClaim = 0; oppClaim < sides; oppClaim++)
            for (int roll = 1; roll <= sides; roll++) {
                printf("%d\t%d\t", oppClaim, roll);
                avg_strategy = claimNodes[oppClaim][roll]->getAverageStrategy();
                print_vector(avg_strategy);
            }
    }
};