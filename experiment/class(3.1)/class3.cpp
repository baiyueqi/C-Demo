#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <termios.h>
#include <unistd.h>
#include <chrono>
#include <algorithm>

using namespace std;
using namespace std::chrono;

// ------------------- BST / AVL 节点 -------------------
struct TreeNode {
    string word;
    string meaning;
    TreeNode* left;
    TreeNode* right;
    int height;
    TreeNode(const string& w, const string& m) : word(w), meaning(m), left(nullptr), right(nullptr), height(1) {}
};

// ------------------- 工具函数 -------------------
string trim(const string &s) {
    size_t start = s.find_first_not_of(" \t\r\n");
    if (start == string::npos) return "";
    size_t end = s.find_last_not_of(" \t\r\n");
    return s.substr(start, end - start + 1);
}

string removeQuotes(const string &s) {
    if (s.size() >= 2 && s.front() == '"' && s.back() == '"')
        return s.substr(1, s.size() - 2);
    return s;
}

// ------------------- 顺序查找 -------------------
string seqSearch(const vector<pair<string,string>>& dict, const string& target, vector<string>& path) {
    for (const auto& p : dict) {
        path.push_back(p.first);
        if (p.first == target) return p.second;
    }
    return "";
}

// ------------------- BST -------------------
TreeNode* insertBST(TreeNode* root, const string& word, const string& meaning) {
    if (!root) return new TreeNode(word, meaning);
    if (word < root->word) root->left = insertBST(root->left, word, meaning);
    else if (word > root->word) root->right = insertBST(root->right, word, meaning);
    return root;
}

string searchBST(TreeNode* root, const string& target, vector<string>& path) {
    TreeNode* cur = root;
    while (cur) {
        path.push_back(cur->word);
        if (target == cur->word) return cur->meaning;
        if (target < cur->word) cur = cur->left;
        else cur = cur->right;
    }
    return "";
}

// ------------------- AVL -------------------
int height(TreeNode* node) { return node ? node->height : 0; }
int balanceFactor(TreeNode* node) { return node ? height(node->left) - height(node->right) : 0; }
void updateHeight(TreeNode* node) { if(node) node->height = max(height(node->left), height(node->right)) + 1; }

TreeNode* rotateRight(TreeNode* y) {
    if (!y || !y->left) return y;
    TreeNode* x = y->left;
    TreeNode* T2 = x->right;
    x->right = y;
    y->left = T2;
    updateHeight(y);
    updateHeight(x);
    return x;
}

TreeNode* rotateLeft(TreeNode* x) {
    if (!x || !x->right) return x;
    TreeNode* y = x->right;
    TreeNode* T2 = y->left;
    y->left = x;
    x->right = T2;
    updateHeight(x);
    updateHeight(y);
    return y;
}

TreeNode* insertAVL(TreeNode* node, const string& word, const string& meaning) {
    if (!node) return new TreeNode(word, meaning);
    if (word < node->word) node->left = insertAVL(node->left, word, meaning);
    else if (word > node->word) node->right = insertAVL(node->right, word, meaning);
    else return node;

    updateHeight(node);
    int bf = balanceFactor(node);

    if (bf > 1 && word < node->left->word) return rotateRight(node);        // LL
    if (bf < -1 && word > node->right->word) return rotateLeft(node);       // RR
    if (bf > 1 && word > node->left->word) { node->left = rotateLeft(node->left); return rotateRight(node);} // LR
    if (bf < -1 && word < node->right->word) { node->right = rotateRight(node->right); return rotateLeft(node);} // RL
    return node;
}

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

// ------------------- 终端设置 -------------------
struct TermiosGuard {
    termios oldt;
    TermiosGuard() { 
        tcgetattr(STDIN_FILENO, &oldt);
        termios newt = oldt;
        newt.c_lflag &= ~(ICANON | ECHO); // 关闭缓冲和回显
        tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    }
    ~TermiosGuard() {
        tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    }
};

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

    cout << "逐字输入查询，按空格退出。\n";
    TermiosGuard guard;  // 终端非缓冲

    string input;
    while (true) {
        input.clear();
        cout << "\n输入前缀：";
        cout.flush();

        char ch;
        while (true) {
            if (read(STDIN_FILENO, &ch, 1) != 1) continue;

            if (ch == ' ') {  // 空格退出
                cout << "\n退出查询。\n";
                return 0;
            }
            if (ch == 10) break; // 回车结束当前查询前缀
            if (ch == 127 || ch == 8) { // 退格
                if (!input.empty()) input.pop_back();
            } else {
                input.push_back(ch);
            }

            // 清屏
            cout << "\033[2J\033[H"; // 清屏并光标置顶
            cout << "逐字输入查询，按空格退出。\n";
            cout << "输入前缀: " << input << "\n";

            // 前缀匹配显示
            if (!input.empty()) {
                vector<string> matches = prefixMatch(seqDict, input, 10);
                cout << "匹配单词: ";
                for (auto& w : matches) cout << w << " ";
                cout << "\n";
            }

            // 顺序查找
            vector<string> pathSeq;
            auto start = high_resolution_clock::now();
            string meaningSeq = seqSearch(seqDict, input, pathSeq);
            auto end = high_resolution_clock::now();
            auto durationSeq = chrono::duration_cast<microseconds>(end - start).count();

            cout << "\n顺序查找路径: ";
            for (auto& w : pathSeq) cout << w << " ";
            cout << "\n顺序查找耗时: " << durationSeq << " 微秒\n";
            if (!meaningSeq.empty()) cout << "含义: " << meaningSeq << "\n";

            // BST 查找
            vector<string> pathBST;
            start = high_resolution_clock::now();
            string meaningBST = searchBST(bstRoot, input, pathBST);
            end = high_resolution_clock::now();
            auto durationBST = chrono::duration_cast<microseconds>(end - start).count();
            cout << "\nBST查找路径: ";
            for (auto& w : pathBST) cout << w << " ";
            cout << "\nBST查找耗时: " << durationBST << " 微秒\n";
            if (!meaningBST.empty()) cout << "含义: " << meaningBST << "\n";

            // AVL 查找
            vector<string> pathAVL;
            start = high_resolution_clock::now();
            string meaningAVL = searchAVL(avlRoot, input, pathAVL);
            end = high_resolution_clock::now();
            auto durationAVL = chrono::duration_cast<microseconds>(end - start).count();
            cout << "\nAVL查找路径: ";
            for (auto& w : pathAVL) cout << w << " ";
            cout << "\nAVL查找耗时: " << durationAVL << " 微秒\n";
            if (!meaningAVL.empty()) cout << "含义: " << meaningAVL << "\n";

            cout.flush();
        }
    }

    return 0;
}
