# 汇编语言与逆向工程课程设计实验报告

# 一、低难度题：Super easy password crack for kids

## 1. 题目信息

- 来源：https://crackmes.one/crackme/681cc54a6297cca3ff7d7743
- 难度：1.2 / Easy
- 文件名：`crackme.exe`
- 本地路径：`Assembly/binaries/easy_super_easy_password/extracted/crackme.exe`
- SHA256：`9e222e9ab72b1b96db6e154fa0e372759958d7632a0f7085ab64192e0bb2c1e1`
- 平台：Windows PE32+ / x86-64 console
- 工具：Detect It Easy、IDA、x64dbg、Python

## 2. 程序功能分析

该程序是基础控制台密码校验程序。运行后读取用户输入，错误时输出 `Access denied!` 并继续循环，正确时输出 `Access granted!`。功能上没有网络、文件和加密流程，校验目标集中在固定字符串比较。

## 3. 静态分析

DIE 显示样本是 PE64 控制台程序，未见明显加壳。图 1-1 先确认了样本平台和基本属性，说明后续可以直接使用 IDA 和 x64dbg 分析。

![图 1-1 DIE 识别 Easy 样本基本信息|600](../screenshots/easy/die/01_die_basic.png)

IDA Strings 窗口中可以直接看到输入提示、成功/失败提示和疑似密码：

```text
Enter the password:
hello
Access granted!
Access denied!
```

图 1-2 中的 `hello` 与成功、失败字符串集中出现，是本题最直接的线索。这里先从字符串入手，比从入口点逐行跟踪更快定位校验逻辑。

![图 1-2 IDA Strings 窗口定位关键字符串|600](../screenshots/easy/ida/02_strings_window.png)

对字符串做交叉引用后，定位到 `sub_1400012A0`。该函数先判断输入长度，再逐字节比较输入内容和 `.rdata` 中的 `hello`。

![图 1-3 关键字符串交叉引用|600](../screenshots/easy/ida/03_string_xrefs.png)

关键地址：

| 地址 | 含义 |
| --- | --- |
| `0x140001326` | 输入长度是否为 5 |
| `0x140001338` | 当前字符比较 |
| `0x14000134F` | 成功/失败分支判断 |
| `0x140001351` | 失败分支 |
| `0x1400013A1` | 成功分支 |

核心汇编：

```asm
cmp     qword ptr [rsp+30h], 5
jne     loc_140001351
movzx   ecx, byte ptr [rdx+rax]
cmp     cl, byte ptr [rsi+rax-1]
jne     loc_140001348
test    eax, eax
je      loc_1400013A1
```

图 1-4 展示了长度判断和字符比较位置。输入长度不等于 5 时会直接跳到失败分支，长度正确后才进入逐字节比较。

![图 1-4 输入读取和长度判断逻辑|600](../screenshots/easy/ida/04_input_and_length_check.png)

图 1-5 是伪代码还原结果，可以清楚看到校验条件已经简化为固定字符串 `hello`。

![图 1-5 IDA 伪代码还原|600](../screenshots/easy/ida/06_pseudocode.png)

## 4. 动态调试（断点 + 寄存器）

x64dbg 断点：

| 断点 | 用途 |
| --- | --- |
| `0x140001326` | 观察长度判断 |
| `0x140001338` | 观察逐字节比较 |
| `0x14000134F` | 观察是否跳转成功分支 |

寄存器和内存：

- `rdx`：用户输入缓冲区。
- `rsi`：常量 `hello`。
- `rax`：循环下标。
- `cl`：当前输入字符。
- `[rsp+30h]`：输入长度。

调试时输入 `hello`，在比较循环中每个字符均匹配，最终进入 `0x1400013A1` 成功分支。

图 1-6 使用错误输入观察长度判断，能够验证静态分析中“先判断长度”的结论。

![图 1-6 x64dbg 命中长度判断断点|600](../screenshots/easy/x64dbg/01_length_check_wrong_input.png)

图 1-7 停在字符比较处，可以直接观察用户输入缓冲区与常量 `hello` 的逐字节匹配过程。

