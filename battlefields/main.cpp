#include "utils.hpp"

int main() {
    auto* model = new Battlefield_Trainer(5, 3);
    model->train(1e6);
    model->print_stragies();
}