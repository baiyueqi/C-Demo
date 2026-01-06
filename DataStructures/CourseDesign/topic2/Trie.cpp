#include "Trie.h"
#include <algorithm>
#include <cctype>

//TrieNode(把 26 个孩子指针全部初始化为 nullptr)
TrieNode::TrieNode() {
    for (auto& c : children)
        c = nullptr;
}

//Trie

Trie::Trie() : root(new TrieNode()) {}//创建 Trie 的 根节点(不代表任何字符,只是所有单词的起点)

void Trie::insert(const std::string& word) {
    TrieNode* node = root;//node 指向当前遍历到的节点
    //遍历单词的每个字符
    for (char ch : word) {
        //过滤非法字符
        if (!std::isalpha(static_cast<unsigned char>(ch)))
            continue;
        ch = std::tolower(ch);//转换为小写
        int idx = ch - 'a';//计算字符在 children 数组中的索引(下标)
        //Trie 的核心思想:需要才创建节点，最大程度复用公共前缀
        if (!node->children[idx])//如果对应的子节点不存在,就创建一个新的 TrieNode
            node->children[idx] = new TrieNode();
        node = node->children[idx];//移动到子节点(向下走一层)
    }
    //结束标志
    node->isEnd = true;
    node->count++;
}

//获取所有单词
std::vector<std::pair<std::string, int>> Trie::getAllWords() {
    std::vector<std::pair<std::string, int>> result;
    std::string current;//当前正在拼的单词
    dfs(root, current, result);//深度优先遍历整个 Trie
    return result;
}

//深度优先遍历 Trie
void Trie::dfs(TrieNode* node,
               std::string& current,
               std::vector<std::pair<std::string, int>>& result) {
    if (!node) return;//空指针保护

    //如果当前节点是某个单词的结尾,就把当前拼的单词和它的出现次数加入结果
    if (node->isEnd) {
        result.emplace_back(current, node->count);
    }

    //遍历 26 个孩子
    for (int i = 0; i < 26; ++i) {
        if (node->children[i]) {
            current.push_back('a' + i);
            dfs(node->children[i], current, result);
            current.pop_back();//回溯,搜索完恢复现场
        }
    }
}

//SuffixTrie
SuffixTrie::SuffixTrie() : root(new TrieNode()) {}

//build —— 构建所有后缀
void SuffixTrie::build(const std::string& s) {
    int n = static_cast<int>(s.size());
    //外层循环：枚举起点
    for (int i = 0; i < n; ++i) {
        TrieNode* node = root;
        //内层循环：构建后缀
        for (int j = i; j < n; ++j) {
            char ch = s[j];
            if (!std::isalpha(static_cast<unsigned char>(ch)))
                break;  // 非字母直接断开这个后缀

            ch = std::tolower(ch);
            int idx = ch - 'a';
            
            //插入到 Trie 中
            if (!node->children[idx])
                node->children[idx] = new TrieNode();
            node = node->children[idx];
        }
    }
}

//contains —— 检查后缀是否存在
bool SuffixTrie::contains(const std::string& s) const {
    TrieNode* node = root;
    //从 root 出发,按字符一路走,只要某一步走不通 → 不是子串,全部走通 → 是子串
    for (char ch : s) {
        if (!std::isalpha(static_cast<unsigned char>(ch)))
            return false;

        ch = std::tolower(ch);
        int idx = ch - 'a';

        if (!node->children[idx])
            return false;
        node = node->children[idx];
    }
    return true;
}


//最长回文子串
std::string longestPalindrome(const std::string& s) {
    if (s.empty()) return "";

    SuffixTrie suffixTrie;
    suffixTrie.build(s);

    std::string best;
    int n = static_cast<int>(s.size());

    auto expand = [&](int l, int r) {//中心扩展法,l、r 是回文中心
        while (l >= 0 && r < n && s[l] == s[r]) {//向两边扩展
            std::string sub = s.substr(l, r - l + 1);
            if (suffixTrie.contains(sub) &&
                sub.size() > best.size()) {
                best = sub;
            }
            --l;
            ++r;
        }
    };

    //枚举所有中心
    for (int i = 0; i < n; ++i) {
        expand(i, i);       // 奇数回文
        expand(i, i + 1);   // 偶数回文
    }

    return best;
}
