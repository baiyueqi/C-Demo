#include <bits/stdc++.h>
using namespace std;

static long long b[2050];

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    cin >> n;

    vector<int> v(n);

    for (int i = 0; i < n; i++) {
        cin >> v[i];
    }

    long long ans = 0;

    for (int i = n - 1; i >= 0; i--) {
        int cur = v[i];

        long long s = 0;
        for (int j = 1; j < cur; j++) {
            s += b[j];
        }

        ans += s;
        b[cur]++;
    }

    cout << ans << "\n";
    return 0;
}