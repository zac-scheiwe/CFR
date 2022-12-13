class LiarDieTrainer {

    // Liar Die definitions
    static int DOUBT = 0, ACCEPT = 1;
    static Random random = new Random();
    int sides;
    Node[][] responseNodes;
    Node[][] claimNodes;

    // Liar Die player decision node
    class Node {
        // Liar Die node definitions
        public double[] regretSum, strategy, strategySum;
        public double u, pPlayer, pOpponent;

        // Liar Die node constructor
        public Node(int numActions) {
            regretSum = new double[numActions];
            strategy = new double[numActions];
            strategySum = new double[numActions];
        }

        // Get Liar Die node current mixed strategy through regret-matching
        public double[] getStrategy() {
            double normalizingSum = 0;
            for (int a = 0; a < strategy.length; a++) {
                strategy[a] = Math.max(regretSum[a], 0);
                normalizingSum += strategy[a];
            }
            for (int a = 0; a < strategy.length; a++) {
                if (normalizingSum > 0)
                strategy[a] /= normalizingSum;
                else
                strategy[a] = 1.0/strategy.length;
                }
            for (int a = 0; a < strategy.length; a++)
                strategySum[a] += pPlayer * strategy[a];
            return strategy;
        }

        // Get Liar Die node average mixed strategy
        public double[] getAverageStrategy() {
            double normalizingSum = 0;
            for (int a = 0; a < strategySum.length; a++)
                normalizingSum += strategySum[a];
            for (int a = 0; a < strategySum.length; a++)
                if (normalizingSum > 0)
                    strategySum[a] /= normalizingSum;
                else
                    strategySum[a] = 1.0 / strategySum.length;
            return strategySum;
        }
    }

    // Construct trainer and allocate player decision nodes
    public LiarDieTrainer(int sides) {
        this.sides = sides;
        responseNodes = new Node[sides][sides + 1];

        for (int myClaim = 0; myClaim <= sides; myClaim++)
            for (int oppClaim = myClaim + 1; oppClaim <= sides; oppClaim++)
                responseNodes[myClaim][oppClaim] = new Node((oppClaim == 0 || oppClaim == sides) ? 1 : 2);
        claimNodes = new Node[sides][sides + 1];

        for (int oppClaim = 0; oppClaim < sides; oppClaim++)
            for (int roll = 1; roll <= sides; roll++)
                claimNodes[oppClaim][roll] = new Node(sides - oppClaim);
    }

    // Train with FSICFR
    public void train(int iterations) {
        double[] regret = new double[sides];
        int[] rollAfterAcceptingClaim = new int[sides];
        for (int iter = 0; iter < iterations; iter++) {
            // Initialize rolls and starting probabilities
            for (int i = 0; i < rollAfterAcceptingClaim.length; i++)
                rollAfterAcceptingClaim[i] = random.nextInt(sides) + 1;
            claimNodes[0][rollAfterAcceptingClaim[0]].pPlayer = 1;
            claimNodes[0][rollAfterAcceptingClaim[0]].pOpponent = 1;

            // Accumulate realization weights forward
            for (int oppClaim = 0; oppClaim <= sides; oppClaim++) {
                // Visit response nodes forward
                if (oppClaim > 0)
                    for (int myClaim = 0; myClaim < oppClaim; myClaim++) {
                        Node node = responseNodes[myClaim][oppClaim];
                        double[] actionProb = node.getStrategy();
                        if (oppClaim < sides) {
                            Node nextNode = claimNodes[oppClaim][rollAfterAcceptingClaim[oppClaim]];
                            nextNode.pPlayer += actionProb[1] * node.pPlayer;
                            nextNode.pOpponent += node.pOpponent;
                        }
                    }
            
                // Visit claim nodes forward
                if (oppClaim < sides) {
                    Node node = claimNodes[oppClaim][rollAfterAcceptingClaim[oppClaim]];
                    double[] actionProb = node.getStrategy();
                    for (int myClaim = oppClaim + 1; myClaim <= sides; myClaim++) {
                        double nextClaimProb = actionProb[myClaim - oppClaim - 1];
                        if (nextClaimProb > 0) {
                            Node nextNode = responseNodes[oppClaim][myClaim];
                            nextNode.pPlayer += node.pOpponent;
                            nextNode.pOpponent += nextClaimProb * node.pPlayer;
                        }
                    }
                }
            }

            // Backpropagate utilities, adjusting regrets and strategies
            for (int oppClaim = sides; oppClaim >= 0; oppClaim--) {
                // Visit claim nodes backward
                if (oppClaim < sides) {
                    Node node = claimNodes[oppClaim][rollAfterAcceptingClaim[oppClaim]];
                    double[] actionProb = node.strategy;
                    node.u = 0.0;
                    for (int myClaim = oppClaim + 1; myClaim <= sides; myClaim++) {
                        int actionIndex = myClaim - oppClaim - 1;
                        Node nextNode = responseNodes[oppClaim][myClaim];
                        double childUtil = - nextNode.u;
                        regret[actionIndex] = childUtil;
                        node.u += actionProb[actionIndex] * childUtil;
                    }
                    for (int a = 0; a < actionProb.length; a++) {
                        regret[a] -= node.u;
                        node.regretSum[a] += node.pOpponent * regret[a];
                    }
                    node.pPlayer = node.pOpponent = 0;
                }

                // Visit response nodes backward
                if (oppClaim > 0)
                    for (int myClaim = 0; myClaim < oppClaim; myClaim++) {
                        Node node = responseNodes[myClaim][oppClaim];
                        double[] actionProb = node.strategy;
                        node.u = 0.0;
                        double doubtUtil = (oppClaim > rollAfterAcceptingClaim[myClaim]) ? 1 : -1;
                        regret[DOUBT] = doubtUtil;
                        node.u += actionProb[DOUBT] * doubtUtil;
                        if (oppClaim < sides) {
                            Node nextNode = claimNodes[oppClaim][rollAfterAcceptingClaim[oppClaim]];
                            regret[ACCEPT] = nextNode.u;
                            node.u += actionProb[ACCEPT] * nextNode.u;
                        }
                        for (int a = 0; a < actionProb.length; a++) {
                            regret[a] -= node.u;
                            node.regretSum[a] += node.pOpponent * regret[a];
                        }
                        node.pPlayer = node.pOpponent = 0;
                    }
            }

            // Reset strategy sums after half of training
            if (iter == iterations / 2) {
                for (Node[] nodes : responseNodes)
                    for (Node node : nodes)
                        if (node != null)
                            for (int a = 0; a < node.strategySum.length; a++)
                                node.strategySum[a] = 0;

                for (Node[] nodes : claimNodes)
                    for (Node node : nodes)
                        if (node != null)
                            for (int a = 0; a < node.strategySum.length; a++)
                                node.strategySum[a] = 0;
            }
        }

        // Print resulting strategy
        for (int initialRoll = 1; initialRoll <= sides; initialRoll++) {
            System.out.printf("Initial claim policy with roll %d: ", initialRoll);
            for (double prob : claimNodes[0][initialRoll].getAverageStrategy())
                System.out.printf("%.2f ", prob);
            System.out.println();
        }
        System.out.println("\nOld Claim\tNew Claim\tAction Probabilities");
        for (int myClaim = 0; myClaim <= sides; myClaim++)
            for (int oppClaim = myClaim + 1; oppClaim <= sides; oppClaim++)
                System.out.printf("\t%d\t%d\t%s\n", myClaim, oppClaim,
                Arrays.toString(responseNodes[myClaim][oppClaim].getAverageStrategy()));
        System.out.println("\nOld Claim\tRoll\tAction Probabilities");
        for (int oppClaim = 0; oppClaim < sides; oppClaim++)
            for (int roll = 1; roll <= sides; roll++)
                System.out.printf("%d\t%d\t%s\n", oppClaim, roll,
                Arrays.toString(claimNodes[oppClaim][roll].getAverageStrategy()));
    }

}