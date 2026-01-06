#pragma once
#include <bits/stdc++.h>
using namespace std;

//打印一条路径
void printPath(const vector<string>& cityNames,const vector<int>& path){
    for(int i=0;i<path.size();i++){
        cout << cityNames[path[i]];
        if(i!=path.size()-1) cout<<"->";
    }
    cout<<endl;
}

//将多条路径保存到文件
void savePathsToFile(const string &filename, const vector<string>& cityNames, const vector<pair<vector<int>,int>>& paths){
    ofstream fout(filename);
    for(auto &p: paths){
        for(int i=0;i<p.first.size();i++){
            fout<<cityNames[p.first[i]];
            if(i!=p.first.size()-1) fout<<"->";
        }
        fout<<", TotalDistance="<<p.second<<endl;
    }
    fout.close();
}