![图 1-7 x64dbg 观察输入与 hello 的逐字节比较|600](../screenshots/easy/x64dbg/02_compare_with_hello.png)

图 1-8 输入 `hello` 后进入成功分支，动态结果与静态结论一致。

![图 1-8 输入 hello 后进入成功分支|600](../screenshots/easy/x64dbg/03_success_branch_hello.png)

## 5. 核心算法（伪代码）

```c
bool check_password(string input) {
    if (input.length() != 5) {
        return false;
    }

    for (int i = 0; i < 5; i++) {
        if (input[i] != "hello"[i]) {
            return false;
        }
    }

    return true;
}
```

## 6. 解决方案

正确输入为：

```text
hello
```

验证脚本：

```text
Assembly/scripts/easy_verify.py
```

## 7. 验证结果

| 编号 | 输入 | 程序结果 | 结论 |
| --- | --- | --- | --- |
| 1 | `test` | `Access denied!` | 错误 |
| 2 | `hell` | `Access denied!` | 错误 |
| 3 | `hello` | `Access granted!` | 正确 |

## 8. 难度分析

本题是基础题，关键在于从字符串交叉引用快速定位主逻辑。主要干扰来自 C++ 标准库生成的字符串处理代码，但核心比较本身很直接。耗时约 1 小时以内。

---

# 二、中等难度题：KeygenMe_3_SWD

## 1. 题目信息

- 来源：https://crackmes.one/crackme/69fa884dd7ff92e1214c0024
- 难度：3.0 / Medium
- 文件名：`KeygenMe_3_SWD.exe`
- 本地路径：`Assembly/binaries/medium_keygenme_3_swd/extracted/KeygenMe_3_SWD.exe`
- SHA256：`73758bc4a5fa71719bf031122c903feadfd7d5094bda87d997e3591937463958`
- 平台：Windows PE32+ / x86-64 console
- 工具：Detect It Easy、IDA、x64dbg、Python

## 2. 程序功能分析

程序要求输入用户名、Secret code 和 Verification PIN。用户名必须是 3 到 15 位字母或数字；Secret code 必须符合 `XXXX-XXXXX-XXX`；PIN 为十进制整数。程序的本质是根据用户名推导两项校验值。

## 3. 静态分析

先用 DIE 确认 Medium 样本属性。图 2-1 显示该文件是 Windows x64 控制台程序，后续继续使用 IDA 和 x64dbg。

![图 2-1 DIE 识别 Medium 样本基本信息|600](../screenshots/medium/die/01_die_basic.png)

关键字符串：

```text
0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ
KeygenMe No 3
Username:
Secret code:
Verification PIN:
```

图 2-2 中的 base36 字符表 `0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ` 是 Secret code 分段编码的重要线索；三个输入提示则对应主流程的三次输入。

![图 2-2 IDA Strings 窗口定位交互字符串|600](../screenshots/medium/ida/01_strings_window.png)

从字符串交叉引用进入 `0x140002BD0` 附近主交互流程。程序读取三项输入后，先做格式校验，再调用 Secret code 和 PIN 校验函数。

![图 2-3 主交互流程读取 Username、Secret code 和 PIN|600](<../screenshots/medium/ida/02_main_interaction _1.png>)

关键函数：

| 地址 | 功能 |
| --- | --- |
| `0x140001E30` | Username 格式校验 |
| `0x140001EC0` | Secret code 格式校验 |
| `0x140001F80` | PIN 格式校验 |
| `0x140001FF0` | base36 转整数 |
| `0x140002100` | Secret code 校验 |
| `0x1400025A0` | PIN 候选值生成 |
| `0x140002910` | PIN 比较 |

图 2-4 是 Secret code 校验中的用户名遍历循环。每个字符按位置参与加权和、异或和乘法取模，说明结果与 Username 强绑定。

![图 2-4 Secret code 用户名遍历循环|600](../screenshots/medium/ida/03_secret_code_loop.png)

