#include <iostream>
#include <vector>
#include <string>
#include <iomanip>
#include <limits>
#include <thread>
#include <chrono>

using namespace std;

const int INF = 1e9;//表示无穷大，用于初始化边权，表示不存在的边
const int n = 10; // 节点数 0~9

struct Step {
    string title;//这一步的描述（如 DFS 访问哪个节点）
    vector<pair<int,int>> probeEdges;//当前考虑的边（试探边），即算法查看的候选边
    vector<int> visited;//标记哪些节点已经访问（DFS）或加入（Prim/Dijkstra）
    vector<int> parent;//每个节点的父节点，用于 MST 或最短路径回溯
    vector<int> dist;//距离或权值信息，用于 Dijkstra 或 Prim
};

class Graph {
public:
    int w[n][n];//w[n][n]：邻接矩阵，存储边权
    //构造函数将所有边初始化为 INF，表示初始时节点之间没有边
    Graph() {
        for(int i=0;i<n;i++)
            for(int j=0;j<n;j++)
                w[i][j] = INF;
    }

    //添加无向边，双向赋值
    void addEdge(int u,int v,int weight){
        w[u][v] = weight;
        w[v][u] = weight;
    }

    // DFS
    void dfsUtil(int u, vector<int> &vis, vector<Step> &steps){//DFS 的递归函数
        vis[u] = 1;//标记节点已访问
        Step s;//记录当前节点及所有试探边
        s.title = "DFS 访问节点 " + to_string(u);//记录当前访问状态
        s.visited = vis;
        //检查所有邻居
        for(int v=0;v<n;v++)
            if(w[u][v]<INF)//存在边 ,表示 u 和 v 有边连接
                s.probeEdges.push_back(make_pair(u,v));//记录试探边
        steps.push_back(s);//每访问一个节点就记录一次，方便动态显示

        for(int v=0;v<n;v++)//递归访问未访问的邻居
            if(w[u][v]<INF && !vis[v])//存在边且未访问
                dfsUtil(v, vis, steps);
    }

    // 运行 DFS 并返回步骤
    vector<Step> runDFS(int start){
        vector<int> vis(n,0);
        vector<Step> steps;
        dfsUtil(start, vis, steps);
        return steps;
    }

    // Prim
    vector<Step> runPrim(int start){
        vector<int> inMST(n,0), low(n,INF), parent(n,-1);//inMST[i]：节点是否已加入 MST,low[i]：节点 i 到 MST 的最小边权,parent[i]：节点 i 的父节点
        low[start]=0;//起点权值为 0，保证从起点开始
        vector<Step> steps;

        for(int k=0;k<n;k++){
            int u=-1;
            //每轮选择 MST 外边权最小的节点 u,u 是当前最小边的终点
            for(int i=0;i<n;i++)
                if(!inMST[i] && (u==-1 || low[i]<low[u]))
                    u=i;
            if(u==-1) break;//标记节点 u 已加入 MST
            inMST[u]=1;//标记哪些节点已加入最小生成树,如果没有节点可选（图不连通），则退出

            Step s;
            s.title = "Prim 加入节点 " + to_string(u);
            s.visited = inMST;//MST 内节点
            s.dist = low;//各节点到 MST 的最小边权
            s.parent = parent;//MST 的父节点关系
            //收集当前节点 u 可连接的试探边，便于显示
            for(int v=0;v<n;v++)
                if(!inMST[v] && w[u][v]<INF)
                    s.probeEdges.push_back(make_pair(u,v));
            steps.push_back(s);
            
            //更新 MST 外节点的最小边权和父节点
            for(int v=0;v<n;v++)
                if(!inMST[v] && w[u][v]<low[v]){
                    low[v] = w[u][v];//保证了每个节点都选择最小边
                    parent[v] = u;//记录连接的父节点
                }
        }
        return steps;
    }

