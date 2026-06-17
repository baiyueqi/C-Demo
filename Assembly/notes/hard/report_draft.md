# Hard 反调试分析报告草稿

## 样本概况

本题样本为 Crackmes.one 上的 `Intermediate Anti-Debug and Runtime Dynamic Password Challenge`，文件名为 `crackme.exe`。样本是 Windows PE32 x86 console 程序，SHA256 为 `0fab24ee3a3ca60296259a895db01d67185b129cfa44e9c615bc02928352ab6a`，入口点为 `0x004012E0`，主要函数位于 `0x401F57`。

本阶段只分析反调试和动态调试恢复，不继续求解最终密码。

## 环境问题排查

直接在 x32dbg 中打开样本时会出现“调试结束”的现象。排查后发现需要先区分运行库问题和反调试问题：

- `0xC0000135`：缺少 DLL，样本依赖 32 位 MinGW 运行库。
- `0xC000007B`：DLL 位数错误，通常是误用了 64 位 DLL。

最终在 `C:\Temp\hard\` 中放置样本和 32 位运行库：

```text
crackme.exe
hard_launch_for_attach.py
libgcc_s_dw2-1.dll
libstdc++-6.dll
libwinpthread-1.dll
```

其中 DLL 来自 MSYS2 的 `C:\msys64\mingw32\bin\`，不是 `mingw64` 目录。

## 反调试点

静态分析确认样本导入并使用了多个反调试相关 API，包括 `IsDebuggerPresent`、`SetUnhandledExceptionFilter`、`VirtualQuery`、`VirtualProtect`、`Sleep` 和 `GetTickCount`。样本还存在 TLS callback：

```text
0x402D00
0x402CB0
```

集中式反调试函数位于 `0x401E9A`，会依次检查 `IsDebuggerPresent`、PEB/NtGlobalFlag 和进程映像标志。除此之外，`main` 附近 `0x402394` 到 `0x4023C4` 还有一段内联检测。因此只 patch `0x401E9A` 不够，还需要处理 `0x4023B8` 的 detected 分支结果。

## 绕过方法

为避免进程在 x32dbg 创建阶段或 TLS/入口前后快速退出，编写辅助脚本 `Assembly/scripts/hard_launch_for_attach.py`。脚本做三件事：

1. 使用 `CreateProcessW` 和 `CREATE_SUSPENDED` 创建挂起进程。
2. 在进程恢复前使用 `WriteProcessMemory` patch 反调试逻辑。
3. 输出 PID，供 x32dbg 通过 `File -> Attach` 附加。

内存 patch 内容如下：

```asm
401E9A  xor eax, eax
401E9C  ret
4023B8  mov eax, 0
```

这样 `checkDebugger` 恒返回未检测到调试器，内联检测即使跳到 detected 位置，也会被改写为 `mov eax,0`。

## 动态验证

执行脚本：

```powershell
cd C:\Temp\hard
python .\hard_launch_for_attach.py
```

脚本输出 patch 信息和 PID 后，在 x32dbg 中选择 `File -> Attach` 附加该进程。附加成功后回到 PowerShell 按回车恢复主线程。程序最终显示：

```text
Enter The INT password:
```

这说明反调试逻辑已被绕过，程序恢复到正常输入流程。

## 截图清单

建议报告中使用以下截图：

```text
Assembly/screenshots/hard/x64dbg/01_runtime_dlls_ready.png
Assembly/screenshots/hard/x64dbg/02_suspended_patch_pid.png
Assembly/screenshots/hard/x64dbg/03_x32dbg_attach_loaded.png
Assembly/screenshots/hard/x64dbg/04_checkdebugger_patched.png
Assembly/screenshots/hard/x64dbg/05_inline_antidebug_patched.png
Assembly/screenshots/hard/x64dbg/06_input_prompt_after_bypass.png
```