图 2-5 展示 Secret code 三段比较，输入值会按 `XXXX-XXXXX-XXX` 拆分后分别比较。

![图 2-5 Secret code 三段比较逻辑|600](../screenshots/medium/ida/04_secret_code_compare.png)

图 2-6 是 PIN 候选值生成中的哈希混合循环，包含循环移位、异或、加法和乘法 avalanche，是本题动态验证的重点。

![图 2-6 PIN 哈希混合循环|600](../screenshots/medium/ida/05_pin_hash_loop.png)

图 2-7 是 PIN 最终比较逻辑，`EAX` / `AL` 的返回值会决定验证是否通过。

![图 2-7 PIN 最终比较逻辑|600](../screenshots/medium/ida/06_pin_compare.png)

## 4. 动态调试

动态调试选择 `Alice` 作为样例：

```text
Username: Alice
Secret code: 0IMB-040EG-YZL
Verification PIN: 775865310
```

断点设置：

| 断点 | 目的 |
| --- | --- |
| `keygenme_3_swd.exe:$2DD9` | Secret code 校验调用前 |
| `keygenme_3_swd.exe:$2DDE` | Secret code 返回后 |
| `keygenme_3_swd.exe:$2FAE` | PIN 校验调用前 |
| `keygenme_3_swd.exe:$2FB3` | PIN 返回后 |

关键寄存器：

- `EAX` / `AL`：校验返回值。
- `RCX`、`RDX`、`R8`：参数传递。
- 栈局部变量：保存 PIN 整数和 Secret code 字符串对象。

图 2-8 停在 Secret code 校验调用前，用于确认 `Alice` 和脚本生成的 Secret code 已经传入校验函数。

![图 2-8 Secret code 校验调用前断点|600](../screenshots/medium/x64dbg/01_before_secret_check.png)

图 2-9 是 Secret code 返回后的位置，返回值显示该项校验通过。

![图 2-9 Secret code 校验返回通过|600](../screenshots/medium/x64dbg/02_secret_check_pass.png)

图 2-10 停在 PIN 校验调用前，便于检查 PIN 整数和前面已通过的输入是否一致。

![图 2-10 PIN 校验调用前断点|600](../screenshots/medium/x64dbg/03_before_pin_check.png)

图 2-11 显示 PIN 校验返回通过，说明 Keygen 中复现的哈希算法与程序内部结果一致。

![图 2-11 PIN 校验返回通过|600](../screenshots/medium/x64dbg/04_pin_check_pass.png)

## 5. 核心算法（伪代码）

Secret code：

```c
username = uppercase(username);
sum_weighted = 0;
xor_mix = 0;
product_mix = 1;

for (i = 0; i < username.length(); i++) {
    ch = username[i];
    sum_weighted += ch * (i + 1);
    xor_mix ^= ch + i;
    product_mix = (product_mix * (ch + 3)) % 100000;
}

part1 = (sum_weighted ^ 0x5A5A) % 46656;
part2 = (xor_mix * 1337 + product_mix) % 60466176;
part3 = (product_mix + sum_weighted + xor_mix) % 46656;
```

PIN：

```c
a = 0xA3B1C2D3;
b = 0x1F2E3D4C;
material = uppercase(username) + char(14 ^ 0x5A) + secret_code;

for each byte ch in material:
    a ^= ch + index * 0x11;
    a = rol32(a, index % 5 + 3);
    a += b ^ 0x9E3779B9;
    b ^= a + ch * 0x83;
    b = ror32(b, index % 7 + 2);
    b += (a << 3) ^ 0x7F4A7C15;
    if index is odd: swap(a, b);

pin = final_avalanche(a ^ b) & 0x7FFFFFFF;
```

## 6. 解决方案

实现 Keygen：

```text
Assembly/scripts/medium_keygen.py
```

运行：

```bash
python3 Assembly/scripts/medium_keygen.py Alice
```

输出可直接作为程序输入。

## 7. 验证结果

