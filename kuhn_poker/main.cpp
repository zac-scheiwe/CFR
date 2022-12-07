#include "new_utils.hpp"

int main() {
    auto* model = new Kuhn_Trainer();
    model->train(1);
    model->print_strategies();
}