#include "utils.hpp"

int main() {
    auto* model = new LiarDieTrainer(13);
    model->train(1e6);
    model->print_strategies();
}