#include <iostream>
#include <cstdlib>
#include <ctime>

using namespace std;

//------------------------------
// 反复平方乘算法：快速模幂
//------------------------------
long long modPow(long long base, long long exponent, long long modulus) {
    long long result = 1;
    base %= modulus;
    while (exponent > 0) {
        if (exponent % 2 == 1) { // 当前位是1
            result = (result * base) % modulus;
        }
        base = (base * base) % modulus; // 平方
        exponent /= 2; // 移到下一位
    }
    return result;
}

//------------------------------
// 米勒–拉宾素数检测
//------------------------------
bool millerRabin(long long n, int k = 5) {
    if (n == 2 || n == 3) return true;
    if (n < 2 || n % 2 == 0) return false;

    long long d = n - 1;
    int s = 0;
    while (d % 2 == 0) {
        d /= 2;
        s++;
    }

    for (int i = 0; i < k; i++) {
        long long a = 2 + rand() % (n - 3);
        long long x = modPow(a, d, n);
        if (x == 1 || x == n - 1) continue;

        bool passed = false;
        for (int r = 0; r < s - 1; r++) {
            x = modPow(x, 2, n);
            if (x == n - 1) {
                passed = true;
                break;
            }
        }
        if (!passed) return false;
    }
    return true;
}

//------------------------------
// 生成大素数（2~maxNum）
//------------------------------
long long generatePrime(long long maxNum) {
    long long p;
    do {
        p = rand() % (maxNum - 2) + 2;
    } while (!millerRabin(p));
    return p;
}

//------------------------------
// 扩展欧几里得算法求模逆
//------------------------------
long long gcdExtended(long long a, long long b, long long &x, long long &y) {
    if (b == 0) {
        x = 1;
        y = 0;
        return a;
    }
    long long x1, y1;
    long long gcd = gcdExtended(b, a % b, x1, y1);
    x = y1;
    y = x1 - (a / b) * y1;
    return gcd;
}

long long modInverse(long long e, long long phi) {
    long long x, y;
    long long g = gcdExtended(e, phi, x, y);
    if (g != 1) {
        cout << "不存在模逆" << endl;
        return -1;
    } else {
        return (x % phi + phi) % phi;
    }
}

//------------------------------
// RSA 主函数
//------------------------------
int main() {
    srand(time(0));

    // 1. 选择两个大素数 p 和 q
    long long maxPrime = 100000; // 可调大素数范围
    long long p = generatePrime(maxPrime);
    long long q = generatePrime(maxPrime);
    while (q == p) q = generatePrime(maxPrime);

    cout << "选取素数 p=" << p << ", q=" << q << endl;

    // 2. 计算 n 和 φ(n)
    long long n = p * q;
    long long phi = (p - 1) * (q - 1);

    // 3. 选择公钥 e
    long long e = 65537;

    // 4. 计算私钥 d
    long long d = modInverse(e, phi);

    cout << "公钥 (e,n)=(" << e << "," << n << ")" << endl;
    cout << "私钥 d=" << d << endl;

    // 5. 测试加密解密
    long long message;
    cout << "请输入明文（整数）: ";
    cin >> message;

    long long cipher = modPow(message, e, n);
    cout << "加密后密文: " << cipher << endl;

    long long decrypted = modPow(cipher, d, n);
    cout << "解密后明文: " << decrypted << endl;

    return 0;
}