| 编号 | Username | Secret code | Verification PIN | 结论 |
| --- | --- | --- | --- | --- |
| 1 | `abc` | `0I50-02N6V-MB1` | `1846922373` | 通过 |
| 2 | `Alice` | `0IMB-040EG-YZL` | `775865310` | 通过 |
| 3 | `Student2026` | `0EOJ-01C98-62D` | `829076332` | 通过 |

图 2-12 到图 2-14 是三组运行验证截图，覆盖短用户名、普通字母用户名和字母数字混合用户名，均能通过验证。

![图 2-12 abc 验证通过|600](../screenshots/medium/run/01_run_abc.png)

![图 2-13 Alice 验证通过|600](../screenshots/medium/run/02_run_alice.png)

![图 2-14 Student2026 验证通过|600](../screenshots/medium/run/03_run_student2026.png)

## 8. 难度分析

本题难点在于算法分散。Secret code 和 PIN 不在同一函数中完成，必须先识别格式校验，再进入真实校验函数。PIN 部分混合了循环移位、异或、加法和乘法 avalanche，手工计算容易出错，因此使用脚本复现最可靠。

---

# 三、高难度题：Intermediate Anti-Debug and Runtime Dynamic Password Challenge

## 1. 题目信息

- 来源：https://crackmes.one/crackme/68f3c25b2d267f28f69b76a6
- 难度：5.0 / Hard
- 文件名：`crackme.exe`
- 本地路径：`Assembly/binaries/hard_antidebug_runtime_password/extracted/crackme.exe`
- SHA256：`0fab24ee3a3ca60296259a895db01d67185b129cfa44e9c615bc02928352ab6a`
- 平台：Windows PE32 / x86 console
- ImageBase：`0x00400000`
- EntryPoint：`0x004012E0`
- 工具：Detect It Easy、IDA、x32dbg、Python、MSYS2

## 2. 程序功能分析

样本是带反调试保护的整数口令程序。正常流程会出现：

```text
Enter The INT password:
```

错误输入输出 `Incorrect Password Try Again:`，正确输入输出 `Congrats!`。本题分析重点是恢复动态调试能力，而不是继续求解最终动态口令。

## 3. 静态分析

先用 DIE 查看样本基础信息。图 3-1 显示该文件是 PE32 x86 程序，因此动态调试时要使用 x32dbg，并且依赖库也必须是 32 位版本。

![图 3-1 DIE 识别 Hard 样本为 PE32 x86|600](../screenshots/hard/die/01_die_basic.png)

IDA Strings 窗口中可以定位到输入提示、失败提示和成功提示。图 3-2 说明程序表面上仍是整数口令校验，但前置难点在于进入可调试状态。

![图 3-2 IDA Strings 窗口定位输入和成功失败字符串|600](../screenshots/hard/ida/01_strings_window.png)

样本导入多个反调试相关 API：

```text
IsDebuggerPresent
GetTickCount
Sleep
SetUnhandledExceptionFilter
VirtualQuery
VirtualProtect
```

图 3-3 展示了导入表中的反调试相关 API。`IsDebuggerPresent` 用于直接检测调试器，`Sleep` 和 `GetTickCount` 可用于时间检测或延时干扰，`VirtualProtect` 则可能用于运行时 patch 或代码页权限变化。

![图 3-3 导入表中的反调试相关 API|600](../screenshots/hard/ida/03_imports_antidebug.png)

关键地址：

| 地址 | 功能 |
| --- | --- |
| `0x401F57` | main |
| `0x401E9A` | checkDebugger |
| `0x40155A` | CheckNtGlobalFlag |
| `0x4017AB` | CheckGlobalFlagsClearInProcess |
| `0x40183F` | CheckGlobalFlagsClearInFile |
| `0x401AB0` | SpawnDecoy |

还存在 TLS callback：

```text
0x402D00
0x402CB0
```

图 3-4 是 `IsDebuggerPresent` 交叉引用。可以看到检测点不止一个，因此只处理单个调用点并不可靠。

![图 3-4 IsDebuggerPresent 交叉引用|600](../screenshots/hard/ida/04_isdebuggerpresent_xrefs.png)

