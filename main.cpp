#include <iostream>
#include <vector>
#include <random>
#include <fstream>
#include <chrono>
#include "HashTables2.h"
#include "LshadClass.h"

using namespace std;

void testHashTables() {
    ll L = 4;
    ll T = 50;
    ld w = 5;

    HashTables hashTable(L, T, w, 3);

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
    unordered_set<vector<ld>, VectorHash<ld>, VectorEqual<ld>> results = hashTable.search(query);

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

void testLSHATrain(LSHAD &lshad) {
  int numPoints = 99;
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
  vector<ld> query1 = data[0];
  // data.push_back({50, 50, 50});
  data.insert(data.end(), farPoints.begin(), farPoints.end());

  random_device rd;
  mt19937 g(rd());
  shuffle(data.begin(), data.end(), g);

  writePointsToFile(data, "points.txt");
  std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
  lshad.train(data, (ld) 0.1);

  vector<ld> query2 = {50, 50, 50};

  ll count_close = 0;
  cout << "\nclose points: " << endl;
  for(auto point: closePoints) {
    bool isAnomaly = lshad.detection_phase(point);
    cout << isAnomaly << endl;
    count_close = !isAnomaly ? count_close + 1 : count_close;
  }

  ll count_far = 0;
  cout << "\nfar points: " << endl;
  for(auto point: farPoints) {
    bool isAnomaly = lshad.detection_phase(point);
    cout << isAnomaly << endl;
    count_far = isAnomaly ? count_far + 1 : count_far;
  }

  std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
  cout << "Time difference = " << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << "[ms]" << std::endl;

  cout << "Not anomaly accuracy: " << (ld) count_close / numClosePoints << endl;
  cout << "Anomaly accuracy: " << (ld) count_far / numFarPoints << endl;
}

void testLSHADHyperparametersAutotuning() {
    vector<vector<ld>> data = {
            {1.0,   2.0,   3.0},
            {1.1,   2.1,   3.1},
            {10.0,  20.0,  30.0},
            {10.1,  20.1,  30.1},
            {100.0, 200.0, 300.0},
            {100.1, 200.1, 300.1},
            {100.0, 200.0, 303.0},
            {100.1, 210.1, 300.1},
            {50.0, 50.0, 40.0},
            {45.1, 45.1, 45.1},
            {45.0, 45.0, 45.0},
            {100000.1, 200000.1, 300000.1}
    };

    LSHAD lshad;
    tuple<ll, ll, ld> hyperparameters = lshad.tuneHyperparameters(data);

    cout << "Hyperparameters: " << endl;
    cout << "L: " << get<0>(hyperparameters) << endl;
    cout << "T: " << get<1>(hyperparameters) << endl;
    cout << "w: " << get<2>(hyperparameters) << endl;
}

void testEstPerHash() {
  vector<vector<ld>> data = {
    {1.0,   2.0,   3.0},
    {1.1,   2.1,   3.1},
    {1.0,  2.0,  3.1},
    {10.1,  20.1,  30.1},
    {15.0, 25.0, 26.0},
    {15.1, 25.1, 27.0},
    {50.0, 40.0, 30.0},
    {9.1, 7.1, 10.1},
    {16.0, 15.0, 27.0},
    {21.1, 12.1, 35.5},
    {100000.0, 200000.0, 300000.0},
    {100000.1, 200000.1, 300000.1}
  };
  LSHAD lshad;
  lshad.train(data, (ld) 0.1);
}

int main() {
  // testHashTables();
  // testLSHADHyperparametersAutotuning();
  // testEstPerHash();
  LSHAD lshad;

  testLSHATrain(lshad);
  // testEstPerHash();

  return 0;
}
