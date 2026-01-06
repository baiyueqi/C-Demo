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
    while(getline(ss,temp,',')){//循环读取城市名
        cityMap[temp]=n++;
        cityNames.push_back(temp);
    }

    dist.assign(n, vector<int>(n, INT_MAX/2));//初始化距离矩阵(初始化为一个 n×n 的大数)
    for(int i=0;i<n;i++) dist[i][i]=0;//城市自己到自己距离为 0
    adj.assign(n, vector<int>());//初始化邻接表

    int row=0;
    while(getline(fin,line)){//读取每一行的距离数据
        stringstream ss2(line);
        string cityName;
        getline(ss2,cityName,',');
        int u = cityMap[cityName];//找到当前行对应的城市编号 u
        for(int col=0;col<n;col++){
            string val;
            getline(ss2,val,',');
            dist[u][col] = stoi(val);
        }
        row++;
    }
}

// 加载邻接矩阵并构建邻接表
void Graph::loadAdjacency(const string &filename){
    ifstream fin(filename);
    string line;
    getline(fin,line); //跳过表头
    int row=0;
    //读取每一行的邻接数据
    while(getline(fin,line)){
        stringstream ss(line);
        string cityName;
        getline(ss,cityName,',');
        int u = cityMap[cityName];
        //读取 0/1 并建邻接表
        for(int col=0;col<n;col++){
            string val;
            getline(ss,val,',');
            if(stoi(val)==1) adj[u].push_back(col);//如果是 1 则表示有边，加入邻接表
        }//最终效果如：adj[上海] = {北京, 广州}
        row++;
    }
}

void Graph::dfs(int u,int target,vector<int>& path, vector<vector<int>>& allPaths,int maxDepth){
    if(path.size()>maxDepth) return;//深度限制（防止爆炸）
    if(u==target){//找到目标城市,把当前路径保存下来
        allPaths.push_back(path);
        return;
    }
    //遍历所有邻居
    for(int v: adj[u]){
        if(find(path.begin(),path.end(),v)!=path.end()) continue;//防止走回头路（防止死循环）
        //递归 DFS
        path.push_back(v);
        dfs(v,target,path,allPaths,maxDepth);
        path.pop_back();
    }
}
