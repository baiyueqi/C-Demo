#include <iostream>
#include <vector>
using namespace std;

int main() {
    int n, k;
    cin >> n >> k;

    vector<int> arr(n);
    for (int i = 0; i < n; i++) {
        cin >> arr[i];
    }

    // dp[i][len]
    vector<vector<long long>> f(n, vector<long long>(k + 1, 0));

    for (int i = 0; i < n; i++) {
        f[i][1] = 1; // 单个元素
    }

    for (int i = 0; i < n; i++) {
        for (int j = 0; j < i; j++) {
            if (arr[j] < arr[i]) {
                for (int len = 2; len <= k; len++) {
                    f[i][len] += f[j][len - 1];
                }
            }
        }
    }

    long long res = 0;
    for (int i = 0; i < n; i++) {
        res += f[i][k];
    }

    cout << res << endl;

    return 0;
}