    // Dijkstra
    vector<Step> runDijkstra(int start){
        vector<int> dist(n,INF), vis(n,0), parent(n,-1);//起点到节点 i 的最短距离;节点是否已访问;最短路径的父节点
        dist[start]=0;
        vector<Step> steps;

        //选择未访问节点中距离最小的节点 u
        for(int k=0;k<n;k++){
            int u=-1;
            for(int i=0;i<n;i++)
                if(!vis[i] && (u==-1 || dist[i]<dist[u]))
                    u=i;
            if(u==-1) break;//没有节点可选，退出
            vis[u]=1;//标记节点 u 已访问

            Step s;
            s.title = "Dijkstra 确定节点 " + to_string(u);
            s.visited = vis;//已确定最短路径的节点
            s.dist = dist;//当前所有节点距离
            s.parent = parent;//路径回溯信息
            for(int v=0;v<n;v++)
                if(!vis[v] && w[u][v]<INF)
                    s.probeEdges.push_back(make_pair(u,v));
            steps.push_back(s);

            //检查邻居是否可以通过 u 更新最短路径
            for(int v=0;v<n;v++)
                if(!vis[v] && w[u][v]<INF && dist[v]>dist[u]+w[u][v]){
                    dist[v]=dist[u]+w[u][v];//更新最短距离
                    parent[v]=u;//更新父节点
                }
        }
        return steps;
    }
};

// 打印辅助函数（中文）
void printStep(const string &name, const Step &s){
    cout << "================ " << name << " =================\n";
    cout << s.title << "\n";

    if(!s.visited.empty()){
        cout << "已访问 visited: ";
        for(int v : s.visited)
            cout << v << " ";
        cout << "\n";
    }

    if(!s.parent.empty()){
        cout << "父节点 parent: ";
        for(int v : s.parent)
            cout << v << " ";
        cout << "\n";
    }

    if(!s.dist.empty()){
        cout << "距离 dist: ";
        for(int v : s.dist)
            cout << (v>=INF?"INF":to_string(v)) << " ";
        cout << "\n";
    }

    if(!s.probeEdges.empty()){
        cout << "探测边 probeEdges: ";
        for(auto &p : s.probeEdges)
            cout << "(" << p.first << "," << p.second << ") ";
        cout << "\n";
    }
    cout << "============================================\n";
}

// 命令行动画演示
void animateSteps(const vector<Step> &steps, const string &name, int delay_ms){
    for(size_t i=0;i<steps.size();i++){
        //(void)system("clear"); //Linux 清屏，忽略返回值
        printStep(name, steps[i]);
        this_thread::sleep_for(chrono::milliseconds(delay_ms));
    }
}

int main(){
    Graph g;

    // 添加图中边（例子，可根据你的10节点图修改）
    g.addEdge(0,5,840);
    g.addEdge(0,1,340);
    g.addEdge(5,8,1140);
    g.addEdge(5,6,570);
    g.addEdge(1,3,900);
    g.addEdge(3,6,530);
    g.addEdge(6,8,650);
    g.addEdge(6,7,820);
    g.addEdge(6,4,1200);
    g.addEdge(7,4,960);
    g.addEdge(3,4,600);
    g.addEdge(8,9,750);
    g.addEdge(7,9,680);
    g.addEdge(3,2,1380);
    g.addEdge(1,2,2500);
    g.addEdge(2,4,2600);

    int start;
    cout << "请输入起始节点(0~9): ";
    cin >> start;

    vector<Step> dfsSteps = g.runDFS(start);
    vector<Step> primSteps = g.runPrim(start);
    vector<Step> dijSteps = g.runDijkstra(start);

    // 动态演示三算法
    size_t maxSteps = max(dfsSteps.size(), max(primSteps.size(), dijSteps.size()));
    int delay_ms = 800;

    for(size_t i=0;i<maxSteps;i++){
        //(void)system("clear"); // Unix/Linux 清屏
        if(i<dfsSteps.size()) printStep("深度优先遍历 DFS", dfsSteps[i]);
        if(i<primSteps.size()) printStep("Prim 最小生成树", primSteps[i]);
        if(i<dijSteps.size()) printStep("Dijkstra 最短路径", dijSteps[i]);
        this_thread::sleep_for(chrono::milliseconds(delay_ms));
    }

    cout << "演示结束。\n";

    return 0;
}
