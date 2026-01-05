#pragma once
#include "Graph.h"

class TrafficConsultSystem{
public:
    Graph g;
    vector<int> dijkstra(int start, vector<int>& prev, int skip=-1);
    vector<vector<int>> floyd(vector<vector<int>>& next, int skip=-1);
    vector<pair<vector<int>,int>> getAllPathsLengths(int start,int end,int skip=-1);
};
