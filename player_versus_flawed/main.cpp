#include "utils.hpp"
#include <iostream>

int main() {
    auto* model = new PSR_Trainer;
    model->train(1000);
    print_array(model->get_average_strategy());
    print_array(model->get_strategy());
    print_array(model->strategySum);
    print_array(model->regretSum);
}