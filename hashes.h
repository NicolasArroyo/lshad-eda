#pragma once

#include <unordered_map>
#include <vector>
#include <iostream>
#include <functional>
typedef long long ll;
typedef long double ld;

using namespace std;
using InnerHash = vector<ll>;

struct InnerMapHash {
  size_t operator()(const InnerHash& innerHash) const {
    size_t hashValue = 0;

    hashValue = hashVectors(innerHash);

    return hashValue;
  }

private:
  size_t hashVectors(const vector<ll>& vec) const {
    size_t hashValue = 0;
    std::hash<ld> hashFn;
    for (ll i : vec) {
      hashValue ^= hashFn(i) + 0x9e3779b9 + (hashValue << 6) + (hashValue >> 2);
    }
    return hashValue;
  }
};

// Función de comparación de igualdad personalizada para InnerMap
struct InnerMapEqual {
  bool operator()(const InnerHash& lhs, const InnerHash& rhs) const {
    return lhs == rhs;
  }
};


struct VectorHash {
  size_t operator()(const vector<ld>& v) const {
    std::hash<ld> hasher;
    size_t seed = 0;
    for (ld i : v) {
      seed ^= hasher(i) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    }
    return seed;
  }
};

struct VectorEqual {
  bool operator()(const vector<ld>& lhs, const vector<ld>& rhs) const {
    return lhs == rhs;
  }
};
