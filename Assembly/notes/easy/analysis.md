# Easy: Super easy password crack for kids

## 题目信息

- URL: https://crackmes.one/crackme/681cc54a6297cca3ff7d7743
- 难度: Easy / 1-2
- 文件名: `crackme.exe`
- 样本路径: `Assembly/binaries/easy_super_easy_password/extracted/crackme.exe`
- SHA256: `9e222e9ab72b1b96db6e154fa0e372759958d7632a0f7085ab64192e0bb2c1e1`
- 平台/架构: Windows PE32+ / x86-64 console
- 工具: Detect It Easy, IDA, x64dbg

## 程序行为

- 输入: 控制台提示 `Enter the password: ` 后输入一行字符串。
- 错误输入示例: `test`
- 失败提示: `Access denied!`
- 成功提示: `Access granted!`
- 是否闪退: 否。输入错误后回到输入循环，继续提示输入。

## 静态分析结论

- 关键字符串:
  - 输入提示: `Enter the password: `，地址 `0x140003408`
  - 失败提示: `Access denied!\n`，地址 `0x140003440`
  - 成功提示: `Access granted!\n`，地址 `0x140003428`
  - 疑似正确密码: `hello`，地址 `0x140003420`
- 字符串引用函数: 主校验函数 `sub_1400012A0`
- 判断成功/失败的汇编地址:
  - 长度判断: `0x140001326`
  - 字节比较循环: `0x140001331` - `0x140001342`
  - 比较结果判断: `0x14000134D` - `0x14000134F`
  - 失败分支: `0x140001351`
  - 成功分支: `0x1400013A1`
- 关键比较指令:
  - `cmp qword ptr [rsp+30h], 5` 判断输入长度是否为 5
  - `cmp cl, byte ptr [rsi+rax-1]` 逐字节比较用户输入和常量 `hello`
- 条件跳转:
  - `jne loc_140001351` 长度不是 5 时进入失败分支
  - `jne loc_140001348` 任一字符不匹配时进入失败路径
  - `je loc_1400013A1` 比较结果为 0 时进入成功分支
- 核心判断逻辑: 程序读取 `std::string` 输入，要求长度等于 5，并逐字节比较是否等于 `.rdata` 中的 `hello`。匹配则输出成功，否则输出失败并重新进入输入循环。

## 伪代码

```c
int main(void) {
    std::string input;
    const char *password = "hello";

    while (true) {
        std::cout << "Enter the password: ";
        std::cin >> input;

        if (input.length() == 5) {
            bool matched = true;
            for (int i = 0; i < 5; i++) {
                if (input[i] != password[i]) {
                    matched = false;
                    break;
                }
            }

            if (matched) {
                std::cout << "Access granted!\n";
                return 0;
            }
        }

        std::cout << "Access denied!\n";
    }
}
```

## 动态调试计划

- 断点 1: `0x1400012D0`，输出输入提示前，用于确认进入主输入循环。
- 断点 2: `0x140001326`，长度判断位置，用于观察 `[rsp+30h]` 中的输入长度。
- 断点 3: `0x140001338`，逐字节比较位置，用于观察 `cl` 和 `[rsi+rax-1]`。
- 断点 4: `0x14000134F`，成功/失败分支判断位置。
- 观察寄存器: `rcx`, `rdx`, `rax`, `rsp`
- 观察内存: 用户输入缓冲区、正确密码字符串地址 `0x140003420`

## 截图清单

- `Assembly/screenshots/easy/die/01_die_basic.png`: DIE 基本信息
- `Assembly/screenshots/easy/run/02_run_wrong_input.png`: 错误输入运行结果
- `Assembly/screenshots/easy/ida/02_strings_window.png`: IDA Strings 窗口
- `Assembly/screenshots/easy/ida/03_string_xrefs.png`: 关键字符串交叉引用
- `Assembly/screenshots/easy/ida/04_input_and_length_check.png`: 输入和长度判断
- `Assembly/screenshots/easy/ida/05_renamed_function1.png`: 函数重命名前后记录
- `Assembly/screenshots/easy/ida/05_renamed_function2.png`: 函数重命名前后记录
- `Assembly/screenshots/easy/ida/06_pseudocode.png`: IDA 伪代码
- `Assembly/screenshots/easy/x64dbg/01_length_check_wrong_input.png`: 错误输入时的长度判断断点
- `Assembly/screenshots/easy/x64dbg/02_compare_with_hello.png`: 正确输入时和 `hello` 的逐字节比较
- `Assembly/screenshots/easy/x64dbg/03_success_branch_hello.png`: `hello` 进入成功分支验证

## 验证结果

| 输入 | 预期结果 | 实际结果 | 是否通过 |
| --- | --- | --- | --- |
| `test` | 失败 | `Access denied!` | 是 |
| `hell` | 失败 | `Access denied!` | 是 |
| `hello` | 成功 | `Access granted!` | 是 |
