#include <iostream>
#include <vector>
#include "HashTables.h"

using namespace std;

int main() {
    ll L = 4;
    ll T = 50;
    ld w = 5;

    HashTables hashTable(L, T, w);

    vector<vector<ld>> vectors = {
            {1.0,   2.0,   3.0},
            {1.1,   2.1,   3.1},
            {10.0,  20.0,  30.0},
            {10.1,  20.1,  30.1},
            {100.0, 200.0, 300.0},
            {100.1, 200.1, 300.1}
    };

    for (const auto &vec: vectors) {
        hashTable.insert(vec);
    }

    vector<ld> query = {1.05, 2.05, 3.05};
    vector<vector<ld>> results = hashTable.search(query);

    cout << "Nearby vectors:" << endl;

    for (const auto &res: results) {
        cout << "[";

        for (size_t i = 0; i < res.size(); ++i) {
            cout << res[i];
            if (i < res.size() - 1) cout << ", ";
        }

        cout << "]" << endl;
    }

    return 0;
}
