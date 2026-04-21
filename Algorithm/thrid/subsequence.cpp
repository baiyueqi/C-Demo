#include <iostream>
#include <vector>
#include <string>
#include <algorithm>

int main() {
    std::string a, b;
    std::cin >> a >> b;

    int n = a.size(), m = b.size();

    std::vector<int> dp(m + 1, 0);

    for (int i = 1; i <= n; ++i) {
        int prev = 0;
        for (int j = 1; j <= m; ++j) {
            int temp = dp[j];
            if (a[i - 1] == b[j - 1]) {
                dp[j] = prev + 1;
            } else {
                dp[j] = std::max(dp[j], dp[j - 1]);
            }
            prev = temp;
        }
    }

    std::cout << dp[m] << std::endl;
}