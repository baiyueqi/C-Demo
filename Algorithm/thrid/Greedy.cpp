#include <bits/stdc++.h>
using namespace std;

bool cmp(pair<int,int> a, pair<int,int> b) {
    return a.first > b.first; // 分数高优先
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(0);

    int n;
    cin >> n;

    vector<pair<int,int>> v(n);
    int mx = 0;

    for (int i = 0; i < n; i++) {
        cin >> v[i].first >> v[i].second;
        if (v[i].second > mx) mx = v[i].second;
    }

    sort(v.begin(), v.end(), cmp);

    // 标记时间是否被占用
    vector<int> used(mx + 1, 0);

    int res = 0;

    for (int i = 0; i < n; i++) {
        int ddl = v[i].second;

        // 从截止日期往前找
        for (int d = ddl; d >= 1; d--) {
            if (!used[d]) {
                used[d] = 1;
                res += v[i].first;
                break;
            }
        }
    }

    cout << res;
    return 0;
}