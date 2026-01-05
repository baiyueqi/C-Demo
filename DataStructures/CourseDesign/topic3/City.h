#ifndef CITY_H
#define CITY_H

#include <string>
#include <vector>
#include <utility>

struct City {
    std::string name;
    std::vector<std::pair<int, double>> neighbors;
};

struct Path {
    std::vector<int> nodes;
    double totalDistance;
    int nodeCount;
    
    bool operator>(const Path& other) const {
        return totalDistance > other.totalDistance;
    }
};

#endif