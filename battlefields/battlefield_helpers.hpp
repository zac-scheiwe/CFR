#include <unordered_map>
using namespace std;

struct hash_pair {
    template <class T1, class T2>
    size_t operator()(const pair<T1, T2>& p) const
    {
        auto hash1 = hash<T1>{}(p.first);
        auto hash2 = hash<T2>{}(p.second);
    
        if (hash1 != hash2) {
            return hash1 ^ hash2;
        }
        
        // If hash1 == hash2, their XOR is zero.
        return hash1;
    }
};

int get_utility(string my_layout, string other_layout) {
    // Memoize results
    pair<string, string> layout_pair = make_pair(my_layout, other_layout);
    static unordered_map<pair<string, string>, int, hash_pair> cached_utilities;
    auto iter = cached_utilities.find(layout_pair);
    if (iter != cached_utilities.end()) {
        return iter->second;
    }
    // Else calculate result:
    int utility = 0;
    int soldier_diff;
    for (int i=0; i<my_layout.size(); i++) {
        soldier_diff = my_layout[i] - other_layout[i];
        if (soldier_diff > 0) utility++;
        else if (soldier_diff < 0) utility--;
    }
    cached_utilities[layout_pair] = utility;
    return utility;
}


unsigned int nChoosek(unsigned int n, unsigned int k ) {
    if (k > n) return 0;
    if (k * 2 > n) k = n-k;
    if (k == 0) return 1;

    int result = n;
    for (int i = 2; i <= k; ++i) {
        result *= (n-i+1);
        result /= i;
    }
    return result;
}

// function to find all n–digit numbers with a sum of digits equal
// to `target` in a bottom-up manner
void find_layouts(string* store, int& index, string layout, const int& n, const int& target) {
    const char MAX_CHAR = '0' + target;
    // if the number is less than n–digit and its sum of digits is
    // less than the given sum
    if (n && target >= 0) {
        char d = '0';
        // consider every valid digit and put it in the current
        // index and recur for the next index
        while (d <= MAX_CHAR) {
            find_layouts(store, index, layout + d, n - 1, target - (d - '0'));
            d++;
        }
    }

    // if the number becomes n–digit and its sum of digits is
    // equal to the given sum, print it
    else if (n == 0 && target == 0) {
        store[index] = layout;
        index++;
    }
}

