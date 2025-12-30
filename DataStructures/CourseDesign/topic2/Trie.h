#ifndef TRIE_H
#define TRIE_H

#include <string>
#include <vector>

//Trie 节点
struct TrieNode {
    bool isEnd = false;
    int count = 0;
    TrieNode* children[26]{};

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
    TrieNode* root;

    void dfs(TrieNode* node,
             std::string& current,
             std::vector<std::pair<std::string, int>>& result);
};

//后缀 Trie（简化后缀树）
class SuffixTrie {
public:
    SuffixTrie();

    void build(const std::string& s);
    bool contains(const std::string& s) const;

private:
    TrieNode* root;
};

//最长回文子串接口
std::string longestPalindrome(const std::string& s);

#endif // TRIE_H
