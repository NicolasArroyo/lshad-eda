#ifndef LSHAD_EDA_HASHTABLES_H
#define LSHAD_EDA_HASHTABLES_H

typedef long long ll;
typedef long double ld;

#include <vector>
#include <random>
#include <cmath>
#include <unordered_map>
#include <algorithm>

using namespace std;

ld dot_product(const vector<ld> &v1, const vector<ld> &v2) {
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
};

#endif //LSHAD_EDA_HASHTABLES_H
