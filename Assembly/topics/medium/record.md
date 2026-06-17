# Medium 题目记录

## 基本信息

- 题目名称：KeygenMe_3_SWD
- Crackmes.one URL：https://crackmes.one/crackme/69fa884dd7ff92e1214c0024
- 难度等级：3.0
- 所属组合：A（算法 + 反调试）中的算法/Keygen 题
- 文件名：`KeygenMe_3_SWD.exe`
- SHA256：`73758bc4a5fa71719bf031122c903feadfd7d5094bda87d997e3591937463958`
- 平台/架构：Windows PE32+ / x86-64 console
- 是否加壳：未见明显加壳，C++ 运行库和 CRT 导入可直接识别。
- 分析人员：待填写

## 功能观察

- 程序输入：依次输入 `Username`、`Secret code`、`Verification PIN`。
- 成功提示：程序最后输出 `Secret code ->` 和 `Verification PIN ->` 两项验证结果。
- 失败提示：格式错误时清除上一行并重新提示；算法验证失败时结果项为失败状态。
- 初步判断：程序对用户名长度和字符集做格式校验，再基于用户名生成 Secret code 与 Verification PIN 期望值。

## 静态分析记录

- 关键字符串：
  - 字符表: `0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ`，地址 `0x140007420`
  - 标题: `KeygenMe No 3`，地址 `0x1400074A0`
  - 用户名提示: `Username: `，地址 `0x1400074D8`
  - Secret code 提示: `Secret code: `，地址 `0x140007500`
  - Verification PIN 提示: `Verification PIN: `，地址 `0x140007528`
- 字符串交叉引用：主交互流程集中在 `0x140002BD0` 之后。
- 关键函数地址：
  - `0x140002BD0`：主交互/校验流程候选入口
  - `0x140001E30`：Username 字符格式校验，要求非空且全为字母或数字
  - `0x140001EC0`：Secret code 输入格式校验，要求 `XXXX-XXXXX-XXX`
  - `0x140001F80`：Verification PIN 输入格式校验，并转为整数
  - `0x140001FF0`：base36 字符串转整数
  - `0x140002100`：Secret code 核心校验函数
  - `0x1400025A0`：PIN 候选值生成函数
  - `0x140002910`：Verification PIN 核心比较函数
- 循环位置：
  - `0x140002300` - `0x140002399`：遍历用户名计算 Secret code 三个中间量
  - `0x1400026C9` - `0x140002808`：遍历 PIN 输入材料计算 32 位哈希
- XOR/加法/编码逻辑：程序先把用户名转为大写，再使用加权和、异或累积、乘法取模生成 Secret code 并编码为 base36；PIN 使用大写用户名、两个 32 位状态、循环移位、异或和乘法混合。
- IDA/Ghidra 截图编号：待补充。

## 汇编还原

关键汇编片段：

```asm
; Secret code 用户名遍历核心
140002345  mov eax, [rsp+40h]          ; eax = username[i]
140002351  imul rax, rcx               ; ch * (i + 1)
14000235f  mov [rsp+34h], eax          ; sum_weighted
140002370  xor rcx, rax                ; xor_mix ^= ch + i
140002385  imul ecx, eax               ; product_mix *= ch + 3
140002391  div ecx                     ; % 100000

; Secret code 三段比较
1400023A2  xor eax, 5A5Ah              ; part1
1400023B6  imul eax, [rsp+38h], 539h   ; part2
1400023E7  mov ecx, 0B640h             ; part3 modulo 46656
```

伪代码：

```c
username = uppercase(username);

uint32_t sum_weighted = 0;
uint32_t xor_mix = 0;
uint32_t product_mix = 1;

for (size_t i = 0; i < username.length(); i++) {
    uint32_t ch = username[i];
    sum_weighted += ch * (i + 1);
    xor_mix ^= ch + i;
    product_mix = (product_mix * (ch + 3)) % 100000;
}

part1 = (sum_weighted ^ 0x5A5A) % 46656;
part2 = (xor_mix * 1337 + product_mix) % 60466176;
part3 = (product_mix + sum_weighted + xor_mix) % 46656;

secret_code = base36(part1, 4) + "-" + base36(part2, 5) + "-" + base36(part3, 3);
```

## Keygen 设计

- 输入参数：用户名，长度 3 到 15，字符为字母或数字。
- 输出格式：Secret code 为 `XXXX-XXXXX-XXX`，Verification PIN 为十进制整数。
- 算法步骤：先按用户名生成 Secret code，再使用 `username + 'T' + secret_code` 作为 PIN 哈希输入材料。
- 对应脚本：`Assembly/scripts/medium_keygen.py`。

```python
python3 Assembly/scripts/medium_keygen.py Alice
```

## 动态调试记录

- 断点位置：
- 断点目的：
- 关键寄存器：
- 循环变量：
- 中间结果：
- 成功分支：
- 失败分支：
- x64dbg 截图编号：

## 验证结果

| 编号 | 输入 | 生成结果 | 程序验证结果 | 是否通过 |
| --- | --- | --- | --- | --- |
| 1 | `abc` | `0I50-02N6V-MB1` / `1846922373` | 待 x64dbg/Windows 实机确认 | 待补 |
| 2 | `Alice` | `0IMB-040EG-YZL` / `775865310` | 待 x64dbg/Windows 实机确认 | 待补 |
| 3 | `Student2026` | `0EOJ-01C98-62D` / `829076332` | 待 x64dbg/Windows 实机确认 | 待补 |

## 难度分析

- 难点：核心逻辑分散在格式校验、base36 转换、Secret code 比较、PIN 哈希和 PIN 二次变换多个函数中。
- 解决方法：先从字符串交叉引用定位主流程，再沿 `0x140002D69` 之后的两个布尔判断函数分别还原 Secret code 和 PIN。
- 耗时：静态反汇编约 2 小时，脚本复现约 30 分钟。
- 不足：当前 Linux 环境未直接运行 Windows PE，仍需在 Windows/x64dbg 中补 3 组实际通过截图。
