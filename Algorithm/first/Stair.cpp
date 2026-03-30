//台阶问题
#include <iostream>
#include<vector>
using namespace std;

const int MOD = 100003;

int main() {    
    int N, K;
    cin >> N >> K;
    vector<int> met(N + 1, 0);
    vector<int> pre(N + 1, 0);
    met[0] = 1;
    pre[0] = 1;

    for (int i = 1; i <= N; i++) {
        int left = max(0, i - K);
        int right = i - 1;
        met[i] = pre[right];
        if (left > 0) {
            met[i] = (met[i] - pre[left - 1] + MOD) % MOD;
        }
        pre[i] = (pre[i - 1] + met[i]) % MOD;
    }
    cout << met[N] % MOD << '\n';
    return 0;
}