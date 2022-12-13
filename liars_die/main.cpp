#include "new_utils.hpp"

int main() {
    auto* model = new LiarDieTrainer();
    model->train(1);
    model->print_strategies();
}