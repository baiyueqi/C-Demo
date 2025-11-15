/* enigma.c
   使用你提供的三转子 wiring 实现的简易 Enigma 加密程序（带完整中文注释）
   - 按你的教材要求：最左边 fast rotor 每次按键都转动一次
   - fast 走一圈（回到 0）推动 medium 转动一次
   - medium 走一圈推动 slow 转动一次
   - Wiring 来自你上传的图片（已手动提取）
   - 使用 Reflector B（教学常用反射器）
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define N 26   // 26 个字母,定义常量 N = 26 表示字母数。后续所有数组、模运算都以 N 为基数，方便统一维护

/*-----------------------------------------
  将字母 A–Z/a–z 转换为 0–25
-----------------------------------------*/
static int letter_to_idx(char c) {
    if ('A' <= c && c <= 'Z') return c - 'A';
    if ('a' <= c && c <= 'z') return c - 'a';//利用ASCII码的连续性，将大写字母和小写字母分别映射到0-25的范围内
    return -1;   // 非字母返回 -1
}

/*-----------------------------------------
  将 0–25 转换为大写字母 A–Z
-----------------------------------------*/
static char idx_to_letter(int idx) {
    idx %= N;
    if (idx < 0) idx += N;
    return (char)('A' + idx);//(char) 将结果强制转换回字符类型
}

/*---------------------------------------------------
  图片中提取出的 3 个转子 wiring 映射
  每个数组表示：forward[i] = j
  即：输入位置 i（字母 i）经过该转子，输出为 j。
  （位置未旋转时的静态 wiring）
----------------------------------------------------*/

/* fast rotor（快速转子） forward 映射 */
static const int fast_forward_init[N] = {
    /* A  B  C  D  E  F  G  H  I  J  K  L  M  N  O  P  Q  R  S  T  U  V  W  X  Y  Z */
      23,24,25, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15,16,17,18,19,20,21,22, 0, 1, 2, 3
};

/* medium rotor（中速转子） forward 映射 */
static const int medium_forward_init[N] = {
    /* A  B  C  D  E  F  G  H  I  J  K  L  M  N  O  P  Q  R  S  T  U  V  W  X  Y  Z */
      19, 0,25, 4, 5, 6,11,22,13,10, 3, 2, 9,12,21,14, 8, 7,17,16,23,20,24, 1,15,18
};

/* slow rotor（慢速转子） forward 映射 */
static const int slow_forward_init[N] = {
    /* A  B  C  D  E  F  G  H  I  J  K  L  M  N  O  P  Q  R  S  T  U  V  W  X  Y  Z */
       7, 1,25, 5, 3, 6, 2,13,15,17,21,10,11, 8, 0,24,25,14,16,23,22,18,20,19, 4,12
};

/* Reflector B（标准反射器 B） */
static const char reflectorB_str[] = "YRUHQSLDPXNGOKMIEBFZCWVJAT";

/*---------------------------------------------------
  根据 forward 映射构造 backward（反向）映射
  backward[j] = i 代表：反向进入 j 时来自 i
----------------------------------------------------*/
void build_backward(const int forward[N], int backward[N]) {
    for (int i = 0; i < N; ++i) backward[i] = -1;
    for (int i = 0; i < N; ++i) {
        int out = forward[i];
        backward[out] = i;
    }
    for (int i = 0; i < N; ++i)
        if (backward[i] == -1)
            backward[i] = i;  // 理论上不会发生，但为了安全做填补
}

/*---------------------------------------------------
  构造反射器映射，将字符串转为整数数组
----------------------------------------------------*/
void build_reflector(const char *ref_str, int ref_map[N]) {
    for (int i = 0; i < N; ++i)
        ref_map[i] = letter_to_idx(ref_str[i]);
}

/*---------------------------------------------------
  rotor 结构体：包含
  - forward（正向表）
  - backward（反向表）
  - pos（当前旋转位置）
----------------------------------------------------*/
typedef struct {
    int forward[N];
    int backward[N];
    int pos;   // 当前旋转位移（0–25）
} Rotor;

/*---------------------------------------------------
  正向通过转子一次
  模拟 Enigma 真实电路：
  1. 输入 + 当前转子偏移 pos
  2. 查表 forward[]
  3. 输出 - pos
----------------------------------------------------*/
int rotor_forward(const Rotor *r, int entry) {
    int translated = (entry + r->pos) % N;   // 入口位置 + 转子旋转位移
    int wired = r->forward[translated];      // 查 forward wiring
    int exit = (wired - r->pos) % N;         // 减去旋转位移
    if (exit < 0) exit += N;
    return exit;
}

