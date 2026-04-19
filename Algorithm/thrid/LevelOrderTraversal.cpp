#include <iostream>
#include <queue>
#include <string>
#include <utility>
#include <vector>

using namespace std;

struct TreeNode {
    int val;
    TreeNode* left;
    TreeNode* right;

    explicit TreeNode(int value) : val(value), left(nullptr), right(nullptr) {}
};

vector<vector<int>> levelOrder(TreeNode* root) {
    vector<vector<int>> result;
    if (root == nullptr) {
        return result;
    }

    queue<TreeNode*> nodes;
    nodes.push(root);

    while (!nodes.empty()) {
        const size_t levelSize = nodes.size();
        vector<int> level;
        level.reserve(levelSize);

        for (size_t i = 0; i < levelSize; ++i) {
            TreeNode* current = nodes.front();
            nodes.pop();

            level.push_back(current->val);

            if (current->left != nullptr) {
                nodes.push(current->left);
            }
            if (current->right != nullptr) {
                nodes.push(current->right);
            }
        }

        result.push_back(std::move(level));
    }

    return result;
}

TreeNode* buildTreeFromLevelOrder(const vector<string>& values) {
    if (values.empty() || values[0] == "null" || values[0] == "#") {
        return nullptr;
    }

    vector<TreeNode*> nodes(values.size(), nullptr);
    for (size_t i = 0; i < values.size(); ++i) {
        if (values[i] != "null" && values[i] != "#") {
            nodes[i] = new TreeNode(stoi(values[i]));
        }
    }

    for (size_t i = 0; i < values.size(); ++i) {
        if (nodes[i] == nullptr) {
            continue;
        }

        const size_t leftIndex = 2 * i + 1;
        const size_t rightIndex = 2 * i + 2;

        if (leftIndex < values.size()) {
            nodes[i]->left = nodes[leftIndex];
        }
        if (rightIndex < values.size()) {
            nodes[i]->right = nodes[rightIndex];
        }
    }

    return nodes[0];
}

void destroyTree(TreeNode* root) {
    if (root == nullptr) {
        return;
    }

    queue<TreeNode*> nodes;
    nodes.push(root);

    while (!nodes.empty()) {
        TreeNode* current = nodes.front();
        nodes.pop();

        if (current->left != nullptr) {
            nodes.push(current->left);
        }
        if (current->right != nullptr) {
            nodes.push(current->right);
        }

        delete current;
    }
}

void printLevels(const vector<vector<int>>& levels) {
    cout << "[\n";
    for (size_t i = 0; i < levels.size(); ++i) {
        cout << "  [";
        for (size_t j = 0; j < levels[i].size(); ++j) {
            if (j > 0) {
                cout << ", ";
            }
            cout << levels[i][j];
        }
        cout << "]";
        if (i + 1 < levels.size()) {
            cout << ',';
        }
        cout << '\n';
    }
    cout << "]\n";
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    cin >> n;

    vector<string> values(n);
    for (int i = 0; i < n; ++i) {
        cin >> values[i];
    }

    TreeNode* root = buildTreeFromLevelOrder(values);
    const vector<vector<int>> levels = levelOrder(root);
    printLevels(levels);
    destroyTree(root);

    return 0;
}
