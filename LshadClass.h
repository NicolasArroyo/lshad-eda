#pragma once
#include "HashTables2.h"
#include "hashes.h"
#include <unordered_map>
#include <tuple>

using namespace std;

class LSHAD {
  HashTables *hasher;
  unordered_map<InnerHash, ld, InnerMapHash, InnerMapEqual> estPerHash;
  // unordered_map<vector<ll>, ld> estPerHash;
  ld threshold;

public:
  LSHAD(): hasher(nullptr), threshold(0){}
  ~LSHAD(){
    delete hasher;
  }

  ld hashGroupAndCount(const vector<vector<ld>> data, ll L, ll T, ld wCandidate) {
    ld averageBucketSize;

    HashTables *tempHasher = new HashTables(L, T, wCandidate, data[0].size());
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

  void print_EstPerHash(){
    for (const auto& est : estPerHash) {
      cout << "Hash: ";
      for (ll i : est.first) {
        cout << i << " ";
      }
      cout << "Estimator: " << est.second << endl;
    }
  }

  // Training phase of the LSHAD algorithm
  void train(const vector<vector<ld>> data, ld anomalyRatio){
    tuple<ll, ll, ld> hyperparameters = tuneHyperparameters(data);
    ll L = get<0>(hyperparameters);
    ll T = get<1>(hyperparameters);
    ld w = get<2>(hyperparameters);
    cout << "L: " << L << " T: " << T << " w: " << w << endl;

    // Hasher of L * T hyperplanes generated for hashing the data points
    hasher = new HashTables(L, T, w, data[0].size());
    
    // Hashing the data points and computing the dictionary with the estimators per hash
    estPerHash = hasher->HashAndEstimatePerHash(data);
    cout << "Hasher size: " << hasher->getTables().size() << endl;
    // hasher->print();
    // print_EstPerHash();

    // Computing the threshold for the anomaly detection using the estimators calculated
    threshold = findThreshold(estPerHash, anomalyRatio);

    cout << "Threshold: " << threshold << endl;
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
    size_t index = static_cast<size_t>((1 - anomalyRatio) * estimates.size());

    return estimates[index];
  }
  
  bool detection_phase(const vector<ld> point) {
    // auto hashes = hasher->getHashes(point);  TODO: Implement getHashes
    vector<InnerHash> hashes = hasher->search_tables(point);
    cout << "hashes.size(): " << hashes.size() << endl;
    ld estimator = 0;

    for (const auto& hash: hashes) {
      // cout << "-> " << estPerHash[hash] << endl;
        estimator += estPerHash[hash];
    }

    return estimator < threshold;
  }
};