内联检测位于 `0x402394` 到 `0x4023C4`，检测成功后跳到 `0x4023B8` 设置 `eax = 1`。

图 3-5 是 `checkDebugger` 函数主体，内部组合了 API 检测、PEB/NtGlobalFlag 相关检测和文件/进程标志检查。该函数适合作为统一 patch 点。

![图 3-5 checkDebugger 函数|600](../screenshots/hard/ida/05_check_debugger.png)

图 3-6 显示 main 中还存在内联反调试循环，所以除了 patch `checkDebugger`，还需要处理 `0x4023B8` 附近的 detected 结果。

![图 3-6 main 中的内联反调试循环|600](../screenshots/hard/ida/06_main_antidebug_loop.png)

## 4. 动态调试

直接调试时先遇到环境问题：

- `0xC0000135`：缺少 DLL。
- `0xC000007B`：DLL 位数错误。

解决方式是从 MSYS2 `mingw32` 目录复制：

```text
libgcc_s_dw2-1.dll
libstdc++-6.dll
libwinpthread-1.dll
```

动态调试采用挂起进程附加方式。脚本：

```text
Assembly/scripts/hard_launch_for_attach.py
```

断点和观察点：

| 位置 | 目的 |
| --- | --- |
| `0x402D00` / `0x402CB0` | TLS callback |
| `0x4012E0` | 程序入口 |
| `0x401F57` | main |
| `0x401E9A` | checkDebugger |
| `0x4023B8` | 内联 detected 分支结果 |

关键寄存器：

- `EAX` / `AL`：检测结果。
- `EIP`：确认是否进入退出路径。

图 3-7 是附加进程后的模块状态，证明补齐 32 位 DLL 后程序已经可以正常加载。

![图 3-7 x32dbg 附加后 DLL 已载入|600](../screenshots/hard/x32dbg/01_attach_dll_loaded.png)

图 3-8 展示关键断点设置，覆盖 TLS callback、入口点、main、`checkDebugger` 和内联 detected 分支，便于确认执行流没有提前退出。

![图 3-8 x32dbg 设置关键断点|600](../screenshots/hard/x32dbg/02_breakpoints_set.png)

图 3-9 命中入口点 `int3`，说明挂起进程再 attach 的方法能够在主程序执行前取得控制权。

![图 3-9 命中入口点 int3|600](../screenshots/hard/x32dbg/03_entry_int3_hit.png)

图 3-10 恢复入口点原始指令，避免临时断点影响后续执行。

![图 3-10 恢复入口点原始指令|600](../screenshots/hard/x32dbg/04_entry_restored.png)

图 3-11 是 `checkDebugger` patch 后的结果。让该函数直接返回 0，可以绕过主流程中多处复用的检测。

![图 3-11 checkDebugger patch 结果|600](../screenshots/hard/x32dbg/05_checkdebugger_patched.png)

图 3-12 显示程序已经进入 `Enter The INT password:`，说明运行库问题、附加时机和反调试检测都已处理完成。

![图 3-12 反调试绕过后进入输入提示|600](../screenshots/hard/x32dbg/06_input_prompt_after_bypass.png)

## 5. 核心算法（伪代码）

反调试检测伪代码：

```c
bool checkDebugger() {
    if (IsDebuggerPresent()) return true;
    if (CheckGlobalFlagsClearInFile()) return true;
    if (CheckGlobalFlagsClearInProcess()) return true;
    if (CheckNtGlobalFlag()) return true;
    return false;
}

if (checkDebugger()) {
    Sleep(random_delay);
    SpawnDecoy();
    ExitProcess(0);
}
```

内联检测伪代码：

```c
detected = IsDebuggerPresent()
        || CheckGlobalFlagsClearInFile()
        || CheckGlobalFlagsClearInProcess()
        || CheckNtGlobalFlag();

if (detected) {
    enter_decoy_or_exit_path();
}
```

## 6. 解决方案

脚本先创建挂起进程，再 patch 反调试位置：

