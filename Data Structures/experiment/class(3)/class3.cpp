#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <chrono>//高精度计时，用于对比查找耗时 

using namespace std;
using namespace std::chrono;

// ------------------- BST / AVL 节点 -------------------
struct TreeNode {
    string word; // 单词
    string meaning;// 释义
    TreeNode* left;// 左子树
    TreeNode* right;// 右子树
    int height; // AVL 树用到的节点高度(只在 AVL 平衡树时使用)
    TreeNode(const string& w, const string& m) : word(w), meaning(m), left(nullptr), right(nullptr), height(1) {}
};

// ------------------- 工具函数 -------------------
string trim(const string &s) {
    size_t start = s.find_first_not_of(" \t\r\n");//去掉字符串首尾的空格、制表符、回车或换行
    if (start == string::npos) return "";
    size_t end = s.find_last_not_of(" \t\r\n");
    return s.substr(start, end - start + 1);//用于处理 CSV 文件里的单词和释义
}

//去掉 CSV 引号
string removeQuotes(const string &s) {
    if (s.size() >= 2 && s.front() == '"' && s.back() == '"')
        return s.substr(1, s.size() - 2);
    return s;
}

// ------------------- 顺序查找 -------------------
string seqSearch(const vector<pair<string,string>>& dict, const string& target, vector<string>& path) {
    for (const auto& p : dict) {
        path.push_back(p.first);//// 记录访问路径
        if (p.first == target) return p.second;//// 找到返回释义
    }
    return "";//// 未找到返回空字符串
}

// ------------------- BST -------------------
TreeNode* insertBST(TreeNode* root, const string& word, const string& meaning) {
    if (!root) return new TreeNode(word, meaning);//插入新节点
    if (word < root->word) root->left = insertBST(root->left, word, meaning);//小于当前节点 -> 左子树
    else if (word > root->word) root->right = insertBST(root->right, word, meaning);//大于当前节点 -> 右子树
    return root;//相等则不插入
}

// BST 查找
string searchBST(TreeNode* root, const string& target, vector<string>& path) {
    TreeNode* cur = root;
    while (cur) {
        path.push_back(cur->word);//每访问一个节点加入路径path
        if (target == cur->word) return cur->meaning;// 找到返回释义
        if (target < cur->word) cur = cur->left;// 小于当前节点 -> 左子树
        else cur = cur->right;// 大于当前节点 -> 右子树
    }
    return "";
}

// ------------------- AVL -------------------
// AVL 辅助函数
int height(TreeNode* node) { return node ? node->height : 0; }//节点高度
int balanceFactor(TreeNode* node) { return node ? height(node->left) - height(node->right) : 0; }//平衡因子（左高减右高）
void updateHeight(TreeNode* node) { if(node) node->height = max(height(node->left), height(node->right)) + 1; }//更新当前节点高度

// 旋转操作
TreeNode* rotateRight(TreeNode* y) {//右旋
    if (!y || !y->left) return y;
    TreeNode* x = y->left;
    TreeNode* T2 = x->right;
    x->right = y;
    y->left = T2;
    updateHeight(y);
    updateHeight(x);
    return x;
}

TreeNode* rotateLeft(TreeNode* x) {//左旋
    if (!x || !x->right) return x;
    TreeNode* y = x->right;
    TreeNode* T2 = y->left;
    y->left = x;
    x->right = T2;
    updateHeight(x);
    updateHeight(y);
    return y;
}

// AVL 插入
TreeNode* insertAVL(TreeNode* node, const string& word, const string& meaning) {
    if (!node) return new TreeNode(word, meaning);
    if (word < node->word) node->left = insertAVL(node->left, word, meaning);//递归插入左子树
    else if (word > node->word) node->right = insertAVL(node->right, word, meaning);//递归插入右子树
    else return node;

    updateHeight(node);//更新节点高度
    int bf = balanceFactor(node);// 计算平衡因子

    // LL
    if (bf > 1 && word < node->left->word) return rotateRight(node);// 左左情况右旋
    // RR
    if (bf < -1 && word > node->right->word) return rotateLeft(node);// 右右情况左旋
    // LR
    if (bf > 1 && word > node->left->word) { node->left = rotateLeft(node->left); return rotateRight(node);}// 左右情况先左旋再右旋
    // RL
    if (bf < -1 && word < node->right->word) { node->right = rotateRight(node->right); return rotateLeft(node);}// 右左情况先右旋再左旋
    return node;// 返回平衡后的节点
}

