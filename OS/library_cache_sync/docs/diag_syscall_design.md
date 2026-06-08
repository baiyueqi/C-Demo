# 自定义 Linux 系统调用实现记录

本文件用于题目二验收说明。仓库内只保存用户态包装和设计记录，不提交 Linux 内核源码、编译产物或截图，避免资料过大。

## 1. 目标

把题目一中的四类模拟耗时函数替换为 Linux 系统调用：

```text
delay_read    -> sys_diag_read(number)
delay_load    -> sys_diag_load(number)
delay_write   -> sys_diag_write(number)
delay_refresh -> sys_diag_refresh(number)
```

每个系统调用接收学号 `number`，以 `number + offset` 作为随机种子，生成 1000 到 5000 毫秒的等待时间，并让当前进程睡眠。

当前学号 `20241001655` 超过 32 位 `int` 范围，因此用户态代码使用 `long long student_no`，内核侧系统调用参数建议使用 `long number`。如果内核仍使用 `int number`，日志中的学号会被截断。

当前 WSL2 实验记录：

```text
内核源码目录：~/kernel-lab/WSL2-Linux-Kernel
内核版本后缀：6.18.26.3-microsoft-standard-WSL2-diag+
diag_read    = 548
diag_load    = 549
diag_write   = 550
diag_refresh = 551
```

## 2. 内核侧步骤

以下路径以常见 x86_64 Linux 内核为例，实际课程环境可能随内核版本调整。

1. 在内核源码中增加四个 `SYSCALL_DEFINE1` 入口，例如放在 `kernel/sys.c` 或单独新增文件。
2. 在 `include/linux/syscalls.h` 中声明四个系统调用原型。
3. 在 `arch/x86/entry/syscalls/syscall_64.tbl` 中注册系统调用号和函数名。
4. 重新编译并安装内核，启动到新内核。
5. 在用户态使用 `syscall(__NR_diag_read, number)` 等方式调用。

## 3. 内核实现示例

```c
static long diag_delay_impl(long number, int op_offset)
{
    unsigned int seed = (unsigned int)((unsigned long)number + (unsigned long)op_offset);
    unsigned int delay_ms = 1000 + (seed % 4001);

    msleep(delay_ms);
    return 0;
}

SYSCALL_DEFINE1(diag_read, long, number)
{
    return diag_delay_impl(number, 1);
}

SYSCALL_DEFINE1(diag_load, long, number)
{
    return diag_delay_impl(number, 2);
}

SYSCALL_DEFINE1(diag_write, long, number)
{
    return diag_delay_impl(number, 3);
}

SYSCALL_DEFINE1(diag_refresh, long, number)
{
    return diag_delay_impl(number, 4);
}
```

## 4. 用户态编译方式

题目一程序默认使用用户态延时，便于普通环境编译运行。完成内核改造并启动到带 `diag` 后缀的新内核后，可用 548 到 551 四个调用号编译：

```bash
make -C OS/library_cache_sync build-diag
```

如果系统调用号不是 548 到 551，可覆盖 Makefile 变量：

```bash
make -C OS/library_cache_sync build-diag \
  DIAG_NR_READ=<nr1> DIAG_NR_LOAD=<nr2> \
  DIAG_NR_WRITE=<nr3> DIAG_NR_REFRESH=<nr4>
```

单独验证四个系统调用：

```bash
make -C OS/library_cache_sync run-diag-demo
```

预期每一行的耗时约为 1000 到 5000 毫秒：

```text
diag_read(20241001655) returned 0, elapsed_ms=...
diag_load(20241001655) returned 0, elapsed_ms=...
diag_write(20241001655) returned 0, elapsed_ms=...
diag_refresh(20241001655) returned 0, elapsed_ms=...
```

再运行主程序集成验证：

```bash
make -C OS/library_cache_sync run-diag-smoke
```

最后查看内核日志：

```bash
dmesg | grep diag_
```

## 5. 报告截图清单

1. 系统调用实现函数。
2. 系统调用声明。
3. 系统调用表注册。
4. 内核编译成功。
5. `user_call_demo` 调用输出。
6. 主程序切换到系统调用延时后的运行日志。

## 6. 已完成验证记录

以下记录来自 WSL2 Ubuntu 终端，可作为后续复现、截图和报告分析依据。

### 6.1 新内核确认

命令：

