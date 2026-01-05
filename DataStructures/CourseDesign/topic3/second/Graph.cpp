#include "Graph.h"
#include <fstream>
#include <sstream>

void Graph::loadDistances(const string &filename){
    ifstream fin(filename);
    string line;
    getline(fin,line);
    stringstream ss(line);
    string temp;
    getline(ss,temp,','); // skip first column
    n=0;
    while(getline(ss,temp,',')){
        cityMap[temp]=n++;
        cityNames.push_back(temp);
    }

    dist.assign(n, vector<int>(n, INT_MAX/2));
    for(int i=0;i<n;i++) dist[i][i]=0;
    adj.assign(n, vector<int>());

    int row=0;
    while(getline(fin,line)){
        stringstream ss2(line);
        string cityName;
        getline(ss2,cityName,',');
        int u = cityMap[cityName];
        for(int col=0;col<n;col++){
            string val;
            getline(ss2,val,',');
            dist[u][col] = stoi(val);
        }
        row++;
    }
}

void Graph::loadAdjacency(const string &filename){
    ifstream fin(filename);
    string line;
    getline(fin,line); // skip header
    int row=0;
    while(getline(fin,line)){
        stringstream ss(line);
        string cityName;
        getline(ss,cityName,',');
        int u = cityMap[cityName];
        for(int col=0;col<n;col++){
            string val;
            getline(ss,val,',');
            if(stoi(val)==1) adj[u].push_back(col);
        }
        row++;
    }
}

void Graph::dfs(int u,int target,vector<int>& path, vector<vector<int>>& allPaths,int maxDepth){
    if(path.size()>maxDepth) return;
    if(u==target){
        allPaths.push_back(path);
        return;
    }
    for(int v: adj[u]){
        if(find(path.begin(),path.end(),v)!=path.end()) continue;
        path.push_back(v);
        dfs(v,target,path,allPaths,maxDepth);
        path.pop_back();
    }
}
