#ifndef TRIE_H
#define TRIE_H

#include <string>
#include <vector>

//Trie 节点(Trie 的“最小单位”)
struct TrieNode {
    bool isEnd = false;//true:某个单词在这里结束;false:只是前缀，不是完整单词
    int count = 0;//记录单词出现的次数
    TrieNode* children[26]{};//每个节点最多有 26 个子节点,对应 26 个字母

    TrieNode();
};

//Trie 树
class Trie {
public:
    Trie();

    // 插入单词
    void insert(const std::string& word);

    // 获取所有单词及词频
    std::vector<std::pair<std::string, int>> getAllWords();

private:
    TrieNode* root;//根节点(不存字符，只作为入口)

    // 深度优先搜索辅助函数(一路往下走,遇到 isEnd == true,把 current 加入结果,回溯继续遍历)
    void dfs(TrieNode* node,
             std::string& current,
             std::vector<std::pair<std::string, int>>& result);
};

//后缀 Trie（简化后缀树）
class SuffixTrie {
public:
    SuffixTrie();

    void build(const std::string& s);//构建后缀 Trie
    bool contains(const std::string& s) const;//字符串s是否是原字符串的 子串

private:
    TrieNode* root;
};

//最长回文子串接口
std::string longestPalindrome(const std::string& s);

#endif // TRIE_H