```asm
401E9A  xor eax, eax
401E9C  ret
4023B8  mov eax, 0
```

运行方式：

```powershell
cd C:\Temp\hard
python .\hard_launch_for_attach.py
```

流程：

1. 脚本输出 PID。
2. x32dbg 使用 `File -> Attach` 选择该 PID。
3. 回 PowerShell 按回车恢复主线程。
4. 程序进入 `Enter The INT password:`。

## 7. 验证结果

| 编号 | 操作 | 结果 | 结论 |
| --- | --- | --- | --- |
| 1 | 未放运行库 | `0xC0000135` | 确认缺少 DLL |
| 2 | 使用错误位数 DLL | `0xC000007B` | 确认 DLL 位数错误 |
| 3 | 补齐 32 位 DLL 并 patch 反调试 | 出现 `Enter The INT password:` | 绕过成功 |

## 8. 难度分析

本题难点在动态调试前置条件。程序自身反调试和运行库依赖问题叠加，导致“调试结束”不一定都是反调试。组员 B 的主要工作是把环境错误、附加时机和反调试 patch 分离处理，最终用挂起进程方式稳定进入输入提示。

---

# 四、综合分析

## 1. 难度差异

| 题目 | 难度 | 表现 |
| --- | --- | --- |
| Easy | 低 | 固定字符串 `hello`，逻辑短 |
| Medium | 中 | 多阶段算法，需 Keygen |
| Hard | 高 | 运行库、TLS、反调试和附加调试问题叠加 |

## 2. 技术点差异

Easy 偏基础静态分析；Medium 偏算法还原和脚本实现；Hard 偏动态调试对抗和环境排查。三题覆盖了字符串比较、编码/哈希算法、反调试绕过三个层次。

## 3. 分析方法差异

- Easy：Strings -> Xrefs -> 条件跳转 -> 动态验证。
- Medium：主流程定位 -> 函数拆解 -> 伪代码还原 -> Python Keygen -> 三组验证。
- Hard：运行库排查 -> TLS/反调试定位 -> 挂起进程 -> 内存 patch -> x32dbg attach 验证。

## 4. 时间成本

| 题目 | 估计耗时 | 主要消耗 |
| --- | --- | --- |
| Easy | 1 小时以内 | 截图和基础验证 |
| Medium | 2 到 3 小时 | 算法还原和 Keygen 调试 |
| Hard | 3 到 4 小时 | 环境排查和反调试绕过 |

---

# 五、个人分工与总结

## 1. 个人分工

| 姓名 | 学号 | 分工 |
| --- | --- | --- |
| 刘宁 |  | 静态分析、样本信息整理、IDA 字符串与交叉引用定位、关键函数命名、汇编逻辑还原、伪代码和报告整理 |
| 赵旭 |  | 动态调试与验证、x64dbg/x32dbg 断点设置、寄存器和内存观察、运行截图采集、Hard 题反调试附加流程调试 |

- 三个题目均由两名组员共同参与，每个题目都包含静态分析、动态调试和结果验证。
- 组员 A 主要负责每个题目的静态分析：DIE/IDA 基本识别、字符串窗口和交叉引用分析、关键函数定位、核心汇编还原、伪代码和算法说明整理。
- 组员 B 主要负责每个题目的动态调试：x64dbg/x32dbg 断点设置、寄存器观察、输入验证、截图采集、Hard 题附加调试和反调试绕过验证。
- 脚本部分共同完成：A 侧重算法复现和 Keygen 输出正确性，B 侧重脚本运行环境、进程创建、attach 流程和 patch 效果验证。
- 报告部分共同完成：A 侧重静态分析章节，B 侧重动态调试、验证结果和截图说明。

## 2. 个人总结

组员 B 在三题中主要承担动态调试和验证工作。Easy 和 Medium 题通过断点、寄存器和返回值验证静态结论；Hard 题则进一步处理运行库依赖、调试器附加时机和反调试 patch。这个过程说明逆向分析不能只停留在反汇编结果上，还必须通过真实运行和调试截图证明分析结论。