/*---------------------------------------------------
  反向通过转子一次（反射器回程）
  逻辑与 forward 相同，只是查 backward[]
----------------------------------------------------*/
int rotor_backward(const Rotor *r, int entry) {
    int translated = (entry + r->pos) % N;
    int wired = r->backward[translated];
    int exit = (wired - r->pos) % N;
    if (exit < 0) exit += N;
    return exit;
}

/*---------------------------------------------------
  转子步进逻辑（按你教材所写的规则）
  - 每次按键：fast 一定前进 1
  - fast 完整一圈 → medium 前进 1
  - medium 完整一圈 → slow 前进 1
----------------------------------------------------*/
void step_rotors(Rotor *fast, Rotor *medium, Rotor *slow) {
    fast->pos = (fast->pos + 1) % N;

    if (fast->pos == 0) { // fast 完成一圈
        medium->pos = (medium->pos + 1) % N;

        if (medium->pos == 0) // medium 也一圈，则 slow 前进
            slow->pos = (slow->pos + 1) % N;
    }
}

/*---------------------------------------------------
  加密单个字母（大写 0–25），流程：
  1. 转动转子
  2. fast → medium → slow 正向通过
  3. 反射器反射
  4. slow → medium → fast 反向通过
----------------------------------------------------*/
int enigma_encrypt_char(Rotor *fast, Rotor *medium, Rotor *slow,
                        const int reflector[N], int ch)
{
    step_rotors(fast, medium, slow);  // 每按一个键，先步进

    int x = rotor_forward(fast, ch);
    x = rotor_forward(medium, x);
    x = rotor_forward(slow, x);

    x = reflector[x]; // 反射

    x = rotor_backward(slow, x);
    x = rotor_backward(medium, x);
    x = rotor_backward(fast, x);

    return x;
}

/* 初始化 rotor */
void init_rotor(Rotor *r, const int forward_init[N], int initial_pos) {
    for (int i = 0; i < N; ++i)
        r->forward[i] = forward_init[i];

    build_backward(r->forward, r->backward);
    r->pos = (initial_pos % N + N) % N;
}

/* 打印当前转子位置 */
void print_positions(const Rotor *fast, const Rotor *medium, const Rotor *slow) {
    printf("当前转子位置: fast=%c  medium=%c  slow=%c\n",
           idx_to_letter(fast->pos),
           idx_to_letter(medium->pos),
           idx_to_letter(slow->pos));
}

/*---------------------------------------------------
  主程序：输入初始位，输入明文，输出密文
----------------------------------------------------*/
int main(void) {
    Rotor fast, medium, slow;
    int reflector[N];
    build_reflector(reflectorB_str, reflector);

    printf("=== Enigma 加密程序（使用你提供的三转子 wiring）===\n");
    printf("请输入初始转子位置（fast medium slow，例如 ABC）：");

    char buf[128];
    fgets(buf, sizeof(buf), stdin);

    int pfast = 0, pmed = 0, pslow = 0;
    int found = 0;

    /* 解析三个字母作为初始位置 */
    for (int i = 0; buf[i] && found < 3; ++i) {
        if (isalpha((unsigned char)buf[i])) {
            int v = toupper(buf[i]) - 'A';
            if (found == 0) pfast = v;
            else if (found == 1) pmed = v;
            else pslow = v;
            ++found;
        }
    }

    if (found < 3) {
        printf("输入不足，默认使用 AAA。\n");
        pfast = pmed = pslow = 0;
    }

    /* 初始化三个转子 */
    init_rotor(&fast, fast_forward_init, pfast);
    init_rotor(&medium, medium_forward_init, pmed);
    init_rotor(&slow, slow_forward_init, pslow);

    print_positions(&fast, &medium, &slow);

    printf("\n请输入明文（英文 letters），回车加密；空行退出。\n");

    while (1) {
        printf("\n明文> ");
        if (!fgets(buf, sizeof(buf), stdin)) break;
        if (buf[0] == '\n') break;

        char out[1024];
        int outpos = 0;

        for (int i = 0; buf[i] && outpos < 1023; ++i) {
            char c = buf[i];
            int idx = letter_to_idx(c);

            if (idx == -1) {
                out[outpos++] = c;  // 非字母原样输出
                continue;
            }

            int enc = enigma_encrypt_char(&fast, &medium, &slow, reflector, idx);
            char enc_c = idx_to_letter(enc);

            // 保持大小写
            if ('a' <= c && c <= 'z')
                enc_c = tolower(enc_c);

            out[outpos++] = enc_c;
        }
        out[outpos] = '\0';

        printf("密文> %s", out);
        print_positions(&fast, &medium, &slow);
    }

    printf("程序结束。\n");
    return 0;
}
