#include <bits/stdc++.h>
using namespace std;

const int MAXV = 1e6 + 10;

vector<int> primeList;
vector<char> isPrimeFlag;

void buildPrimeTable(int limit) {
    isPrimeFlag.assign(limit + 1, true);
    isPrimeFlag[0] = isPrimeFlag[1] = false;

    for (int i = 2; i <= limit; ++i) {
        if (isPrimeFlag[i]) {
            primeList.push_back(i);
            if (1LL * i * i <= limit) {
                for (long long k = 1LL * i * i; k <= limit; k += i) {
                    isPrimeFlag[k] = false;
                }
            }
        }
    }
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int val;
    cin >> val;

    buildPrimeTable(val);

    int operations = 0;

    while (val >= 4) {
        bool changed = false;

        for (int idx = 0; idx < (int)primeList.size(); ++idx) {
            int small = primeList[idx];
            if (small > val) break;

            int rest = val - small;
            if (rest >= 0 && isPrimeFlag[rest]) {
                val = val - 2 * small;
                operations++;
                changed = true;
                break;
            }
        }

        if (!changed) break; // 防止异常情况死循环
    }

    cout << operations << "\n";
    return 0;
}