#pragma once
#include <bits/stdc++.h>
using namespace std;

class Graph{
public:
    int n;//城市数量
    vector<string> cityNames;//编号 → 城市名,如0 → "北京"
    unordered_map<string,int> cityMap;//是 cityNames 的反向映射,如"北京" → 0 
    vector<vector<int>> dist;//距离矩阵(二维数组),dist[i][j] = 城市 i → 城市 j 的距离
    vector<vector<int>> adj;//邻接矩阵(图的结构描述),adj[i][j] = 1 表示城市 i 和城市 j 之间有道路相连,否则为 0

    Graph(){}
    void loadDistances(const string &filename);//从文件中读取城市间距离矩阵
    void loadAdjacency(const string &filename);//从文件中读取城市间邻接矩阵
    void dfs(int u,int target,vector<int>& path, vector<vector<int>>& allPaths, int maxDepth=10);//深度优先搜索寻找所有路径
};
