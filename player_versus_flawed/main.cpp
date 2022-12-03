#include "utils.hpp"

int main() {
    auto* model = new PSR_Trainer;
    model->train(1e6);
    print_array(model->get_average_strategy());
}