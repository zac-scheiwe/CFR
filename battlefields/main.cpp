#include "utils.hpp"
using namespace std;

int main() {
    auto* model = new Battlefield_Trainer(5, 3);
    model->train(1);
    model->print_stragies();
}