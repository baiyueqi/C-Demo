# Hard 题目记录

## 基本信息

- 题目名称：Intermediate Anti-Debug and Runtime Dynamic Password Challenge
- Crackmes.one URL：https://crackmes.one/crackme/68f3c25b2d267f28f69b76a6
- 难度等级：5.0
- 文件名：`crackme.exe`
- 样本路径：`Assembly/binaries/hard_antidebug_runtime_password/extracted/crackme.exe`
- SHA256：`0fab24ee3a3ca60296259a895db01d67185b129cfa44e9c615bc02928352ab6a`
- 平台/架构：Windows PE32 / x86 console
- ImageBase：`0x00400000`
- EntryPoint：`0x004012E0`
- 调试器：x32dbg
- 是否加壳：未见明显加壳，重点为反调试和运行时动态口令

## 功能观察

- 程序输入：整数口令，提示字符串为 `Enter The INT password: `
- 成功提示：`Congrats!`
- 失败提示：`Incorrect Password Try Again: `
- 异常退出或反调试现象：
  - 直接用 x32dbg 打开时可能显示“调试结束”。
  - 缺少 32 位 MinGW 运行库时，进程退出码为 `0xC0000135`。
  - 运行库位数错误时，进程退出码为 `0xC000007B`。
  - 恢复执行后若未绕过反调试，程序会进入延迟、诱饵进程或 `ExitProcess` 路径。
- 初步判断：样本为 32 位 MinGW C++ 程序，使用 API、PEB 标志和内联检测组合反调试。

## 静态分析记录

- 关键字符串：
  - `Enter The INT password: `：`0x40711C`
  - `Invalid input. Please enter a number.`：`0x407138`
  - `Congrats!`：`0x40715E`
  - `Incorrect Password Try Again: `：`0x40716C`
- 关键函数地址：
  - `0x401F57`：`main`
  - `0x401E9A`：`checkDebugger`
  - `0x40155A`：`CheckNtGlobalFlag`
  - `0x4017AB`：`CheckGlobalFlagsClearInProcess`
  - `0x40183F`：`CheckGlobalFlagsClearInFile`
  - `0x401AB0`：`SpawnDecoy`
  - `0x401460`：`GetPEB`
- TLS callback：
  - `0x402D00`
  - `0x402CB0`
- 反调试 API：
  - `IsDebuggerPresent`
  - `GetTickCount`
  - `Sleep`
  - `SetUnhandledExceptionFilter`
  - `VirtualQuery`
  - `VirtualProtect`
  - `GetCurrentProcessId`
- 运行库依赖：
  - `libgcc_s_dw2-1.dll`
  - `libstdc++-6.dll`
  - `libwinpthread-1.dll`

## 汇编还原

`checkDebugger` 函数在 `0x401E9A` 处执行组合检测：

```asm
401EAA  call 404E30        ; IsDebuggerPresent
401EAF  test eax, eax
401EB1  jne  401ECE        ; detected
401EB3  call 40183F        ; CheckGlobalFlagsClearInFile
401EB8  test al, al
401EBA  jne  401ECE        ; detected
401EBC  call 4017AB        ; CheckGlobalFlagsClearInProcess
401EC1  test al, al
401EC3  jne  401ECE        ; detected
401EC5  call 40155A        ; CheckNtGlobalFlag
401ECA  test al, al
401ECC  je   401ED5        ; clean
401ECE  mov  eax, 1        ; detected
```

`main` 附近还存在一段内联反调试检测，不能只 patch `checkDebugger`：

```asm
402394  call 404E30        ; IsDebuggerPresent
402399  test eax, eax
40239B  jne  4023B8        ; detected
40239D  call 40183F        ; CheckGlobalFlagsClearInFile
4023A2  test al, al
4023A4  jne  4023B8        ; detected
4023A6  call 4017AB        ; CheckGlobalFlagsClearInProcess
4023AB  test al, al
4023AD  jne  4023B8        ; detected
4023AF  call 40155A        ; CheckNtGlobalFlag
4023B4  test al, al
4023B6  je   4023BF        ; clean
4023B8  mov  eax, 1        ; detected
4023BF  mov  eax, 0        ; clean
4023C4  test al, al
```

伪代码：

```c
bool detected = false;
if (IsDebuggerPresent()) detected = true;
if (CheckGlobalFlagsClearInFile()) detected = true;
if (CheckGlobalFlagsClearInProcess()) detected = true;
if (CheckNtGlobalFlag()) detected = true;

if (detected) {
    Sleep(random_delay);
    SpawnDecoy();
    ExitProcess(0);
}
```

## 反调试或混淆分析

