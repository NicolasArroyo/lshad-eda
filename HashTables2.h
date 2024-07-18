#ifndef LSHAD_EDA_HASHTABLES_H
#define LSHAD_EDA_HASHTABLES_H

#include <unordered_set>
typedef long long ll;
typedef long double ld;

#include <vector>
#include <random>
#include <cmath>
#include "hashes.h"
#include <unordered_map>
#include <algorithm>

using namespace std;

inline ld dot_product(const vector<ld> &v1, const vector<ld> &v2) {
  ld result = 0;

  for (ll i = 0; i < (ll) v1.size(); ++i) {
    result += v1[i] * v2[i];
  }

  return result;
}

class RandomProjection {
private:
  // Generates Alpha, a random vector drawn from a Gaussian distribution
  static vector<ld> generate_alpha(const ll &n_dims, const ld &mean = 0.0, const ld &stddev = 1.0) {
    random_device rd;
    default_random_engine generator(rd());
    normal_distribution<ld> distribution(mean, stddev);

    vector<ld> alpha(n_dims);
    for (ll i = 0; i < n_dims; ++i) {
      alpha[i] = distribution(generator);
    }

    return alpha;
  }

  // Generates Beta, a real number uniformly chosen from the interval [0:w]
  static ld generate_beta(const ld &w) {
    random_device rd;
    default_random_engine generator(rd());
    uniform_real_distribution<ld> distribution(0.0, w);

    ld beta = distribution(generator);

    return beta;
  }

public:
  // Calculates the values of the random projection that will map a d dimensional vector x onto the set of integers
  static pair<vector<ld>, ld> calculate_projection(const ll &dim, const ld &w) {
    vector<ld> alpha = generate_alpha(dim);
    ld beta = generate_beta(w);
    
    return make_pair(alpha, beta);
  }
};

class HashFunction {
public:
  // Generates a hash function, represented by L random projections, defining the hash value
  static vector<pair<vector<ld>, ld>> generate_hash_function(const ll &dim, const ld &w, const ll &L) {
    vector<pair<vector<ld>, ld>> hash_function(L);

    for (ll i = 0; i < L; ++i) {
      hash_function[i] = RandomProjection::calculate_projection(dim, w);
    }

    return hash_function;
  }
};

class HashTables {
private:
  // Vector of T hash tables
  // Each hash table uses an unordered map to map a hash value to a vector of data points
  vector<unordered_map<InnerHash, vector<vector<ld>>, InnerMapHash, InnerMapEqual>> tables;

  // Vector of L random projections for each hash table
  vector<vector<pair<vector<ld>, ld>>> random_projections;

  vector<vector<vector<ll>>> hashes_per_points;

  // L: Number of random projections for each hash function
  // T: Number of hash tables
  // Size of the quantization bins used for the random projections
  ll L, T;
  ld w;
  ll DIM;

public:
  HashTables(ll L, ll T, ld w, ll dim) : L(L), T(T), w(w), DIM(dim) {
    tables.resize(T);
    random_projections.resize(T);
    for(int i = 0; i < T; ++i){
      random_projections[i].resize(L);
      // Generates L random projections for each hass table
      random_projections[i] = HashFunction::generate_hash_function(DIM, w, L);
    }
  }

  const vector<unordered_map<InnerHash, vector<vector<ld>>, InnerMapHash, InnerMapEqual>> &getTables() const {
    return tables;
  }

  const vector<vector<vector<ll>>> &getHashesPerPoints() const {
    return hashes_per_points;
  }

  void print() {
    int idx = 0;
    for(auto table : tables) {
      cout << "\n------- Table: " << idx++ << "-------" << endl;
      for(auto bucket : table) {
        cout << "Bucket: ";
        for (auto hash : bucket.first) {
          cout << hash << " ";
        }
        cout << ": " << endl;
        for(auto point : bucket.second) {
          cout << "[";
          for(auto coord : point) {
            cout << coord << " ";
          }
          cout << "]" << endl;
        }
        cout << endl;
      }
    }
  }

  vector<ll> hash(const vector<ld> &x, const ll t){
    vector<ll> hash_values(L);
    for (ll l = 0; l < L; ++l) {
      ld numerator = dot_product(x, random_projections[t][l].first) + random_projections[t][l].second;
      hash_values[l] = floor(numerator / w);
    }

    return hash_values;
  }

  void insert(const vector<ld> &x) {
    // For each of the T hash tables...
    vector<vector<ll>> hashes_per_point;
    for (ll t = 0; t < T; ++t) {
      // Generates the hash value based on the L random projections of the table
      vector<ll> hash_value = hash(x, t);
      hashes_per_point.push_back(hash_value);

      // And inserts the data point in the corresponding bucket
      auto bucket = tables[t].find(hash_value);
      if (bucket == tables[t].end()) {
        tables[t][hash_value] = {x};
      } else {
        bucket->second.push_back(x);
      }
    }
    hashes_per_points.push_back(hashes_per_point);
  }

