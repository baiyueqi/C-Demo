#include "Utils.h"
#include <algorithm>
#include <cctype>

using namespace std;

string Utils::trim(const string& str) {
    size_t start = str.find_first_not_of(" \t\n\r");
    if (start == string::npos) return "";
    
    size_t end = str.find_last_not_of(" \t\n\r");
    return str.substr(start, end - start + 1);
}

vector<string> Utils::split(const string& str, char delim) {
    vector<string> tokens;
    size_t start = 0;
    size_t end = str.find(delim);
    
    while (end != string::npos) {
        tokens.push_back(trim(str.substr(start, end - start)));
        start = end + 1;
        end = str.find(delim, start);
    }
    tokens.push_back(trim(str.substr(start)));
    
    return tokens;
}