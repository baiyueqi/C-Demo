/* enigma.c
   简易 Enigma 加密程序（根据图表配置）
   - 三转子 wiring 根据图表设置
   - 使用 Reflector B
   - 支持大小写保持和非字母输出
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define N 26   // 26 个字母

/*-----------------------------------------
  将字母 A–Z/a–z 转换为 0–25
-----------------------------------------*/
static int letter_to_idx(char c) {
    if ('A' <= c && c <= 'Z') return c - 'A';
    if ('a' <= c && c <= 'z') return c - 'a';
    return -1;   // 非字母返回 -1
}

/*-----------------------------------------
  将 0–25 转换为大写字母 A–Z
-----------------------------------------*/
static char idx_to_letter(int idx) {
    idx %= N;
    if (idx < 0) idx += N;
    return (char)('A' + idx);
}

/*-----------------------------------------
  根据图表重新定义三转子 wiring
  注意：图表中的数字1-26对应A-Z，需要转换为0-25
-----------------------------------------*/
static const int fast_forward_init[N] = {
   23, 24, 25, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22  // 根据快转子列调整
};

static const int medium_forward_init[N] = {
   20, 2, 14, 0, 18, 4, 16, 6, 22, 8, 24, 10, 25, 11, 3, 12, 5, 13, 7, 15, 9, 17, 1, 19, 21, 23   // 根据中转子列调整
};

static const int slow_forward_init[N] = {
   25, 19, 0, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 1, 20, 21, 22, 23, 24, 2   // 根据慢转子列调整
};

/* Reflector B */
static const char reflectorB_str[] = "YRUHQSLDPXNGOKMIEBFZCWVJAT";

/*-----------------------------------------
  构造 backward 映射
-----------------------------------------*/
void build_backward(const int forward[N], int backward[N]) {
    for (int i = 0; i < N; ++i) backward[i] = -1;
    for (int i = 0; i < N; ++i)
        backward[forward[i]] = i;
    for (int i = 0; i < N; ++i)
        if (backward[i] == -1) backward[i] = i;
}

/* 构造 reflector 映射 */
void build_reflector(const char *ref_str, int ref_map[N]) {
    for (int i = 0; i < N; ++i)
        ref_map[i] = letter_to_idx(ref_str[i]);
}

/*-----------------------------------------
  rotor 结构体
-----------------------------------------*/
typedef struct {
    int forward[N];
    int backward[N];
    int pos;
} Rotor;

/* 正向通过转子 */
int rotor_forward(const Rotor *r, int entry) {
    int translated = (entry + r->pos) % N;
    int wired = r->forward[translated];
    int exit = (wired - r->pos) % N;
    if (exit < 0) exit += N;
    return exit;
}

/* 反向通过转子 */
int rotor_backward(const Rotor *r, int entry) {
    int translated = (entry + r->pos) % N;
    int wired = r->backward[translated];
    int exit = (wired - r->pos) % N;
    if (exit < 0) exit += N;
    return exit;
}

/* 转子步进 */
void step_rotors(Rotor *fast, Rotor *medium, Rotor *slow) {
    fast->pos = (fast->pos + 1) % N;
    if (fast->pos == 0) {
        medium->pos = (medium->pos + 1) % N;
        if (medium->pos == 0)
            slow->pos = (slow->pos + 1) % N;
    }
}

/* 加密单个字符 */
int enigma_encrypt_char(Rotor *fast, Rotor *medium, Rotor *slow,
                        const int reflector[N], int ch)
{
    step_rotors(fast, medium, slow);

    int x = rotor_forward(fast, ch);
    x = rotor_forward(medium, x);
    x = rotor_forward(slow, x);

    x = reflector[x];

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

/* 测试特定输入输出 */
void test_specific(Rotor *fast, Rotor *medium, Rotor *slow, const int reflector[N]) {
    printf("\n=== 测试特定序列 ===\n");
    
    // 重置到初始位置
    init_rotor(fast, fast_forward_init, 0);  // A
    init_rotor(medium, medium_forward_init, 0); // A  
    init_rotor(slow, slow_forward_init, 0);  // A
    
    char test_input[] = "ABC";
    printf("输入: %s\n", test_input);
    printf("输出: ");
    
    for (int i = 0; test_input[i]; ++i) {
        int idx = letter_to_idx(test_input[i]);
        if (idx != -1) {
            int enc = enigma_encrypt_char(fast, medium, slow, reflector, idx);
            printf("%c", idx_to_letter(enc));
        }
    }
    printf("\n");
    
    // 测试单步后的状态
    printf("\n单步后位置: fast=%c medium=%c slow=%c\n", 
           idx_to_letter(fast->pos), idx_to_letter(medium->pos), idx_to_letter(slow->pos));
}

/*-----------------------------------------
  主程序
-----------------------------------------*/
int main(void) {
    Rotor fast, medium, slow;
    int reflector[N];
    build_reflector(reflectorB_str, reflector);

    printf("=== Enigma 加密程序（根据图表配置）===\n");
    
    // 测试特定序列
    test_specific(&fast, &medium, &slow, reflector);

    printf("\n请输入初始转子位置（fast medium slow，例如 AAA）：");
    
    char buf[128];
    if (!fgets(buf, sizeof(buf), stdin)) {
        printf("读取输入失败，默认使用 AAA。\n");
        buf[0]='A'; buf[1]='A'; buf[2]='A'; buf[3]='\0';
    }

    int pfast = 0, pmed = 0, pslow = 0;
    int found = 0;

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
                out[outpos++] = c;
                continue;
            }

            int enc = enigma_encrypt_char(&fast, &medium, &slow, reflector, idx);
            char enc_c = idx_to_letter(enc);

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
