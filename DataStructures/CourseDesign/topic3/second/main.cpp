#include "Graph.h"
#include "TrafficConsultSystem.h"
#include "Utils.h"
#include <iostream>
using namespace std;

void showMenu(){
    cout << "\n=== 交通咨询系统菜单 ===\n";
    cout << "1. 查询两城市最短路径（可选择算法）\n";
    cout << "2. 查询两城市所有路径（最多10个节点）并排序输出\n";
    cout << "3. 查询两城市最短路径，绕过指定城市\n";
    cout << "4. 查询两城市第K短路径\n";
    cout << "5. 验证：各省会到武汉中转不超过 2 个\n";
    cout << "0. 退出\n";
    cout << "请输入选项: ";
}

int main(){
    TrafficConsultSystem tcs;
    tcs.g.loadDistances("city_distances.csv");
    tcs.g.loadAdjacency("city_adjacency.csv");

    cout<<"城市总数: "<<tcs.g.n<<endl;
    for(int i=0;i<tcs.g.n;i++) cout<<tcs.g.cityNames[i]<<" "; 
    cout<<endl;

    int choice;
    while(true){
        showMenu();
        cin>>choice;
        if(choice==0) break;

        string startCity,endCity;
        cout<<"请输入起点城市: "; cin>>startCity;
        cout<<"请输入终点城市: "; cin>>endCity;
        int start = tcs.g.cityMap[startCity];
        int end = tcs.g.cityMap[endCity];

        if(choice==1){
            cout<<"选择算法: 1-Dijkstra 2-Floyd: ";
            int algo; cin>>algo;
            if(algo==1){
                vector<int> prev;
                auto dis = tcs.dijkstra(start,prev);
                cout<<"Dijkstra 最短距离: "<<dis[end]<<endl;
            }else{
                vector<vector<int>> next;
                auto d = tcs.floyd(next);
                cout<<"Floyd 最短距离: "<<d[start][end]<<endl;
            }
        }else if(choice==2){
            auto paths = tcs.getAllPathsLengths(start,end);
            cout<<"找到 "<<paths.size()<<" 条可行路径，按总长度排序如下:"<<endl;
            for(auto &p: paths){
                printPath(tcs.g.cityNames,p.first);
                cout<<"总长度: "<<p.second<<endl;
            }
            savePathsToFile("paths.txt",tcs.g.cityNames,paths);
            cout<<"已保存到 paths.txt\n";
        }else if(choice==3){
            string skipCity;
            cout<<"请输入需要绕过的城市: "; cin>>skipCity;
            int skip = tcs.g.cityMap[skipCity];
            vector<int> prev;
            auto dis = tcs.dijkstra(start,prev,skip);
            cout<<"绕过 "<<skipCity<<" 的 Dijkstra 最短距离: "<<dis[end]<<endl;
        }else if(choice==4){
            int k;
            cout<<"请输入 K (第几短路径): "; cin>>k;
            auto paths = tcs.getAllPathsLengths(start,end);
            if(k>0 && k<=paths.size()){
                cout<<"第 "<<k<<" 短路径: ";
                printPath(tcs.g.cityNames,paths[k-1].first);
                cout<<"总长度: "<<paths[k-1].second<<endl;
            }else{
                cout<<"路径数量不足 "<<k<<" 条\n";
            }
        }else if(choice == 5){
            tcs.verifyWuhanCenter();
        
        }else{
            cout<<"无效选项，请重新输入\n";
        }
    }

    cout<<"感谢使用交通咨询系统！\n";
    return 0;
}