- 保护类型：API 检测、PEB/NtGlobalFlag 检测、进程映像标志检测、TLS callback、异常处理和延迟退出。
- 触发位置：
  - `0x401E9A`：集中式 `checkDebugger`
  - `0x402394` 到 `0x4023C4`：主流程内联检测
  - `0x401F16`、`0x401F4F`、`0x40256B`：检测后退出路径
- 判断依据：
  - 导入表包含 `IsDebuggerPresent`、`Sleep`、`SetUnhandledExceptionFilter`、`VirtualQuery`、`VirtualProtect`。
  - x32dbg 直接打开时提前结束。
  - patch 检测结果后程序可进入正常输入提示。
- 绕过方法：
  - 先补齐 32 位 MinGW 运行库：`libgcc_s_dw2-1.dll`、`libstdc++-6.dll`、`libwinpthread-1.dll`。
  - 使用脚本 `Assembly/scripts/hard_launch_for_attach.py` 调用 `CreateProcessW` 并设置 `CREATE_SUSPENDED`，让进程先以挂起状态出现在 x32dbg 的 Attach 列表。
  - 脚本在恢复主线程前使用 `WriteProcessMemory` patch 内存：

```asm
401E9A  xor eax, eax
401E9C  ret
4023B8  mov eax, 0
```

- 绕过后验证：x32dbg attach 后恢复主线程，程序控制台出现 `Enter The INT password: `，说明反调试检测被绕过，主流程恢复到输入阶段。

## 动态调试记录

- 断点位置：
  - `0x402D00`、`0x402CB0`：TLS callback
  - `0x4012E0`：程序入口点
  - `0x401F57`：`main`
  - `0x401E9A`：集中式反调试函数
  - `0x4023B8`：内联反调试 detected 结果
- 断点目的：确认 TLS、入口、主函数和反调试检测位置。
- 关键寄存器：反调试判断主要看 `EAX` / `AL`，非零表示检测到调试器。
- 栈/内存观察：脚本 patch 目标为进程内存，不修改原始文件。
- x32dbg 截图编号：
  - `01_runtime_dlls_ready.png`：运行库 DLL 已补齐
  - `02_suspended_patch_pid.png`：脚本创建挂起进程并 patch
  - `03_x32dbg_attach_loaded.png`：x32dbg attach 成功
  - `04_checkdebugger_patched.png`：`0x401E9A` patch 结果
  - `05_inline_antidebug_patched.png`：`0x4023B8` patch 结果
  - `06_input_prompt_after_bypass.png`：绕过后进入输入提示

## 解决方案

- 解法类型：反调试绕过 / 内存 Patch / 辅助脚本
- 解法说明：
  1. 将样本复制到 Windows 本地目录，例如 `C:\Temp\hard\`。
  2. 从 MSYS2 `mingw32` 目录复制 32 位运行库：

```text
C:\msys64\mingw32\bin\libgcc_s_dw2-1.dll
C:\msys64\mingw32\bin\libstdc++-6.dll
C:\msys64\mingw32\bin\libwinpthread-1.dll
```

  3. 运行脚本：

```powershell
cd C:\Temp\hard
python .\hard_launch_for_attach.py
```

  4. x32dbg 使用 `File -> Attach` 附加脚本输出的 PID。
  5. 回到 PowerShell 按回车恢复主线程。
  6. 程序进入 `Enter The INT password: ` 输入提示。

- 对应文件：`Assembly/scripts/hard_launch_for_attach.py`

## 验证结果

| 编号 | 操作 | 结果 | 是否通过 |
| --- | --- | --- | --- |
| 1 | 未补运行库直接启动 | `0xC0000135`，缺少 DLL | 通过，确认依赖问题 |
| 2 | 复制错误位数 DLL | `0xC000007B`，DLL 位数不匹配 | 通过，确认必须使用 32 位 DLL |
| 3 | 补齐 32 位 DLL 并执行挂起附加脚本 | x32dbg 可 attach，恢复后出现 `Enter The INT password: ` | 通过，确认绕过反调试 |

## 难度分析

- 难点：
  - 样本不是单一 `IsDebuggerPresent` 检测，还包含 PEB/NtGlobalFlag、文件映像标志、TLS callback 和内联检测。
  - x32dbg 直接打开时容易在入口前或检测点后退出，不便手动下断点。
  - MinGW 运行库缺失和 DLL 位数错误会造成与反调试相似的“调试结束”现象，需要先排除环境问题。
- 解决方法：
  - 先用退出码区分环境问题和反调试问题。
  - 使用 `CREATE_SUSPENDED` 创建进程，先 patch 后 attach。
  - 同时 patch `checkDebugger` 和内联检测结果，避免漏掉主流程检测点。
- 不足：
  - 本记录只完成反调试绕过和动态调试入口恢复，未继续还原动态口令算法。
