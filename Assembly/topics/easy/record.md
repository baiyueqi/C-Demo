# Easy 题目记录

## 基本信息

- 题目名称：Super easy password crack for kids
- Crackmes.one URL：https://crackmes.one/crackme/681cc54a6297cca3ff7d7743
- 难度等级：1.2
- 所属组合：A（算法 + 反调试）中的基础算法题
- 文件名：`crackme.exe`
- SHA256：`9e222e9ab72b1b96db6e154fa0e372759958d7632a0f7085ab64192e0bb2c1e1`
- 平台/架构：Windows PE32+ / x86-64 console
- 是否加壳：未见明显加壳，字符串和 C++ 运行库导入可直接识别。
- 分析人员：待填写

## 功能观察

- 程序输入：控制台输入一行密码字符串。
- 成功提示：`Access granted!`
- 失败提示：`Access denied!`
- 初步判断：程序使用固定字符串校验。输入长度必须为 5，并逐字节等于 `hello`。

## 静态分析记录

- 关键字符串：`Enter the password: `、`hello`、`Access granted!`、`Access denied!`
- 字符串交叉引用：从提示字符串和成功/失败字符串定位到 `sub_1400012A0`
- 关键函数地址：`0x1400012A0`
- 关键比较或跳转：
  - `0x140001326`: 判断输入长度是否为 5
  - `0x140001338`: 逐字节比较输入和 `hello`
  - `0x14000134F`: 成功/失败分支判断
  - `0x140001351`: 失败分支
  - `0x1400013A1`: 成功分支
- IDA/Ghidra 截图编号：`screenshots/easy/ida/02_strings_window.png` 至 `06_pseudocode.png`

## 汇编还原

关键汇编片段：

```asm
cmp     qword ptr [rsp+30h], 5
jne     loc_140001351
mov     rax, rbp

loc_140001331:
movzx   ecx, byte ptr [rdx+rax]
inc     rax
cmp     cl, byte ptr [rsi+rax-1]
jne     loc_140001348
cmp     rax, 5
jne     loc_140001331
mov     eax, ebp
jmp     loc_14000134D

loc_14000134D:
test    eax, eax
je      loc_1400013A1
```

伪代码：

```c
bool check_password(const std::string& input) {
    const char *password = "hello";
    if (input.length() != 5) {
        return false;
    }

    for (int i = 0; i < 5; i++) {
        if (input[i] != password[i]) {
            return false;
        }
    }

    return true;
}
```

## 动态调试记录

- 断点位置：`0x140001326`、`0x140001338`、`0x14000134F`
- 断点目的：确认长度判断、逐字节比较和最终分支。
- 关键寄存器：`rdx` 指向输入字符串缓冲区，`rsi` 指向常量 `hello`，`rax` 是比较循环下标，`cl` 是当前输入字符。
- 栈/内存观察：`[rsp+30h]` 为输入长度，`0x140003420` 为 `hello`。
- 成功分支：`0x1400013A1`
- 失败分支：`0x140001351`
- x64dbg 截图编号：
  - `screenshots/easy/x64dbg/01_length_check_wrong_input.png`
  - `screenshots/easy/x64dbg/02_compare_with_hello.png`
  - `screenshots/easy/x64dbg/03_success_branch_hello.png`

## 解决方案

- 解法类型：正确输入 + 验证脚本
- 解法说明：静态分析得到固定密码 `hello`，输入该字符串进入成功分支。
- 对应文件：`Assembly/scripts/easy_verify.py`

## 验证结果

| 编号 | 输入 | 结果 | 说明 |
| --- | --- | --- | --- |
| 1 | `test` | 失败 | 长度不为 5，进入失败分支 |
| 2 | `hell` | 失败 | 长度不为 5，进入失败分支 |
| 3 | `hello` | 成功 | 长度和逐字节比较均通过 |

## 难度分析

- 难点：需要区分真正的密码长度判断和 `std::string` 清理逻辑。失败分支后的 `cmp rdi, 0Fh` 是字符串存储清理判断，不是密码校验。
- 解决方法：从字符串交叉引用进入主函数，结合 `rsi = 0x140003420` 和比较循环确认固定密码。
- 耗时：约 1 小时以内。
- 不足：后续整理报告时需要为每张截图补充图号和文字说明。
