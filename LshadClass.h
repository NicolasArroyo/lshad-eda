#pragma once
#include "HashTables.h"
#include "hashes.h"
#include <unordered_map>
#include <tuple>

using namespace std;

class LSHAD {
  HashTables *hasher;
  unordered_map<InnerMap, ld, InnerMapHash, InnerMapEqual> estPerHash;
  //unordered_map<pair<ll, vector<vector<ld>>>, ld> estPerHash;
  ld threshold;

public:
  LSHAD(): hasher(nullptr), estPerHash(unordered_map<InnerMap, ld, InnerMapHash, InnerMapEqual>()), threshold(0){}
  ~LSHAD(){
    delete hasher;
  }

  ld hashGroupAndCount(const vector<vector<ld>> data, ll L, ll T, ld wCandidate) {
    ld averageBucketSize;

    HashTables *tempHasher = new HashTables(L, T, wCandidate);
    for (const auto& vec: data) {
        tempHasher->insert(vec);
    }

    pair<ll, ll> p = tempHasher->getNumberBucketsAndSumBucketSizes();
    ll BC = p.first;
    ll sumBucketSizes = p.second;
    averageBucketSize = ( static_cast<ld>(sumBucketSizes) / static_cast<ld>(BC) ) / static_cast<ld>(data.size());  // (sumBucketSizes / BC) / |data|

    delete tempHasher;

    return averageBucketSize;
  }

  tuple<ll, ll, ld> tuneHyperparameters(const vector<vector<ld>> data){
    ll L = 4;
    ll T = 50;

    ld wCandidate = 1;
    ld avBucketSize = 0;
    ll leftLimit = 1;
    ll rightLimit = 1;

    while (avBucketSize < 0.05) {
        avBucketSize = hashGroupAndCount(data, L, T, wCandidate);
        wCandidate *= 2;
    }
    

    rightLimit = wCandidate;
    while (avBucketSize < 0.05 || avBucketSize > 0.1) {
        wCandidate = floor((leftLimit + rightLimit) / 2);
        avBucketSize = hashGroupAndCount(data, L, T, wCandidate);

        if (avBucketSize < 0.05) {
            leftLimit = wCandidate;
        }
        else if (avBucketSize > 0.1) {
            rightLimit = wCandidate;
        }

        if (leftLimit >= rightLimit) {
            break;
        }
    }

    return make_tuple(L, T, wCandidate);
  }

  // Training phase of the LSHAD algorithm
  void train(const vector<vector<ld>> data, ld anomalyRatio){
    tuple<ll, ll, ld> hyperparameters = tuneHyperparameters(data);
    ll L = get<0>(hyperparameters);
    ll T = get<1>(hyperparameters);
    ld w = get<2>(hyperparameters);

    // Hasher of L * T hyperplanes generated for hashing the data points
    hasher = new HashTables(L, T, w);

    // Hashing the data points and computing the dictionary with the estimators per hash
    unordered_map<InnerHash, ld, InnerMapHash, InnerMapEqual> estPerHash = hasher->HashAndEstimatePerHash(data);

    // Computing the threshold for the anomaly detection using the estimators calculated
    threshold = findThreshold(estPerHash, anomalyRatio);
  }

  ld findThreshold(unordered_map<InnerHash, ld, InnerMapHash, InnerMapEqual> estPerHash, ll anomalyRatio){
    ld threshold = 0;
    
    vector<ld> estimates;
    // Get all the estimators from the hash tables in a list
    for (const auto& est : estPerHash) {
      estimates.push_back(est.second);
    }

    // Sort the estimators
    sort(estimates.begin(), estimates.end());
    
    // Get the estimator that corresponds to the anomaly ratio
    size_t index = static_cast<size_t>(anomalyRatio * estimates.size());

    return estimates[index];
  }

  bool detection_phase(const vector<ld> point) {
    // auto hashes = hasher->getHashes(point);  TODO: Implement getHashes
    vector<InnerMap> hashes;
    ld estimator = 0;

    for (const auto& hash: hashes) {
        estimator += estPerHash[hash];
    }

    return estimator < threshold;
  }
};

