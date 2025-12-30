#include "Trie.h"
#include <iostream>
#include <fstream>
#include <algorithm>

int main() {
    Trie trie;

    std::ifstream fin("words.txt");
    if (!fin) {
        std::cerr << "Error: cannot open words.txt\n";
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

    std::cout << "Top 10 frequent words:\n";
    for (size_t i = 0; i < std::min<size_t>(10, allWords.size()); ++i) {
        std::cout << allWords[i].first
                  << " : " << allWords[i].second << '\n';
    }

    std::cout << "\n--------------------------------\n";

    std::string test = "babadlevelracecar";
    std::cout << "Original string: " << test << '\n';
    std::cout << "Longest palindrome: "
              << longestPalindrome(test) << '\n';

    return 0;
}
