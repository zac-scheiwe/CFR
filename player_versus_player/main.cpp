#include "utils.hpp"

int main() {
    auto* model = new PSR_Trainer;
    model->train(1e6);
    model->print_stragies();
}