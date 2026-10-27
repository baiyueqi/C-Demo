# NixOS/Linux 环境实验说明

## 为什么需要一个 Linux 复现版

原题目程序是 Windows 程序，使用了 `stricmp` 和 `cmd.exe`。在 NixOS/WSL 环境中无法直接编译运行。

Linux 版使用 `strcasecmp` 替代 `stricmp`，使用 `true` 替代 `cmd.exe`。

## Linux 下栈布局差异

Clang x86_64 在 `-O0` 下不一定把 `is_admin` 放在 `user_buffer` 后面。为了让漏洞能够在 Linux 下稳定复现，`SSEx_vul_linux_exploit.c` 使用结构体固定局部变量布局：

```c
struct locals
{
    char user_buffer[64];
    int is_admin;
};
```

反汇编结果：

```asm
state 起始地址   = rbp - 0x58
user_buffer     = rbp - 0x58
is_admin        = rbp - 0x18
```

所以：

```text
user_buffer -> is_admin = 0x58 - 0x18 = 0x40 = 64 字节
```

这与 Windows x86 汇编中的偏移一致。

## 一键运行

```bash
cd /home/nixos/workspace/C-Demo/Software
bash run_linux_exploit.sh
```

或者手动执行：

```bash
clang -std=c17 -O0 -g -fno-stack-protector -no-pie \
    -o SSEx_vul_linux_exploit SSEx_vul_linux_exploit.c

python3 gen_payload.py
./SSEx_vul_linux_exploit test
```

预期输出：

```text
Linux Buffer Overflow Demonstration
===================================
Usage: SSEx_vul_linux_exploit ID

user: test
Processing: User=test, Data=AAAA...
Admin privileges activated!
```
