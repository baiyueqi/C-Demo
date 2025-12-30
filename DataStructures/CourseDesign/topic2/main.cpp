#include "Trie.h"
#include <iostream>
#include <fstream>
#include <algorithm>

//数据结构选择（显式）
enum class WordStatStructure {
    Trie
};

void printWordStatChoice(WordStatStructure type) {
    std::cout << "【数据结构选择】\n";
    switch (type) {
        case WordStatStructure::Trie:
            std::cout << "词频统计所使用的数据结构：Trie 树（字典树）\n";
            break;
    }
    std::cout << std::endl;
}

void printWordStatComplexity() {
    std::cout << "【时间复杂度分析 —— 词频统计】\n";
    std::cout << "1. 将所有单词插入 Trie 树：O(M)\n";
    std::cout << "2. 遍历 Trie 树收集所有单词：O(M)\n";
    std::cout << "3. 对不同单词按频率排序：O(K log K)\n";
    std::cout << "其中：M 表示所有单词字符总数，K 表示不同单词的数量\n\n";
}

void printPalindromeComplexity() {
    std::cout << "【时间复杂度分析 —— 最长回文子串】\n";
    std::cout << "1. 构建后缀 Trie（后缀树）：O(n^2)\n";
    std::cout << "2. 采用中心扩展法查找回文子串：O(n^2)\n";
    std::cout << "总体时间复杂度：O(n^2)\n\n";
}

int main() {
    //1. 选择数据结构
    WordStatStructure structure = WordStatStructure::Trie;
    printWordStatChoice(structure);

    //2. 词频统计
    Trie trie;

    std::ifstream fin("words.txt");
    if (!fin) {
        std::cerr << "错误：无法打开 words.txt 文件\n";
        return 1;
    }

    std::string word;
    while (std::getline(fin, word)) {
        if (!word.empty())
            trie.insert(word);
    }
    fin.close();

    auto allWords = trie.getAllWords();

    std::sort(allWords.begin(), allWords.end(),
              [](const auto& a, const auto& b) {
                  return a.second > b.second;
              });

    std::cout << "【出现频率最高的前 10 个单词】\n";
    for (size_t i = 0; i < std::min<size_t>(10, allWords.size()); ++i) {
        std::cout << i + 1 << ". "
                  << allWords[i].first
                  << " —— 出现次数："
                  << allWords[i].second << '\n';
    }
    std::cout << std::endl;

    //3. 输出词频统计复杂度 
    printWordStatComplexity();

    //4. 最长回文子串
    std::string test = "babadlevelracecar";
    std::cout << "【最长回文子串测试】\n";
    std::cout << "原始字符串：" << test << '\n';
    std::cout << "最长回文子串："
              << longestPalindrome(test) << "\n\n";

    //5. 输出回文算法复杂度
    printPalindromeComplexity();

    return 0;
}
