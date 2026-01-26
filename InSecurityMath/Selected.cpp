#include <iostream>
#include <vector>
#include <bitset>
using namespace std;

//第一部分：GF(2) 多项式运算
// 多项式用 unsigned int 的二进制表示
// bit i = x^i 的系数
// 计算多项式的次数
int degree(unsigned int a) {
    for (int i = 31; i >= 0; --i)
        if (a & (1u << i)) return i;// 找到最高位
    return -1;
}

// GF(2) 多项式取模
unsigned int poly_mod(unsigned int a, unsigned int b) {
    int da = degree(a);
    int db = degree(b);
    while (da >= db && a != 0) {
        a ^= (b << (da - db));//异或，加法=减法，b的最高次对齐a的最高次
        da = degree(a);
    }
    return a;
}

// 欧几里得算法
unsigned int poly_gcd(unsigned int a, unsigned int b) {
    while (b != 0) {
        unsigned int r = poly_mod(a, b);
        cout << "  余式 = ";
        cout << "0b" << bitset<8>(r) << endl;
        a = b;
        b = r;
    }
    return a;
}

// 打印多项式
void print_poly(unsigned int a) {
    bool first = true;
    for (int i = 31; i >= 0; --i) {
        if (a & (1u << i)) {
            if (!first) cout << " + ";
            if (i == 0) cout << "1";
            else if (i == 1) cout << "x";
            else cout << "x^" << i;
            first = false;
        }
    }
    cout << endl;
}

//第二部分：原根判断
// 快速幂模运算
long long mod_pow(long long a, long long b, long long m) {
    long long r = 1 % m;
    a %= m;
    while (b) {
        if (b & 1) r = r * a % m;//判断指数当前最低位是不是 1
        a = a * a % m;
        b >>= 1;
    }
    return r;
}

// 求 n 的所有素因子
vector<long long> prime_factors(long long n) {
    vector<long long> f;
    for (long long i = 2; i * i <= n; ++i) {
        if (n % i == 0) {
            f.push_back(i);
            while (n % i == 0) n /= i;
        }
    }
    if (n > 1) f.push_back(n);
    return f;
}

// 判断原根
bool is_primitive_root(long long a, long long p) {
    long long phi = p - 1;
    auto factors = prime_factors(phi);// 求素因子

    cout << "p - 1 = " << phi << "，素因子：";
    for (auto x : factors) cout << x << " ";
    cout << endl;

    for (auto q : factors) {
        long long exp = phi / q;// 计算 a^(phi/q) mod p,使用原根判定定理
        long long val = mod_pow(a, exp, p);
        cout << "  " << a << "^(" << exp << ") mod "
             << p << " = " << val << endl;
        if (val == 1) return false;
    }
    return true;
}

//主函数
int main() {

    cout << "========================================\n";
    cout << "（1）GF(2) 上多项式最大公因式\n";
    cout << "========================================\n";

    // f(x)=x^6+x^4+x+1 → 1010011
    // g(x)=x^4+x+1     → 10011
    unsigned int f = 0b1110;
    unsigned int g = 0b1001;

    cout << "f(x) = ";
    print_poly(f);
    cout << "g(x) = ";
    print_poly(g);

    cout << "\n[欧几里得算法过程]\n";
    unsigned int d = poly_gcd(f, g);

    cout << "\n[结果] gcd(f, g) = ";
    print_poly(d);

    /* ================================================= */

    cout << "\n========================================\n";
    cout << "（2）原根判断\n";
    cout << "========================================\n";

    cout << "\n【验收 1】证明 5 是 17 的原根\n";
    if (is_primitive_root(5, 17))
        cout << "结论：5 是 17 的原根\n";
    else
        cout << "结论：5 不是 17 的原根\n";

    cout << "\n【验收 2】验证 3 是否是 17 的原根\n";
    if (is_primitive_root(3, 17))
        cout << "结论：3 是 17 的原根\n";
    else
        cout << "结论：3 不是 17 的原根\n";

    cout << "\n【验收 3】验证 2 是否是 11 的原根\n";
    if (is_primitive_root(2, 11))
        cout << "结论：2 是 11 的原根\n";
    else
        cout << "结论：2 不是 11 的原根\n";

    return 0;
}
