#include <iostream>
#include <vector>
#include <random>
#include <fstream>
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

vector<ld> generatePointInRange(ld minVal, ld maxVal) {
  random_device rd;
  mt19937 gen(rd());
  uniform_real_distribution<ld> dis(minVal, maxVal);
  return {dis(gen), dis(gen), dis(gen)};
}

vector<ld> generatePointInTwoRanges(ld minVal1, ld maxVal1, ld minVal2, ld maxVal2) {
  random_device rd;
  mt19937 gen(rd());
  uniform_real_distribution<ld> dis(0.0, 1.0);
  vector<ld> point(3);
  for (int i = 0; i < 3; ++i) {
    if (dis(gen) < 0.5) {
      point[i] = generatePointInRange(minVal1, maxVal1)[i];
    } 
    else {
      point[i] = generatePointInRange(minVal2, maxVal2)[i];
    }
  }
  return point;
}

void writePointsToFile(const vector<vector<ld>>& data, const string& filename) {
  ofstream outputFile(filename);
  if (outputFile.is_open()) {
    for (const auto& point : data) {
      outputFile << point[0] << ", " << point[1] << ", " << point[2] << endl;
    }
    outputFile.close();
  }
}

void testLSHADHyperparametersAutotuning(LSHAD &lshad) {
  int numPoints = 100;
  int numClosePoints = numPoints * 0.9;
  int numFarPoints = numPoints - numClosePoints;
  vector<vector<ld>> data;
  vector<vector<ld>> closePoints;
  for (int i = 0; i < numClosePoints; ++i) {
    closePoints.push_back(generatePointInRange(-10.0, 10.0));
  }
  vector<vector<ld>> farPoints;
  for (int i = 0; i < numFarPoints; ++i) {
    farPoints.push_back(generatePointInTwoRanges(-100.0, -50.0, 50.0, 100.0));
  }

  data.insert(data.end(), closePoints.begin(), closePoints.end());
  data.insert(data.end(), farPoints.begin(), farPoints.end());

  random_device rd;
  mt19937 g(rd());
  shuffle(data.begin(), data.end(), g);

  writePointsToFile(data, "points.txt");
  lshad.train(data, (ld) 0.1);

  vector<ld> query1 = {1.0, 2.0, 3.0};
  vector<ld> query2 = {1000.0, 2000.0, 3000.0};

  cout << lshad.detection_phase(query1) << endl;
  cout << lshad.detection_phase(query2) << endl;

  // 0.1
  //   tuple<ll, ll, ld> hyperparameters = lshad.tuneHyperparameters(data);
  //
  //   cout << "Hyperparameters: " << endl;
  //   cout << "L: " << get<0>(hyperparameters) << endl;
  //   cout << "T: " << get<1>(hyperparameters) << endl;
  //   cout << "w: " << get<2>(hyperparameters) << endl;
}

int main() {
    // testHashTables();
    LSHAD lshad;
    testLSHADHyperparametersAutotuning(lshad);

    return 0;
}
