#pragma once

#include <unordered_map>
#include <vector>
#include <iostream>
#include <functional>
typedef long long ll;
typedef long double ld;

using namespace std;
using InnerMap = unordered_map<ll, vector<vector<ld>>>;

struct InnerMapHash {
  size_t operator()(const InnerMap& innerMap) const {
    size_t hashValue = 0;
    std::hash<ll> hashFn1;
    std::hash<ld> hashFn2;

    for (const auto& pair : innerMap) {
      hashValue ^= hashFn1(pair.first) ^ (hashVectors(pair.second) << 1);
    }

    return hashValue;
  }

private:
  size_t hashVectors(const vector<vector<ld>>& vec) const {
    size_t hashValue = 0;
    std::hash<ld> hashFn;
    for (const auto& innerVec : vec) {
      for (const auto& element : innerVec) {
        hashValue ^= hashFn(element);
      }
    }
    return hashValue;
  }
};

// Función de comparación de igualdad personalizada para InnerMap
struct InnerMapEqual {
  bool operator()(const InnerMap& lhs, const InnerMap& rhs) const {
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
