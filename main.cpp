#include <iostream>
#include <vector>
#include "HashTables.h"
#include "LshadClass.h"

using namespace std;

void testHashTables() {
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

}

void testLSHADHyperparametersAutotuning() {
    vector<vector<ld>> data = {
            {1.0,   2.0,   3.0},
            {1.1,   2.1,   3.1},
            {10.0,  20.0,  30.0},
            {10.1,  20.1,  30.1},
            {100.0, 200.0, 300.0},
            {100.1, 200.1, 300.1},
            {1000.0, 2000.0, 3000.0},
            {1000.1, 2000.1, 3000.1},
            {10000.0, 20000.0, 30000.0},
            {10000.1, 20000.1, 30000.1},
            {100000.0, 200000.0, 300000.0},
            {100000.1, 200000.1, 300000.1}
    };

    LSHAD lshad;
    tuple<ll, ll, ld> hyperparameters = lshad.tuneHyperparameters(data);

    cout << "Hyperparameters: " << endl;
    cout << "L: " << get<0>(hyperparameters) << endl;
    cout << "T: " << get<1>(hyperparameters) << endl;
    cout << "w: " << get<2>(hyperparameters) << endl;
}

int main() {
    // testHashTables();
    testLSHADHyperparametersAutotuning();

    return 0;
}
