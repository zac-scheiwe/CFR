#include "utils.hpp"

int main() {
    auto* model = new Kuhn_Trainer();
    model->train(1e5);
    model->print_strategies();
}