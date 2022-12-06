#include "utils.hpp"
#include <iostream>
#include <vector>
using namespace std;

int main() {
    auto* model = new Battlefield_Trainer(5, 3);
    model->train(1e5);
    model->print_stragies();
}