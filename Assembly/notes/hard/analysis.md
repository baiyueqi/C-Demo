# Hard: Intermediate Anti-Debug and Runtime Dynamic Password Challenge

## 样本信息

- URL: https://crackmes.one/crackme/68f3c25b2d267f28f69b76a6
- 文件名: `crackme.exe`
- 样本路径: `Assembly/binaries/hard_antidebug_runtime_password/extracted/crackme.exe`
- SHA256: `0fab24ee3a3ca60296259a895db01d67185b129cfa44e9c615bc02928352ab6a`
- 平台/架构: Windows PE32 / x86 console
- ImageBase: `0x00400000`
- EntryPoint: `0x004012E0`
- 调试器: x32dbg

## 已确认字符串

- 输入提示: `Enter The INT password: `，地址 `0x40711C`
- 输入错误提示: `Invalid input. Please enter a number.`，地址 `0x407138`
- 成功提示: `Congrats!`，地址 `0x40715E`
- 失败提示: `Incorrect Password Try Again: `，地址 `0x40716C`
- 编译器痕迹: `GCC: (MinGW.org GCC-6.3.0-1) 6.3.0`
- 随机数痕迹: `mt19937`

## 已确认导入和保护点

- `IsDebuggerPresent`，IAT thunk `0x404E30`
- `GetTickCount`，IAT thunk `0x404E40`
- `Sleep`，IAT thunk `0x404E00`
- `GetCurrentProcessId`
- `SetUnhandledExceptionFilter`
- `VirtualQuery`
- `VirtualProtect`

## 关键函数

- `0x401F57`: `main`
- `0x401E9A`: `checkDebugger`
- `0x40155A`: `CheckNtGlobalFlag`
- `0x4017AB`: `CheckGlobalFlagsClearInProcess`
- `0x40183F`: `CheckGlobalFlagsClearInFile`
- `0x401D89`: `MixValues`
- `0x401AB0`: `SpawnDecoy`
- `0x401460`: `GetPEB`

## 初步流程

1. CRT 入口 `0x4012E0` 进入运行时初始化。
2. 运行时调用 `main`，地址 `0x401F57`。
3. `main` 初始化 PEB、随机数引擎和运行时变量。
4. 程序多次调用 `checkDebugger`，例如：
   - `0x402018 -> 0x401E9A`
   - `0x4021AD -> 0x401E9A`
   - `0x402372 -> 0x401E9A`
   - `0x40258C -> 0x401E9A`
5. 输入一个整数密码后，程序用 `mt19937` 和 `MixValues` 混合运行时值。
6. 若比较通过，进入 `Congrats!` 分支；否则输出 `Incorrect Password Try Again: `。

## 反调试逻辑

`checkDebugger` 和主循环附近综合使用多种检测：

```c
if (IsDebuggerPresent()) detected = true;
if (CheckGlobalFlagsClearInFile()) detected = true;
if (CheckGlobalFlagsClearInProcess()) detected = true;
if (CheckNtGlobalFlag()) detected = true;
```

在 `0x402394` 附近可以看到直接检测：

```asm
402394  call 404E30 ; IsDebuggerPresent
402399  test eax, eax
40239B  jne  4023B8 ; detected
40239D  call 40183F ; CheckGlobalFlagsClearInFile
4023A2  test al, al
4023A4  jne  4023B8 ; detected
4023A6  call 4017AB ; CheckGlobalFlagsClearInProcess
4023AB  test al, al
4023AD  jne  4023B8 ; detected
4023AF  call 40155A ; CheckNtGlobalFlag
4023B4  test al, al
4023B6  je   4023BF ; not detected
4023B8  mov  eax, 1 ; detected
4023BF  mov  eax, 0 ; clean
```

## 下一步

1. 用 DIE 截图确认 PE32、x86、MinGW/GCC、是否加壳。
2. 用 IDA 打开样本，截图 Strings 窗口和导入表。
3. 在 IDA 中把上面的关键函数重命名。
4. 用 x32dbg 设置反调试断点，先证明反调试会被触发。
5. 绕过反调试后再分析密码生成与比较逻辑。