```bash
uname -r
```

输出：

```text
6.18.26.3-microsoft-standard-WSL2-diag+
```

说明：WSL2 已经加载自定义内核，内核版本后缀包含 `diag+`。

### 6.2 四个系统调用单独验证

命令：

```bash
make -C OS/library_cache_sync run-diag-demo
```

关键编译参数：

```text
-DUSE_DIAG_SYSCALL
-D__NR_diag_read=548
-D__NR_diag_load=549
-D__NR_diag_write=550
-D__NR_diag_refresh=551
```

关键输出：

```text
diag_read(20241001655) returned 0, elapsed_ms=...
diag_load(20241001655) returned 0, elapsed_ms=...
diag_write(20241001655) returned 0, elapsed_ms=...
diag_refresh(20241001655) returned 0, elapsed_ms=...
```

说明：四个系统调用都返回 `0`，耗时均在 1000 到 5000 毫秒范围内，符合题目要求。

### 6.3 主程序集成 smoke 验证

命令：

```bash
make -C OS/library_cache_sync run-diag-smoke
```

关键输出：

```text
[Reader-01] cache miss book=Algorithm enqueue request_id=1
[CacheManager] load begin book=Algorithm reader=1 request_id=1
[CacheManager] loaded into free slot book=Algorithm version=1 cache=[Algorithm:v1:pin1]
[Reader-01] read begin book=Algorithm version=1
[Reader-01] read done book=Algorithm version=1
[Writer-01] update database begin book=Linux
[Writer-01] update database done book=Linux version=2
[Writer-01] cache has no copy book=Linux, skip refresh
[Main] finished
```

说明：该用例验证主程序已通过系统调用路径完成缓存加载、读者读取和写者写入。由于读者读取 `Algorithm`，写者更新 `Linux`，缓存中没有 `Linux` 副本，因此本轮不会触发刷新，这是正常结果。

### 6.4 主程序触发缓存刷新验证

命令：

```bash
OS/library_cache_sync/build/library_cache_sync \
  --readers 1 \
  --writers 1 \
  --cache-size 1 \
  --rounds 1 \
  --student-no 20241001655 \
  --seed 2
```

关键输出：

```text
[Reader-01] round=1 request book=OS
[Reader-01] cache miss book=OS enqueue request_id=1
[CacheManager] load begin book=OS reader=1 request_id=1
[CacheManager] loaded into free slot book=OS version=1 cache=[OS:v1:pin1]
[Reader-01] read begin book=OS version=1
[Reader-01] read done book=OS version=1
[Writer-01] update database begin book=OS
[Writer-01] update database done book=OS version=2
[Writer-01] refresh cache begin book=OS version=2 cache=[OS:v2]
[Writer-01] refresh cache done book=OS version=2
[Main] finished
```

时间分析：

```text
load:    000000ms -> 001121ms，约 1.1 秒
read:    001121ms -> 004737ms，约 3.6 秒
write:   004737ms -> 006497ms，约 1.8 秒
refresh: 006497ms -> 008385ms，约 1.9 秒
```

说明：该用例让读者和写者都选择 `OS`，因此读者先把 `OS` 加载到缓存，写者随后更新 `OS` 时能够触发缓存刷新。主程序完整覆盖 `diag_load`、`diag_read`、`diag_write`、`diag_refresh` 四条路径。

### 6.5 内核日志验证

命令：

```bash
dmesg | grep diag_ | tail -n 30
```

关键输出：

```text
diag_read(number=20241001655) sleep ... ms
diag_load(number=20241001655) sleep ... ms
diag_write(number=20241001655) sleep ... ms
diag_refresh(number=20241001655) sleep ... ms
```

说明：`dmesg` 中出现 `diag_read`、`diag_load`、`diag_write`、`diag_refresh`，证明用户态程序确实进入了新增的 Linux 系统调用，而不是普通用户态延时函数。

### 6.6 验收结论

第二题要求已经满足：

1. 已启动带 `diag+` 后缀的 WSL2 自定义内核。
2. 已注册并调用 548 到 551 四个系统调用。
3. 四个系统调用均能返回成功，延时范围符合 1000 到 5000 毫秒要求。
4. 题目一主程序已通过 `USE_DIAG_SYSCALL` 切换到系统调用延时路径。
5. 主程序运行日志和 `dmesg` 内核日志能够相互印证。
