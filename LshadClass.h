#pragma once
#include "HashTables.h"
#include "hashes.h"
#include <unordered_map>

using namespace std;

class LSHAD {
  HashTables *hasher;
  unordered_map<InnerMap, ld, InnerMapHash, InnerMapEqual> estPerHash;
  ld threshold;
public:
  LSHAD(): hasher(nullptr), estPerHash(unordered_map<InnerMap, ld, InnerMapHash, InnerMapEqual>()), threshold(0){}
  ~LSHAD(){}

  // Training phase of the LSHAD algorithm
  void train(const vector<vector<ld>> data, ll anomalyRatio){
    //L, T, w <- tuneHyperparameters(data)
    ll L, T;
    ld w;
    
    // Hasher of L * T hyperplanes generated for hashing the data points
     hasher = new HashTables(L, T, w);
    
    // Hashing the data points and computing the dictionary with the estimators per hash
    unordered_map<InnerMap, ld, InnerMapHash, InnerMapEqual> estPerHash = hasher->HashAndEstimatePerHash(data);

    // Computing the threshold for the anomaly detection using the estimators calculated
    threshold = findThreshold(estPerHash, anomalyRatio);
  }

  ld findThreshold(unordered_map<InnerMap, ld, InnerMapHash, InnerMapEqual> estPerHash, ll anomalyRatio){
    //threshold <- 0
    ld threshold = 0;
    
    vector<ld> estimates;
    // Get all the estimators from the hash tables in a list
    for (const auto& est : estPerHash) {
      estimates.push_back(est.second);
    }

    // Sort the estimators
    sort(estimates.begin(), estimates.end());
    
    // Get the estimator that corresponds to the anomaly ratio
    size_t index = static_cast<size_t>((1 - anomalyRatio) * estimates.size());

    return estimates[index];
  }

};
