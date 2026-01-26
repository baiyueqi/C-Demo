#include <iostream>
#include <vector>
using namespace std;

using int64 = long long;

//公共函数：扩展欧几里得
int64 exgcd(int64 a, int64 b, int64 &x, int64 &y) {
    if(b==0){
        x=1;
        y=0;
        a=a>=0?a:-a;
    }    
    int64 x1, y1;
    int64 g = exgcd(b, a % b, x1, y1);
    x = y1;
    y = x1 - (a / b) * y1;
    return g;
}

//模逆元
bool modInverse(int64 a, int64 m, int64 &inv) {
    int64 x, y;
    int64 g = exgcd(a, m, x, y);
    if (g != 1) return false;
    inv = (x % m + m) % m;//确保逆元为非负数,把逆元调整到标准模范围 [0, m-1]
    return true;
}

//模重复平方
int64 modPow(int64 a, int64 b, int64 m) {
    int64 res = 1 % m;//防止 m = 1 的情况
    a %= m;//防止初始值过大
    while (b) {
        if (b & 1)//检查当前最低位是否为 1,判断是否为奇数
            res = (__int128)res * a % m;//当前位为1则乘上当前底数
        a = (__int128)a * a % m;
        b >>= 1;//右移一位指数，相当于除以2
    }
    return res;
}

//主函数
int main() {

    cout << "========================================\n";
    cout << " 实验 1：最大公因数与扩展欧几里得算法\n";
    cout << "========================================\n";

    // 验收 1（固定数据）
    int64 a = 24871395028LL;
    int64 b = 12435697514LL;
    int64 s, t;

    cout << "[步骤] 计算 gcd(" << a << ", " << b << ")\n";
    int64 g = exgcd(a, b, s, t);

    cout << "[结果] gcd = " << g << endl;
    cout << "[验证] s*a + t*b = "
         << s << "*" << a << " + "
         << t << "*" << b << " = "
         << s * a + t * b << endl;

    // 验收 2（学号）
    int64 id;
    cout << "\n请输入你的学号：";
    cin >> id;

    int64 s2, t2;
    int64 g2 = exgcd(id, id + 2025, s2, t2);

    cout << "[结果] gcd(" << id << ", " << id + 2025 << ") = " << g2 << endl;
    cout << "[验证] s*id + t*(id+2025) = "
         << s2 * id + t2 * (id + 2025) << endl;

    /* ===================================================== */

    cout << "\n========================================\n";
    cout << " 实验 2：模逆元计算\n";
    cout << "========================================\n";

    int64 inv;

    // 验收 1
    cout << "[测试 1] 求 19789 在模 23458 下的逆元\n";
    if (modInverse(19789, 23458, inv))
        cout << "[结果] 逆元 = " << inv << endl;
    else
        cout << "[结果] 不存在逆元\n";

    // 验收 2
    cout << "\n[测试 2] 求 31875 在模 428387 下的逆元\n";
    if (modInverse(31875, 428387, inv))
        cout << "[结果] 逆元 = " << inv << endl;
    else
        cout << "[结果] 不存在逆元\n";

    /* ===================================================== */

    cout << "\n========================================\n";
    cout << " 实验 3：中国剩余定理\n";
    cout << "========================================\n";

    vector<int64> r = {4, 6, 4, 2};//余数
    vector<int64> m = {10, 13, 7, 11};//模数

    int64 M = 1;
    for (int i = 0; i < 4; ++i) M *= m[i];//计算总模

    cout << "[步骤] 总模 M = 10 × 13 × 7 × 11 = " << M << endl;

    int64 x = 0;
    for (int i = 0; i < 4; ++i) {
        int64 Mi = M / m[i];
        int64 yi;//Mi 关于 m[i] 的逆元
        modInverse(Mi, m[i], yi);

        cout << "\n[子步骤 " << i + 1 << "]\n";
        cout << "模 = " << m[i]
             << ", 余数 = " << r[i] << endl;
        cout << "Mi = " << Mi
             << ", Mi^{-1} mod m = " << yi << endl;

        x += r[i] * Mi * yi;//累加求和
    }

    x = (x % M + M) % M;//调整到标准模范围 [0, M-1]

    cout << "\n[最终结果] 满足条件的最小正整数 x = " << x << endl;

    /* ===================================================== */

    cout << "\n========================================\n";
    cout << " 实验 4：快速幂 + 数论定理\n";
    cout << "========================================\n";

    // (1) 2^n mod 143
    cout << "[实验 4-1] 计算 2^n mod 143\n";
    cout << "请输入学号 n：";
    cin >> id;

    int64 ans1 = modPow(2, id, 143);
    cout << "[结果] 2^" << id << " mod 143 = " << ans1 << endl;

    // (2) 3^101 mod 2026
    cout << "\n[实验 4-2] 计算 3^101 mod 2026\n";
    int64 ans2 = modPow(3, 101, 2026);
    cout << "[结果] 3^101 mod 2026 = " << ans2 << endl;

    cout << "\n========================================\n";
    cout << " 所有实验完成\n";
    cout << "========================================\n";

    return 0;
}