  // Gets the total number of buckets in the hash tables and the sum of the sizes of all buckets
  pair<ll, ll> getNumberBucketsAndSumBucketSizes() {
    ll numberBuckets = 0;
    ll sumBucketSizes = 0;

    for (const auto &table: tables) {
      for (const auto &bucket: table) {
        numberBuckets++;
        sumBucketSizes += bucket.second.size();
      }
    }

    return make_pair(numberBuckets, sumBucketSizes);
  }

  ld countNeighbors(const vector<ld> &point){
    ll count = 0;

    // Using a set to store the neighbors without duplicates
    unordered_set<vector<ld>, VectorHash, VectorEqual> neighbors;
    neighbors.insert(point);
    // Go trough all the tables
    for (const auto &table: tables){
      // Go through all the buckets in the hash table
      for (const auto& bucket : table) {
        auto finded = find(bucket.second.begin(), bucket.second.end(), point);
        if(finded != bucket.second.end()){
          neighbors.insert(bucket.second.begin(), bucket.second.end());
        }
      }
    }

    // cout << "\tNeighbors: ";
    // for(auto p: point){
    //   cout << p << ", ";
    // }
    // cout << endl;
    // for(auto n : neighbors){
    //   cout << "\t[";
    //   for(auto l : n){
    //     cout << l << ", ";
    //   }
    //   cout << "]" << endl;
    // }
    // cout << "Neighbors size: " << neighbors.size() << endl;
    // cout << endl;
    // Return the number of neighbors
    return neighbors.size() - 1;
  }

  unordered_map<InnerHash, ld, InnerMapHash, InnerMapEqual> HashAndEstimatePerHash(const vector<vector<ld>> &data) {
    // Unordered map personalized for mapping each hash table to its estimator

    unordered_map<InnerHash, ld, InnerMapHash, InnerMapEqual> estPerHash;

    //Hashing the data points
    for (ll i = 0; i < (ll) data.size(); ++i) {
      insert(data[i]);
    }

    // Generating the estimator for each hash table
    for (const auto& table: tables) {

      for (const auto& bucket : table) {
        // Calculating the number of elements in the bucket
        ld EA = 0.0, EB = 0.0;
        EA = bucket.second.size();
        // cout << "\n------------------------------" << endl;
        // cout << "Bucket: ";
        // for(auto l : bucket.first){
        //   cout << l << " ";
        // }
        // cout << endl;
        // print_bucket(bucket.second);

        // Calculating the number of neighbors of each element in the bucket
        for (const auto& point : bucket.second) {
          ld neighborCount = countNeighbors(point);
          // cout << "neighborCount: " << neighborCount << endl;
          // cout << "Prev EB: " << EB << endl;
          EB += neighborCount;
          // cout << "EB: " << EB << endl;
        }
        // Computing the EB estimator
        // cout << "neighborCountTotal: " << EB << endl;
        EB = EB / EA;
        // cout << "EA: " << EA << " EB: " << EB << endl;
        estPerHash[bucket.first] = EB > 0.0 ? EA / EB : 0.0;
        // estPerHash[bucket.first] = EB;
        // cout << "Estimator: " << estPerHash[bucket.first] << endl;
        // cout << "------------------------------" << endl;
      }
    }

    return estPerHash;
  }

  // ONLY FOR TESTING PURPOSES
  unordered_set<vector<ld>, VectorHash, VectorEqual> search(const vector<ld> &x) {
    unordered_set<vector<ld>, VectorHash, VectorEqual> results;

    // For each of the T hash tables...
    for (ll t = 0; t < T; ++t) {
      // Generates a hash function made of L random projections...
      vector<ll> hash_value = hash(x, t);

      // Collects all data points that shares the same bucket as our query data point...
      auto bucket = tables[t].find(hash_value);
      if (bucket != tables[t].end()) {
        results.insert(bucket->second.begin(), bucket->second.end());
      }
    }

    return results;
  }

  vector<InnerHash> search_tables(const vector<ld> &x) {
    vector<InnerHash> results;

    // cout << "Cantidad de tablas: " << T << "\n";
    for (ll t = 0; t < T; ++t) {
      vector<ll> hash_value = hash(x, t);

      for (ll l = 0; l < L; ++l) {
        auto it = tables[t].find(hash_value);
        if (it != tables[t].end()) {
          results.push_back(it->first);
        }
      }
    }

    sort(results.begin(), results.end());
    results.erase(unique(results.begin(), results.end()), results.end());

    return results;
  }

  void print_bucket(const vector<vector<ld>> &bucket) {
    for (const auto &point : bucket) {
      cout << "[";
      for (ll j = 0; j < DIM; ++j) {
        cout << point[j];
        if (j < DIM - 1) cout << ", ";
      }
      cout << "]" << endl;
    }
  }

  void print_hash_buscket(const vector<ll> &hash) {
    for (ll i = 0; i < T; ++i) {
      auto it = tables[i].find(hash);
      if (it != tables[i].end()) {
        cout << "Table: " << i << endl;
        for (const auto &point : it->second) {
          cout << "[";
          for (ll j = 0; j < DIM; ++j) {
            cout << point[j];
            if (j < DIM - 1) cout << ", ";
          }
          cout << "]" << endl;
        }
      }
    }
  }
};

#endif //LSHAD_EDA_HASHTABLES_H
