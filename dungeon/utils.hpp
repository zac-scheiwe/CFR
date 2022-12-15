#include <iostream>
#include <vector>
#include <set>
#include <iomanip>

using namespace std;

int get_random_integer(const int& exclusive_max, const int& inclusive_min=0) {
    return rand() % (exclusive_max - inclusive_min) + inclusive_min; 
}

template <typename T>
void print_vector(T& vec) {
    cout.precision(2);
    for (int i=0; i<vec.size(); i++) {
        cout << setw(3) << vec[i] << " ";
    }
    cout << "\n";
}

class Deck {
    vector<int> cards = {1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 7, 9};

    public:
    Deck() {
        shuffle();
    }

    void shuffle() {
        int c2, tmp;
        for (int c1 = cards.size() - 1; c1 > 0; c1--) {
            c2 = get_random_integer(c1 + 1);
            tmp = cards[c1];
            cards[c1] = cards[c2];
            cards[c2] = tmp;
        }
    }

    int draw() {
        int out = cards.back();
        cards.pop_back();
        return out;
    }

    void print() {
        print_vector(cards);
    }
};

struct Dungeon {
    multiset<int, less<int>> known_cards = {};
    int num_unknown_cards = 0;
    set<int> items = {0, 1, 2, 3, 4, 5};
};

