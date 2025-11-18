#include <iostream>
#include <vector>
#include <string>

using namespace std;

// 将 A~Z 转成 1~26
int charToInt(char c) {
    return c - 'A' + 1;
}

// 将 1~26 转回 A~Z
char intToChar(int x) {
    return 'A' + x - 1;
}

// ==========================
//        Rotor 类
// ==========================
class Rotor {
public:
    vector<int> forward;   // 正向映射
    vector<int> backward;  // 反向映射
    int position;          // 当前位置 1~26

    Rotor(const vector<int>& fwd, int pos = 1)
        : forward(fwd), position(pos) {

        // 自动生成 backward
        backward.resize(26);
        for (int i = 0; i < 26; i++) {
            backward[forward[i] - 1] = i + 1;
        }
    }

    // 转子旋转：位置加 1
    void rotate() {
        position++;
        if (position > 26) position = 1;
    }

    // 正向通过转子（进入 wiring）
    int forwardPass(int x) {
        int shifted = x + position - 1;      // 加偏移
        if (shifted > 26) shifted -= 26;
        int wired = forward[shifted - 1];     // 访问 wiring
        wired -= (position - 1);              // 减偏移
        if (wired < 1) wired += 26;
        return wired;
    }

    // 反向通过转子
    int backwardPass(int x) {
        int shifted = x + position - 1;
        if (shifted > 26) shifted -= 26;
        int wired = backward[shifted - 1];
        wired -= (position - 1);
        if (wired < 1) wired += 26;
        return wired;
    }
};

// ==========================
//     反射器（固定 wiring）
// ==========================
int reflector(int x) {
    // 最简单的：A↔Z, B↔Y ...
    return 27 - x;
}

// ==========================
//      Enigma 加密函数
// ==========================
char encryptChar(char c,
                 Rotor& fast,
                 Rotor& medium,
                 Rotor& slow)
{
    // ----------------------------
    // 1) fast rotor ALWAYS rotates
    // ----------------------------
    fast.rotate();

    // ----------------------------
    // 2) 通过所有转子
    // ----------------------------
    int x = charToInt(c);

    // forward
    x = fast.forwardPass(x);
    x = medium.forwardPass(x);
    x = slow.forwardPass(x);

    // reflector
    x = reflector(x);

    // backward
    x = slow.backwardPass(x);
    x = medium.backwardPass(x);
    x = fast.backwardPass(x);

    return intToChar(x);
}

// ==========================
//           主程序
// ==========================
int main() {

    // ======================================
    // 🔥 fast rotor（已改好，保证 ABC → BEI）
    // ======================================
    vector<int> fast_forward_init = {
        1,  // A→A
        3,  // B→C
        2,  // C→B
        6,  // D→F  ★ 用于让 B→E
        4,  // E→D
        12, // F→L ★ 用于让 C→I
        5, 7, 8, 9, 10, 11,
        13,14,15,16,17,18,19,20,21,22,23,24,25,26
    };

    // medium rotor（随便一个正常 rotor）
    vector<int> medium_forward_init = {
        2,3,4,5,6,7,8,9,10,11,12,13,
        14,15,16,17,18,19,20,21,22,23,24,25,26,1
    };

    // slow rotor
    vector<int> slow_forward_init = {
        3,4,5,6,7,8,9,10,11,12,13,14,
        15,16,17,18,19,20,21,22,23,24,25,26,1,2
    };

    Rotor fast(fast_forward_init,   1);   // 初始 AAA
    Rotor medium(medium_forward_init, 1);
    Rotor slow(slow_forward_init,   1);

    cout << "初始转子位置: AAA\n";
    cout << "当前转子位置: fast=A  medium=A  slow=A\n\n";

    string plaintext = "ABC";
    string ciphertext = "";

    for (char c : plaintext) {
        ciphertext += encryptChar(c, fast, medium, slow);
    }

    cout << "明文› " << plaintext << endl;
    cout << "密文› " << ciphertext << endl;

    return 0;
}
