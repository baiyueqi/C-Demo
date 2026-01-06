#include "TrafficConsultSystem.h"
#include <algorithm>
#include <queue>
#include <iostream>
using namespace std;

//Dijkstra：单源最短路径（可跳过某城市）
vector<int> TrafficConsultSystem::dijkstra(int start, vector<int>& prev, int skip){
    //从 start 出发，计算到 所有城市的最短距离,prev[v]：最短路径中，v 的前驱节点,skip：要避开的城市（模拟封城/禁行）
    int n = g.n;
    vector<int> dis(n, INT_MAX/2);//start → 各点最短距离
    vector<bool> visited(n,false);//是否已确定最短路
    prev.assign(n,-1);//路径回溯
    dis[start]=0;
    for(int i=0;i<n;i++){
        //找当前“未访问且距离最小”的点
        int u=-1;
        for(int j=0;j<n;j++){
            if(!visited[j] && (u==-1 || dis[j]<dis[u])) u=j;
        }
        //u == skip：这个城市直接跳过（模拟禁行）
        if(u==-1) break;
        if(u==skip) continue;
        visited[u]=true;
        //更新邻接点距离
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

//Floyd：多源最短路径（可跳过某城市）
vector<vector<int>> TrafficConsultSystem::floyd(vector<vector<int>>& next,int skip){
    int n=g.n;
    vector<vector<int>> d = g.dist;
    next.assign(n, vector<int>(n,-1));
    for(int i=0;i<n;i++)//d[i][j]：当前最短距离,next[i][j]：路径恢复用
        for(int j=0;j<n;j++)
            if(d[i][j]<INT_MAX/2 && i!=skip && j!=skip) next[i][j]=j;

    //三重循环（Floyd 核心）
    for(int k=0;k<n;k++){//k：是否经过中转点 k
        if(k==skip) continue;//跳过禁行城市
        for(int i=0;i<n;i++){
            if(i==skip) continue;
            for(int j=0;j<n;j++){
                if(j==skip) continue;//i → j = min( i → j, i → k → j )
                if(d[i][k]+d[k][j]<d[i][j]){
                    d[i][j]=d[i][k]+d[k][j];
                    next[i][j]=next[i][k];
                }
            }
        }
    }
    return d;//返回最短距离矩阵
}

//getAllPathsLengths：枚举所有路径并排序
vector<pair<vector<int>,int>> TrafficConsultSystem::getAllPathsLengths(int start,int end,int skip){
    vector<int> path = {start};
    vector<vector<int>> allPaths;
    g.dfs(start,end,path,allPaths,10);//用 DFS 找所有路径,枚举所有路径，限制路径长度不超过 10
    vector<pair<vector<int>,int>> results;
    for(auto &p: allPaths){
        //计算每条路径长度
        int total=0;
        bool valid=true;
        for(int i=0;i<p.size()-1;i++){
            if(p[i]==skip || p[i+1]==skip){ valid=false; break; }//路径中经过禁行城市 → 无效
            total+=g.dist[p[i]][p[i+1]];//累加路径长度
        }
        if(valid) results.push_back({p,total});//有效路径加入结果
    }
    sort(results.begin(),results.end(),[](auto &a, auto &b){ return a.second<b.second; });//按路径长度排序(升序)
    return results;
}

//verifyWuhanCenter：验证“武汉是全国中心”
//问题建模：全国省会城市到武汉的最少中转数不超过2个=无权图最短路径走过的边数(hop = 城市间“跳”的次数)
void TrafficConsultSystem::verifyWuhanCenter() {
    int n = g.n;
    vector<int> dist(n, -1);

    int wuhan = g.cityMap["武汉"];

    queue<int> q;
    dist[wuhan] = 0;
    q.push(wuhan);

    // BFS(BFS 第一次到达某节点的路径，一定是 hop 最少的路径)
    while (!q.empty()) {
        int u = q.front(); q.pop();
        for (int v : g.adj[u]) {
            if (dist[v] == -1) {//dist[i]：武汉到 i 的最少边数
                dist[v] = dist[u] + 1;
                q.push(v);
            }
        }
    }

    // 排除特殊城市
    vector<string> excluded = {"香港", "澳门", "台北", "海口"};

    bool ok = true;

    cout << "\n【验证结果：省会城市到武汉的最少中转数】\n";
    for (int i = 0; i < n; ++i) {
        string city = g.cityNames[i];

        // 排除
        if (find(excluded.begin(), excluded.end(), city) != excluded.end())
            continue;
        //验证每个省会
        int hop = dist[i];
        int transfer = hop - 1;

        cout << city << " -> 武汉 : ";
        cout << "hop = " << hop << ", 中转省会 = " << transfer;

        if (hop > 3) {
            cout << " ❌ 超过 2 个中转\n";
            ok = false;
        } else {
            cout << " ✅ 符合\n";
        }
    }

    cout << "\n===========================\n";
    if (ok) {
        cout << "【结论】命题成立：\n";
        cout << "全国其他省会城市（不含港澳、台北、海口）到武汉\n";
        cout << "最少路径中转省会城市数均不超过 2 个。\n";
    } else {
        cout << "【结论】命题不成立：存在城市中转超过 2 个。\n";
    }
    cout << "===========================\n";
}