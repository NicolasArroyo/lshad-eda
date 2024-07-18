#pragma once
#include "HashTables2.h"
#include "hashes.h"
#include <unordered_map>
#include <tuple>

using namespace std;

class LSHAD {
  HashTables *hasher;
  unordered_map<InnerHash, ld, InnerMapHash, InnerMapEqual> estPerHash;
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
      cout << "\n-----------------------------------------------" << endl;
      cout << "Hash: ";
      for (ll i : est.first) {
        cout << i << " ";
      }
      cout << ": " << endl;
      hasher->print_hash_buscket(est.first);
      cout << "Estimator: " << est.second << endl;
      cout << "-----------------------------------------------" << endl;
    }
  }

  // Training phase of the LSHAD algorithm
  void train(const vector<vector<ld>> data, ld anomalyRatio){
    // std::cout << std::setprecision(20);
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
    threshold = findThreshold(estPerHash, anomalyRatio,data);
    
    cout << "Threshold: " << threshold << endl;
  }

  ld findThreshold(unordered_map<InnerHash, ld, InnerMapHash, InnerMapEqual> estPerHash, ld anomalyRatio, vector<vector<ld>>data){
    
    vector<ld> estimators;
    // Get all the estimators from the hash tables in a list
    for (auto point:data){
      vector<InnerHash> hashes = hasher->search_tables(point);

      ld estimator = 0;
      for (const auto &hash : hashes)
      {
        estimator += estPerHash[hash];
      }
      estimators.push_back(estimator);
    }

    ll idx = 0;

    // for (auto point: this->hasher->getHashesPerPoints()){
    //   ld sum = 0;
    //   for(auto pointHash: data[idx]){
    //     cout << pointHash << ", ";
    //   }
    //   cout << endl;
    //   idx++;
    //   for(auto hash: point){
    //     sum += estPerHash[hash];
    //     // cout << "Hash: ";
    //     // for(auto i: hash){
    //     //   cout << i << " ";
    //     // }
    //     // cout << "Estimator: " << estPerHash[hash] << endl;
    //   }
    //   // cout << "Sum: " << sum << endl;
    //   estimators.push_back(sum);
    // }

    sort(estimators.begin(), estimators.end());
    
    // Get the estimator that corresponds to the anomaly ratio
    ld index = static_cast<size_t>(estimators.size() * (1-anomalyRatio));
    index = round(index);
    
    if (index > estimators.size() - 1) {
      index = estimators.size() - 1;
    }
    return estimators[index];
  }
  
  bool detection_phase(const vector<ld> point) {
    vector<InnerHash> hashes = hasher->search_tables(point);
    cout << "hashes.size(): " << hashes.size() << endl;
    cout << "Point: ";
    for(auto i: point){
      cout << i << " ";
    }
    ld estimator = 0;

    for (const auto& hash: hashes) {
      // cout << "-> " << estPerHash[hash] << endl;
      estimator += estPerHash[hash];
    }

    cout << "Estimator: " << estimator << endl;

    return estimator >= threshold;
  }
};

