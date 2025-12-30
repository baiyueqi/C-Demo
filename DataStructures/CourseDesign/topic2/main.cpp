#include "Trie.h"
#include <iostream>
#include <fstream>
#include <algorithm>

//功能菜单
void showMenu() {
    std::cout << "==============================\n";
    std::cout << "请选择要使用的数据结构及功能：\n";
    std::cout << "1. Trie 树 —— 单词词频统计（Top 10）\n";
    std::cout << "2. 后缀树 —— 查找字符串的最长回文子串\n";
    std::cout << "0. 退出程序\n";
    std::cout << "==============================\n";
    std::cout << "请输入选项（0 / 1 / 2）：";
}

//Trie 树词频统计功能
void runTrieWordStatistics() {
    std::cout << "\n【已选择 Trie 树】\n";
    std::cout << "功能：统计文本中出现频率最高的前 10 个单词\n\n";

    Trie trie;

    std::ifstream fin("words.txt");
    if (!fin) {
        std::cerr << "错误：无法打开 words.txt 文件\n";
        return;
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

    std::cout << "\n【时间复杂度分析】\n";
    std::cout << "插入所有单词：O(M)\n";
    std::cout << "遍历 Trie 树：O(M)\n";
    std::cout << "排序不同单词：O(K log K)\n";
    std::cout << "M 为字符总数，K 为不同单词数量\n\n";
}

//后缀树最长回文功能
void runSuffixTriePalindrome()
{
    std::cout << "\n【已选择 后缀 Trie（简化后缀树）】\n";
    std::cout << "功能：查找文件中“单个单词”的最长回文子串\n\n";

    std::ifstream fin("words.txt");
    if (!fin.is_open())
    {
        std::cout << "无法打开文件 words.txt\n";
        return;
    }

    std::string word;
    std::string bestWord;        // 含有最长回文的单词
    std::string bestPalindrome;  // 最长回文子串

    while (std::getline(fin, word))
    {
        if (word.empty()) continue;

        std::string cur = longestPalindrome(word);

        if (cur.length() > bestPalindrome.length())
        {
            bestPalindrome = cur;
            bestWord = word;
        }
    }
    fin.close();

    if (bestPalindrome.empty())
    {
        std::cout << "未找到回文子串\n";
        return;
    }

    std::cout << "【结果】\n";
    std::cout << "包含最长回文的单词：" << bestWord << '\n';
    std::cout << "最长回文子串：" << bestPalindrome << '\n';
    std::cout << "回文长度：" << bestPalindrome.length() << '\n';

    std::cout << "\n【时间复杂度分析】\n";
    std::cout << "设单词数量为 N，单词最大长度为 L\n";
    std::cout << "单个单词回文查找：O(L^2)\n";
    std::cout << "总体时间复杂度：O(N * L^2)\n\n";
}



//主函数
int main() {
    while (true) {
        showMenu();

        int choice;
        if (!(std::cin >> choice)) {
            std::cin.clear();
            std::cin.ignore(1024, '\n');
            std::cout << "输入错误，请输入数字选项。\n\n";
            continue;
        }

        if (choice == 0) {
            std::cout << "程序已退出。\n";
            break;
        }

        switch (choice) {
            case 1:
                runTrieWordStatistics();
                break;
            case 2:
                runSuffixTriePalindrome();
                break;
            default:
                std::cout << "无效选项，请重新选择。\n\n";
                break;
        }
    }

    return 0;
}
