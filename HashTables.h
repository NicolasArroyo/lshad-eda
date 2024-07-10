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
    static vector<ld> generate_alpha(const ll &n_dims, const ld &mean = 0.0, const ld &stddev = 100.0) {
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
    // Calculates a projection that maps a d dimensional vector x onto the set of integers
    static ll calculate_projection(const vector<ld> &x, const ld &w) {
        vector<ld> alpha = generate_alpha((ll) x.size());
        ld beta = generate_beta(w);

        ld numerator = dot_product(x, alpha) + beta;
        ld result = floor(numerator / w);

        return static_cast<ll>(result);
    }
};

class HashFunction {
public:
    // Generates a hash function, represented by L random projections, defining the hash value
    static vector<ll> generate_hash_function(const vector<ld> &x, const ld &w, const ll &L) {
        vector<ll> hash_function(L);

        for (ll i = 0; i < L; ++i) {
            hash_function[i] = RandomProjection::calculate_projection(x, w);
        }

        return hash_function;
    }
};

class HashTables {
private:
    // Vector of T hash tables
    // Each hash table uses an unordered map to map a hash value to a vector of data points
    vector<unordered_map<ll, vector<vector<ld>>>> tables;

    // L: Number of random projections for each hash function
    // T: Number of hash tables
    // Size of the quantization bins used for the random projections
    ll L, T;
    ld w;

public:
    HashTables(ll L, ll T, ld w) : L(L), T(T), w(w) {
        tables.resize(T);
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

    void insert(const vector<ld> &x) {
        // For each of the T hash tables...
        for (ll t = 0; t < T; ++t) {
            // Generates a hash function made of L random projections...
            vector<ll> hash_value = HashFunction::generate_hash_function(x, w, L);

            // And inserts the data point in the corresponding bucket
            for (ll l = 0; l < L; ++l) {
                tables[t][hash_value[l]].push_back(x);
            }
        }
    }
    
    ll countNeighbors(const vector<ld> &point){
      ll count = 0;

      // Using a set to store the neighbors without duplicates
      unordered_set<vector<ld>, VectorHash, VectorEqual> neighbors;
      neighbors.insert(point);
      // Go trough all the tables
      for (const auto &table: tables){
          // Go through all the buckets in the hash table
          for (const auto& bucket : table) {
              // Find the point in the bucket
              auto finded = find(bucket.second.begin(), bucket.second.end(), point);
              // If the point is in the bucket, insert all the points in the bucket in the neighbors set
              if(finded != bucket.second.end()){
                  neighbors.insert(bucket.second.begin(), bucket.second.end());
              }
          }
      }
      
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
            ld EA = 0.0, EB = 0.0;

            for (const auto& bucket : table) {
                // Calculating the number of elements in the bucket
                EA = bucket.second.size();
                
                // Calculating the number of neighbors of each element in the bucket
                for (const auto& point : bucket.second) {
                    ll neighborCount = countNeighbors(point);
                    EB += neighborCount;
                }
                // Computing the EB estimator
                EB = EB / EA;
                estPerHash[make_pair(bucket.first, bucket.second)] = EA / EB;
            }
        }

        return estPerHash;
    }

    // ONLY FOR TESTING PURPOSES
    vector<vector<ld>> search(const vector<ld> &x) {
        vector<vector<ld>> results;

        // For each of the T hash tables...
        for (ll t = 0; t < T; ++t) {
            // Generates a hash function made of L random projections...
            vector<ll> hash_value = HashFunction::generate_hash_function(x, w, L);

            // Collects all data points that shares the same bucket as our query data point...
            for (ll l = 0; l < L; ++l) {
                if (tables[t].find(hash_value[l]) != tables[t].end()) {
                    results.insert(results.end(), tables[t][hash_value[l]].begin(), tables[t][hash_value[l]].end());
                }
            }
        }

        sort(results.begin(), results.end());
        results.erase(unique(results.begin(), results.end()), results.end());

        return results;
    }


    vector<InnerHash> search_tables(const vector<ld> &x) {
      vector<InnerHash> results;

      cout << "Cantidad de tablas: " << T << "\n";
      // For each of the T hash tables...
      for (ll t = 0; t < T; ++t) {
          // Generates a hash function made of L random projections...
          vector<ll> hash_value = HashFunction::generate_hash_function(x, w, L);

          // Collects all data points that shares the same bucket as our query data point...
          for (ll l = 0; l < L; ++l) {
            if (tables[t].find(hash_value[l]) != tables[t].end()) {
              results.emplace_back(hash_value[l], tables[t][hash_value[l]]);
                // results.emplace_back(results.end(), tables[t][hash_value[l]].begin(), tables[t][hash_value[l]].end());
            }
          }
      }

    sort(results.begin(), results.end(), [](const InnerHash &a, const InnerHash &b) {
        return a.first < b.first;});

    results.erase(unique(results.begin(), results.end(), [](const InnerHash &a, const InnerHash &b) {
        return a.first == b.first;}), results.end());

    return results;
    }
};

#endif //LSHAD_EDA_HASHTABLES_H
