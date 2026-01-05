#pragma once
#include <bits/stdc++.h>
using namespace std;

class Graph{
public:
    int n;
    vector<string> cityNames;
    unordered_map<string,int> cityMap;
    vector<vector<int>> dist;
    vector<vector<int>> adj;

    Graph(){}
    void loadDistances(const string &filename);
    void loadAdjacency(const string &filename);
    void dfs(int u,int target,vector<int>& path, vector<vector<int>>& allPaths, int maxDepth=10);
};
