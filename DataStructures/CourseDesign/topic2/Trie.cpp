#include "Trie.h"
#include <algorithm>
#include <cctype>

//TrieNode
TrieNode::TrieNode() {
    for (auto& c : children)
        c = nullptr;
}

//Trie

Trie::Trie() : root(new TrieNode()) {}

void Trie::insert(const std::string& word) {
    TrieNode* node = root;
    for (char ch : word) {
        if (!std::isalpha(static_cast<unsigned char>(ch)))
            continue;
        ch = std::tolower(ch);
        int idx = ch - 'a';
        if (!node->children[idx])
            node->children[idx] = new TrieNode();
        node = node->children[idx];
    }
    node->isEnd = true;
    node->count++;
}

std::vector<std::pair<std::string, int>> Trie::getAllWords() {
    std::vector<std::pair<std::string, int>> result;
    std::string current;
    dfs(root, current, result);
    return result;
}

void Trie::dfs(TrieNode* node,
               std::string& current,
               std::vector<std::pair<std::string, int>>& result) {
    if (!node) return;

    if (node->isEnd) {
        result.emplace_back(current, node->count);
    }

    for (int i = 0; i < 26; ++i) {
        if (node->children[i]) {
            current.push_back('a' + i);
            dfs(node->children[i], current, result);
            current.pop_back();
        }
    }
}

//SuffixTrie
SuffixTrie::SuffixTrie() : root(new TrieNode()) {}

void SuffixTrie::build(const std::string& s) {
    int n = static_cast<int>(s.size());
    for (int i = 0; i < n; ++i) {
        TrieNode* node = root;
        for (int j = i; j < n; ++j) {
            int idx = s[j] - 'a';
            if (!node->children[idx])
                node->children[idx] = new TrieNode();
            node = node->children[idx];
        }
    }
}

bool SuffixTrie::contains(const std::string& s) const {
    TrieNode* node = root;
    for (char ch : s) {
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

    auto expand = [&](int l, int r) {
        while (l >= 0 && r < n && s[l] == s[r]) {
            std::string sub = s.substr(l, r - l + 1);
            if (suffixTrie.contains(sub) &&
                sub.size() > best.size()) {
                best = sub;
            }
            --l;
            ++r;
        }
    };

    for (int i = 0; i < n; ++i) {
        expand(i, i);       // 奇数回文
        expand(i, i + 1);   // 偶数回文
    }

    return best;
}
