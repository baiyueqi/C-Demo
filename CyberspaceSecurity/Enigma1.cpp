/* enigma.cpp
   使用 1-26 编号系统的 Enigma 加密程序
*/

#include <iostream>
#include <string>
#include <vector>
#include <cctype>

const int N = 26;   // 26 个字母

/*-----------------------------------------
  Rotor 类：封装转子功能
-----------------------------------------*/
class Rotor {
private:
    std::vector<int> forward;
    std::vector<int> backward;
    int position;  // 位置使用 1-26 表示

    // 根据 forward 映射构造 backward（反向）映射
    void buildBackward() {
        for (int i = 1; i <= N; ++i) backward[i] = -1;
        for (int i = 1; i <= N; ++i) {
            int out = forward[i];
            backward[out] = i;
        }
        // 安全检查，确保所有位置都有映射
        for (int i = 1; i <= N; ++i) {
            if (backward[i] == -1) {
                backward[i] = i;
            }
        }
    }

public:
    // 构造函数：使用初始映射和位置初始化转子
    Rotor(const std::vector<int>& forward_init, int initial_pos = 1) 
        : forward(27, 0), backward(27, 0), position(initial_pos) {
        // 初始化 forward 数组（使用 1-26 索引）
        for (int i = 1; i <= N; ++i) {
            forward[i] = forward_init[i-1];
        }
        buildBackward();
    }

    // 正向通过转子一次
    int forwardPass(int entry) const {
        // entry 是 1-26 的字母编号
        int translated = (entry + position - 1) % N + 1;
        int wired = forward[translated];
        int exit_val = (wired - position + N) % N + 1;
        return exit_val;
    }

    // 反向通过转子一次
    int backwardPass(int entry) const {
        int translated = (entry + position - 1) % N + 1;
        int wired = backward[translated];
        int exit_val = (wired - position + N) % N + 1;
        return exit_val;
    }

    // 转子步进
    void step() {
        position = (position % N) + 1;
    }

    // 获取当前位置
    int getPosition() const {
        return position;
    }

    // 检查是否完成一圈
    bool isFullRotation() const {
        return position == 1;
    }

    // 设置位置
    void setPosition(int pos) {
        position = ((pos - 1) % N + N) % N + 1;
    }
};

/*-----------------------------------------
  EnigmaMachine 类：封装完整的 Enigma 机器
-----------------------------------------*/
class EnigmaMachine {
private:
    Rotor fast;
    Rotor medium;
    Rotor slow;
    std::vector<int> reflector;

    // 将字母 A–Z/a–z 转换为 1–26
    static int letterToIndex(char c) {
        if ('A' <= c && c <= 'Z') return c - 'A' + 1;
        if ('a' <= c && c <= 'z') return c - 'a' + 1;
        return -1;
    }

    // 将 1–26 转换为大写字母 A–Z
    static char indexToLetter(int idx) {
        idx = ((idx - 1) % N + N) % N + 1;
        return static_cast<char>('A' + idx - 1);
    }

    // 构造反射器映射（使用标准 Reflector B，转换为 1-26）
    static std::vector<int> buildReflector() {
        std::string reflectorB_str = "YRUHQSLDPXNGOKMIEBFZCWVJAT";
        std::vector<int> ref_map(27, 0);
        for (int i = 0; i < N; ++i) {
            ref_map[i+1] = letterToIndex(reflectorB_str[i]);
        }
        return ref_map;
    }

public:
    // 构造函数
    EnigmaMachine(const std::vector<int>& fast_wiring, 
                  const std::vector<int>& medium_wiring,
                  const std::vector<int>& slow_wiring)
        : fast(fast_wiring, 1), medium(medium_wiring, 1), slow(slow_wiring, 1),
          reflector(buildReflector()) {}

    // 设置转子初始位置
    void setPositions(int fast_pos, int medium_pos, int slow_pos) {
        fast.setPosition(fast_pos);
        medium.setPosition(medium_pos);
        slow.setPosition(slow_pos);
    }

    // 转子步进逻辑
    void stepRotors() {
        fast.step();

        if (fast.isFullRotation()) {
            medium.step();

            if (medium.isFullRotation()) {
                slow.step();
            }
        }
    }

    // 加密单个字母（1-26）
    int encryptChar(int ch) {
        stepRotors();  // 先步进转子

        // 正向通过三个转子
        int x = fast.forwardPass(ch);
        x = medium.forwardPass(x);
        x = slow.forwardPass(x);

        // 反射器反射
        x = reflector[x];

        // 反向通过三个转子
        x = slow.backwardPass(x);
        x = medium.backwardPass(x);
        x = fast.backwardPass(x);

        return x;
    }

