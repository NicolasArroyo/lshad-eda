#include <iostream>
#include <vector>
#include <random>
#include <fstream>
#include <chrono>
#include "HashTables2.h"
#include "LshadClass.h"

using namespace std;

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

void writePointsToFile(const vector<vector<ld>> &data, const string &filename) {
    ofstream outputFile(filename);

    if (outputFile.is_open()) {
        for (const auto &point: data) {
            outputFile << point[0] << ", " << point[1] << ", " << point[2] << endl;
        }
        outputFile.close();
    }
}

void testLSHATrain(LSHAD &lshad) {
    /////////////////////////////////////////////////////////////////////////
    // RANDOM POINTS CREATION ///////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////

    int numPoints = 100;

    int numNonAnomalyPoints = int(numPoints * 0.9);
    int numAnomalyPoints = numPoints - numNonAnomalyPoints;

    vector<vector<ld>> data;

    vector<vector<ld>> closePoints;
    closePoints.reserve(numNonAnomalyPoints);
    for (int i = 0; i < numNonAnomalyPoints; ++i) {
        closePoints.push_back(generatePointInRange(-10.0, 10.0));
    }
    data.insert(data.end(), closePoints.begin(), closePoints.end());

    vector<vector<ld>> farPoints;
    farPoints.reserve(numAnomalyPoints);
    for (int i = 0; i < numAnomalyPoints; ++i) {
        farPoints.push_back(generatePointInTwoRanges(-100.0, -50.0, 50.0, 100.0));
    }
    data.insert(data.end(), farPoints.begin(), farPoints.end());

    random_device rd;
    mt19937 g(rd());
    shuffle(data.begin(), data.end(), g);
    writePointsToFile(data, "points.txt");

    /////////////////////////////////////////////////////////////////////////
    // TRAINING PHASE ///////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////

    chrono::steady_clock::time_point training_start_clock = chrono::steady_clock::now();

    lshad.train(data, 0.1);

    chrono::steady_clock::time_point training_end_clock = chrono::steady_clock::now();
    cout << "Training phase elapsed time: "
         << chrono::duration_cast<chrono::milliseconds>(training_end_clock - training_start_clock).count()
         << "[ms]"
         << endl;

    /////////////////////////////////////////////////////////////////////////
    // QUERIES PHASE  ///////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////

    chrono::steady_clock::time_point query_start_clock = chrono::steady_clock::now();

    ll nonAnomalyCounter = 0;
    cout << "\nNon-anomaly points: " << endl;
    for (const auto &point: closePoints) {
        bool isAnomaly = lshad.detection_phase(point);
        nonAnomalyCounter = !isAnomaly ? nonAnomalyCounter + 1 : nonAnomalyCounter;

        cout << "("
             << point[0] << ", "
             << point[1] << ", "
             << point[2] << ")"
             << " = " << boolalpha << isAnomaly
             << endl;
    }

    ll anomalyCounter = 0;
    cout << "\nAnomaly points: " << endl;
    for (const auto &point: farPoints) {
        bool isAnomaly = lshad.detection_phase(point);
        anomalyCounter = isAnomaly ? anomalyCounter + 1 : anomalyCounter;

        cout << "("
             << point[0] << ", "
             << point[1] << ", "
             << point[2] << ")"
             << " = " << boolalpha << isAnomaly
             << endl;
    }

    cout << "\nNot-anomaly accuracy: "
         << (ld) nonAnomalyCounter / numNonAnomalyPoints
         << endl;

    cout << "Anomaly accuracy: "
         << (ld) anomalyCounter / numAnomalyPoints
         << endl;

    chrono::steady_clock::time_point query_end_clock = chrono::steady_clock::now();

    cout << "Queries phase elapsed time: "
         << chrono::duration_cast<chrono::milliseconds>(query_end_clock - query_start_clock).count()
         << "[ms]"
         << endl;

    /////////////////////////////////////////////////////////////////////////
    // QUERIES PHASE END ////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////
}

int main() {
    LSHAD lshad;
    testLSHATrain(lshad);
}
