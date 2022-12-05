#include "utils.hpp"

int main() {
    int soldiers = 12;
    int battlefields = 3;
    const int NUM_LAYOUTS = nChoosek(soldiers + battlefields - 1, battlefields - 1);
    string* store = new string[NUM_LAYOUTS]; 
    int index = 0;
    string layout;
    char max_rep = '0' + soldiers;
    findNums(store, index, layout, max_rep, battlefields, soldiers);
    // print_array(store, NUM_LAYOUTS);

    
}