// AVL 查找
string searchAVL(TreeNode* root, const string& target, vector<string>& path) {
    TreeNode* cur = root;
    while (cur) {
        path.push_back(cur->word);
        if (target == cur->word) return cur->meaning;
        if (target < cur->word) cur = cur->left;
        else cur = cur->right;
    }
    return "";
}

// ------------------- 前缀匹配 -------------------
//输入前缀，返回前 10 个匹配单词
vector<string> prefixMatch(const vector<pair<string,string>>& dict, const string& prefix, int limit=10) {
    vector<string> res;
    for (const auto& p : dict) {
        if (p.first.substr(0, prefix.size()) == prefix) {
            res.push_back(p.first);
            if (res.size() >= limit) break;
        }
    }
    return res;
}

// ------------------- 主程序 -------------------
int main() {
    vector<pair<string,string>> seqDict;
    TreeNode* bstRoot = nullptr;
    TreeNode* avlRoot = nullptr;

    ifstream fin("EnWords.csv");
    if (!fin.is_open()) { cerr << "无法打开文件 EnWords.csv\n"; return 1; }

    string line;
    int count = 0;
    while (getline(fin, line)) {
        if (line.empty()) continue;
        size_t pos = line.find("\",\"");
        if (pos == string::npos) continue;

        string word = trim(removeQuotes(line.substr(0, pos+1)));
        string meaning = trim(removeQuotes(line.substr(pos+2)));

        if (word.empty() || meaning.empty()) continue;

        seqDict.emplace_back(word, meaning);
        bstRoot = insertBST(bstRoot, word, meaning);
        avlRoot = insertAVL(avlRoot, word, meaning);
        count++;
    }
    fin.close();
    cout << "词典加载完毕，共 " << count << " 个单词。\n";

    // ---------------- 查询循环 ----------------
    string input;
    while (true) {
        cout << "\n输入单词或前缀（空格退出）：";
        getline(cin, input);
        input = trim(input);
        if (input.empty() || input==" ") { cout << "退出查询。\n"; break; }

        // 前缀匹配
        vector<string> matches = prefixMatch(seqDict, input, 10);
        if (!matches.empty()) {
            cout << "匹配单词: ";
            for (auto& w : matches) cout << w << " ";
            cout << "\n";
        }

        // 顺序查找
        vector<string> pathSeq;
        auto start = high_resolution_clock::now();
        string meaningSeq = seqSearch(seqDict, input, pathSeq);
        auto end = high_resolution_clock::now();
        auto durationSeq = duration_cast<microseconds>(end - start).count();

        cout << "\n顺序查找路径: ";
        for (auto& w : pathSeq) cout << w << " ";
        cout << "\n顺序查找耗时: " << durationSeq << " 微秒\n";
        if (!meaningSeq.empty()) cout << "含义: " << meaningSeq << "\n";

        // BST 查找
        vector<string> pathBST;
        start = high_resolution_clock::now();// 开始计时
        string meaningBST = searchBST(bstRoot, input, pathBST);
        end = high_resolution_clock::now();// 结束计时
        auto durationBST = duration_cast<microseconds>(end - start).count();//使用 duration_cast<microseconds> 转换为微秒
        cout << "\nBST查找路径: ";
        for (auto& w : pathBST) cout << w << " ";
        cout << "\nBST查找耗时: " << durationBST << " 微秒\n";
        if (!meaningBST.empty()) cout << "含义: " << meaningBST << "\n";

        // AVL 查找
        vector<string> pathAVL;
        start = high_resolution_clock::now();
        string meaningAVL = searchAVL(avlRoot, input, pathAVL);
        end = high_resolution_clock::now();
        auto durationAVL = duration_cast<microseconds>(end - start).count();
        cout << "\nAVL查找路径: ";
        for (auto& w : pathAVL) cout << w << " ";
        cout << "\nAVL查找耗时: " << durationAVL << " 微秒\n";
        if (!meaningAVL.empty()) cout << "含义: " << meaningAVL << "\n";
    }

    return 0;
}
