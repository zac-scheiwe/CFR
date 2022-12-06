#include "utils.hpp"
using namespace std;

int main() {
    auto* model = new Battlefield_Trainer(5, 3);
    model->train(1e5);

    for (int i=0; i<21; i++) {
        cout << model->layouts[i] << " ";
    }
    cout << "\n";
    // for (int i=0; i<21; i++) {
    //     cout << model->my_regretSum[i] << " ";
    // }
    // cout << "\n";
    // for (int i=0; i<21; i++) {
    //     cout << model->my_strategySum[i] << " ";
    // }
    // cout << "\n";

    // for (int i=0; i<21; i++) {
    //     for (int j=0; j<21; j++) {
    //         cout << model->utility_table[i][j] << " ";
    //     }
    //     cout << "\n";
    // }
    // cout << model->calculate_utility(model->layouts, 0, 0);

    model->print_stragies();
}