    // 加密字符串
    std::string encrypt(const std::string& plaintext) {
        std::string ciphertext;
        
        for (char c : plaintext) {
            int idx = letterToIndex(c);
            
            if (idx == -1) {
                ciphertext += c;  // 非字母原样输出
                continue;
            }

            int enc = encryptChar(idx);
            char enc_c = indexToLetter(enc);

            // 保持大小写
            if (std::islower(static_cast<unsigned char>(c))) {
                enc_c = std::tolower(static_cast<unsigned char>(enc_c));
            }

            ciphertext += enc_c;
        }
        
        return ciphertext;
    }

    // 打印当前转子位置
    void printPositions() const {
        std::cout << "当前转子位置: fast=" << indexToLetter(fast.getPosition())
                  << "  medium=" << indexToLetter(medium.getPosition())
                  << "  slow=" << indexToLetter(slow.getPosition()) << std::endl;
    }

    // 调试函数：显示转子映射
    void debugRotors() {
        std::cout << "\n=== 转子映射调试 ===" << std::endl;
        std::cout << "Fast Rotor 位置: " << fast.getPosition() << std::endl;
        std::cout << "Medium Rotor 位置: " << medium.getPosition() << std::endl;
        std::cout << "Slow Rotor 位置: " << slow.getPosition() << std::endl;
    }
};

/*---------------------------------------------------
  从图片中提取的转子接线（使用 1-26 编号）
----------------------------------------------------*/

// Fast Rotor 接线（从图片右侧第一列）
const std::vector<int> fast_forward_init = {
    /* 输入: 1(A)  2(B)  3(C)  4(D)  5(E)  6(F)  7(G)  8(H)  9(I) 10(J) 11(K) 12(L) 13(M) 14(N) 15(O) 16(P) 17(Q) 18(R) 19(S) 20(T) 21(U) 22(V) 23(W) 24(X) 25(Y) 26(Z) */
    /* 输出: */ 21,    3,   15,    1,   19,   10,   14,   26,   20,    8,   16,    7,   22,    4,   11,    5,   17,    9,   12,   23,   18,    2,   25,    6,   24,   13
};

// Medium Rotor 接线（从图片中间第二列）
const std::vector<int> medium_forward_init = {
    /* 输入: 1(A)  2(B)  3(C)  4(D)  5(E)  6(F)  7(G)  8(H)  9(I) 10(J) 11(K) 12(L) 13(M) 14(N) 15(O) 16(P) 17(Q) 18(R) 19(S) 20(T) 21(U) 22(V) 23(W) 24(X) 25(Y) 26(Z) */
    /* 输出: */ 20,    1,    6,    4,   15,    3,   14,   12,   23,    5,   16,    2,   22,   19,   11,   18,   25,   24,   13,    7,   10,   21,    9,   26,   17,   14
};

// Slow Rotor 接线（从图片左侧第三列）
const std::vector<int> slow_forward_init = {
    /* 输入: 1(A)  2(B)  3(C)  4(D)  5(E)  6(F)  7(G)  8(H)  9(I) 10(J) 11(K) 12(L) 13(M) 14(N) 15(O) 16(P) 17(Q) 18(R) 19(S) 20(T) 21(U) 22(V) 23(W) 24(X) 25(Y) 26(Z) */
    /* 输出: */ 20,    1,    6,    4,   15,    3,   14,   12,   23,    5,   16,    2,   22,   19,   11,   18,   25,   24,   13,    7,   10,   21,    9,   26,   17,   14
};

/*---------------------------------------------------
  主程序
----------------------------------------------------*/
int main() {
    // 创建 Enigma 机器实例
    EnigmaMachine enigma(fast_forward_init, medium_forward_init, slow_forward_init);

    std::cout << "=== Enigma 加密程序（使用 1-26 编号系统）===" << std::endl;
    std::cout << "初始转子位置: AAA" << std::endl;

    // 设置初始位置为 AAA (1,1,1)
    enigma.setPositions(1, 1, 1);
    enigma.printPositions();

    std::cout << "\n请输入明文（英文 letters），回车加密；空行退出。" << std::endl;

    while (true) {
        std::cout << "\n明文> ";
        std::string input;
        std::getline(std::cin, input);
        
        if (input.empty()) break;

        std::string ciphertext = enigma.encrypt(input);
        std::cout << "密文> " << ciphertext << std::endl;
        enigma.printPositions();
    }

    std::cout << "程序结束。" << std::endl;
    return 0;
}