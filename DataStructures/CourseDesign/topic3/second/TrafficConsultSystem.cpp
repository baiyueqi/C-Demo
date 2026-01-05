#include "TrafficConsultSystem.h"
#include <algorithm>

vector<int> TrafficConsultSystem::dijkstra(int start, vector<int>& prev, int skip){
    int n = g.n;
    vector<int> dis(n, INT_MAX/2);
    vector<bool> visited(n,false);
    prev.assign(n,-1);
    dis[start]=0;
    for(int i=0;i<n;i++){
        int u=-1;
        for(int j=0;j<n;j++){
            if(!visited[j] && (u==-1 || dis[j]<dis[u])) u=j;
        }
        if(u==-1) break;
        if(u==skip) continue;
        visited[u]=true;
        for(int v=0;v<n;v++){
            if(v==skip) continue;
            if(dis[u]+g.dist[u][v]<dis[v]){
                dis[v]=dis[u]+g.dist[u][v];
                prev[v]=u;
            }
        }
    }
    return dis;
}

vector<vector<int>> TrafficConsultSystem::floyd(vector<vector<int>>& next,int skip){
    int n=g.n;
    vector<vector<int>> d = g.dist;
    next.assign(n, vector<int>(n,-1));
    for(int i=0;i<n;i++)
        for(int j=0;j<n;j++)
            if(d[i][j]<INT_MAX/2 && i!=skip && j!=skip) next[i][j]=j;

    for(int k=0;k<n;k++){
        if(k==skip) continue;
        for(int i=0;i<n;i++){
            if(i==skip) continue;
            for(int j=0;j<n;j++){
                if(j==skip) continue;
                if(d[i][k]+d[k][j]<d[i][j]){
                    d[i][j]=d[i][k]+d[k][j];
                    next[i][j]=next[i][k];
                }
            }
        }
    }
    return d;
}

vector<pair<vector<int>,int>> TrafficConsultSystem::getAllPathsLengths(int start,int end,int skip){
    vector<int> path = {start};
    vector<vector<int>> allPaths;
    g.dfs(start,end,path,allPaths,10);
    vector<pair<vector<int>,int>> results;
    for(auto &p: allPaths){
        int total=0;
        bool valid=true;
        for(int i=0;i<p.size()-1;i++){
            if(p[i]==skip || p[i+1]==skip){ valid=false; break; }
            total+=g.dist[p[i]][p[i+1]];
        }
        if(valid) results.push_back({p,total});
    }
    sort(results.begin(),results.end(),[](auto &a, auto &b){ return a.second<b.second; });
    return results;
}
