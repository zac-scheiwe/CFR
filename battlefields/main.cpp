#include "utils.hpp"
#include "battlefield_helpers.hpp"
using namespace std;

int main() {
    const int SOLDIERS = 5, BATTLEFIELDS = 3;
    const int NUM_LAYOUTS = nChoosek(SOLDIERS + BATTLEFIELDS - 1, BATTLEFIELDS - 1);
    string* store = new string[NUM_LAYOUTS]; 
    int index = 0;
    string layout;
    find_layouts(store, index, layout, BATTLEFIELDS, SOLDIERS);
    // print_array(store, NUM_LAYOUTS);


    auto* model = new PSR_Trainer(NUM_LAYOUTS);
    model->train(1);
    model->print_stragies();

    // cout << get_utility(store[0], store[19]);
}