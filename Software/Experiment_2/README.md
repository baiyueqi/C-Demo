# 实验二：静态代码分析与安全缺陷检测（本地实验说明）

## 1. 结论：能否在本地完成

可以，但需要分两部分看：

- C 语言部分：当前环境已提供 `clang 16.0.6` 和 `clang-tidy 16.0.6`，可直接完成缺陷代码编写、静态扫描、问题定位、修复和复扫。
- Java 语言部分：当前环境没有安装 JDK、`javac` 或 IntelliJ IDEA，不能直接运行 Java 静态扫描；已提供可导入的 `Vulnerable.java` 和 `Fixed.java`，安装 JDK/IDEA 后即可按本说明完成。

本实验的本地替代方案：

| 原实验工具 | 本地替代工具 | 说明 |
| --- | --- | --- |
| Visual Studio 2013 自带静态分析 | `clang -Wall -Wextra -Wpedantic` + `clang --analyze` | 前者用于编译期告警，后者用于 Clang Static Analyzer |
| Cppcheck 2.20 | `clang-tidy` | 可检测内存泄漏、空指针、数组越界、未初始化、UAF 等 |
| IntelliJ IDEA 静态分析插件 | IDEA Community + JDK 17/21，或命令行 `javac -Xlint:all` | 安装后使用 Inspect Code |

当前已安装：

```text
clang version 16.0.6
clang-tidy (LLVM 16.0.6)
```

当前未安装，需要自行补充：

```text
cppcheck
java / javac
IntelliJ IDEA
```

## 2. 目录结构

```text
Software/Experiment_2/
├── README.md
├── 实验报告.md
├── c/
│   ├── Makefile
│   ├── compile_commands.json
│   ├── vulnerable.c
│   └── fixed.c
└── java/
    ├── Vulnerable.java
    ├── Fixed.java
    └── input.txt
```

## 3. C 语言实验步骤

### 3.1 查看缺陷代码

打开 [c/vulnerable.c](c/vulnerable.c)，其中集中演示了 6 类典型 C 缺陷：

1. 缓冲区溢出：`strcpy` 写入超长字符串。
2. 空指针引用：对 `NULL` 指针解引用。
3. 内存泄漏：`malloc` 后未 `free`。
4. 数组越界：访问 `arr[5]`。
5. 未初始化变量：读取未赋值的局部变量。
6. 释放后使用：`free` 后继续访问原指针。

### 3.2 编译并查看编译期告警

```bash
cd Software/Experiment_2/c
make all
```

编译产物输出到 `/tmp/exp2`，不会污染仓库。`vulnerable.c` 会显示 3 个典型编译告警：

```text
vulnerable.c:9: warning: destination buffer will always be overflown by source
vulnerable.c:33: warning: array index 5 is past the end of the array
vulnerable.c:40: warning: variable 'value' is uninitialized when used here
```

`fixed.c` 使用相同编译选项应无告警。

### 3.3 运行 Clang Static Analyzer

```bash
make analyze
```

`vulnerable.c` 会被识别出 5 条分析告警，包括：

```text
vulnerable.c:17: warning: Dereference of null pointer [core.NullDereference]
vulnerable.c:27: warning: Potential leak of memory pointed to by 'p' [unix.Malloc]
vulnerable.c:33: warning: 2nd function call argument is an uninitialized value [core.CallAndMessage]
vulnerable.c:40: warning: The left operand of '>' is a garbage value [core.UndefinedBinaryOperatorResult]
vulnerable.c:52: warning: Use of memory after it is freed [unix.Malloc]
```

`fixed.c` 不应出现以上告警。

### 3.4 运行 clang-tidy

```bash
make tidy
```

`vulnerable.c` 会显示 11 条告警，覆盖 6 类缺陷；`fixed.c` 应为 0 条告警。这里在 Makefile 中排除了 `clang-analyzer-security.insecureAPI.DeprecatedOrUnsafeBufferHandling`，原因是修复代码使用标准安全的 `snprintf`，但该检查会把所有传统缓冲区函数都标记为“非 C11 Annex K”，并非真正缓冲区溢出。

### 3.5 运行修复后的程序

```bash
make run-fixed
```

预期输出：

```text
buffer_overflow_fixed: This st
null_deref_fixed: 42
memory_leak_fixed: released memory
array_oob_fixed[0] = 1
array_oob_fixed[1] = 2
array_oob_fixed[2] = 3
array_oob_fixed[3] = 4
array_oob_fixed[4] = 5
uninitialized_fixed: non-positive
use_after_free_fixed: hello
```

`snprintf` 截断后只输出 `This st`，这正是“有边界写入”的正常结果。

## 4. Java 语言实验步骤

### 4.1 安装环境

选择一种方式安装 JDK 和 IDEA：

- NixOS/Nix：在开发环境或 `flake.nix` 中加入 `jdk` 和 `idea-community`。
- Debian/Ubuntu：`sudo apt install openjdk-17-jdk`，再安装 IntelliJ IDEA Community。
- 已有 IDEA 的 Windows/macOS 环境：直接导入本目录。

安装 JDK 后，命令行验证：

```bash
java -version
javac -version
```

### 4.2 查看缺陷代码

打开 [java/Vulnerable.java](java/Vulnerable.java)，其中包含：

1. SQL 注入：字符串拼接构造 SQL。
2. 硬编码敏感信息：`DB_PASSWORD = "P@ssw0rd"`。
3. 空指针异常：对 `null` 调用 `length()`。
4. 资源泄漏：`BufferedReader` 未关闭。

修复版见 [java/Fixed.java](java/Fixed.java)。

### 4.3 IDEA 静态扫描

1. 在 IDEA 中选择 `File -> Open`，打开 `Software/Experiment_2/java`。
2. 设置 Project SDK 为已安装的 JDK 17 或 21。
3. 在项目目录右键，选择 `Analyze -> Inspect Code`。
4. 等待扫描完成，在 Inspection Results 中查看告警。
5. 对 `Vulnerable.java` 截图记录缺陷告警。
6. 用 `Fixed.java` 替换缺陷版本后再次扫描，确认告警消失。

### 4.4 命令行编译与运行

```bash
cd Software/Experiment_2/java
javac -Xlint:all Vulnerable.java Fixed.java
java Vulnerable
java Fixed
```

注意：`Vulnerable.java` 只有传入 `run` 参数才会触发 `nullPointer()` 和 `resourceLeak()` 的运行路径；`sqlInjection()` 需要真实 `Connection` 参数，因此扫描时重点由 IDE 静态分析和人工审计完成。

## 5. 缺陷定位与修复速查

C 语言：

| 缺陷 | 位置 | 工具告警 | 修复方法 |
| --- | --- | --- | --- |
| 缓冲区溢出 | `vulnerable.c:9` | `strcpy` insecure / `fortify-source` | 使用 `snprintf` + `sizeof` 限长 |
| 空指针引用 | `vulnerable.c:17` | `core.NullDereference` | 解引用前判空 |
| 内存泄漏 | `vulnerable.c:27` | `unix.Malloc` | 使用完毕 `free` |
| 数组越界 | `vulnerable.c:33` | `array-bounds` / `core.CallAndMessage` | 循环下标 `< 数组长度` |
| 未初始化变量 | `vulnerable.c:40` | `uninitialized` / garbage value | 声明时初始化 |
| 释放后使用 | `vulnerable.c:52` | `unix.Malloc` | 释放前完成访问，释放后置 `NULL` |

Java 语言：

| 缺陷 | 位置 | 修复方法 |
| --- | --- | --- |
| SQL 注入 | `Vulnerable.java` 的 `sqlInjection()` | 使用 `PreparedStatement` 参数绑定 |
| 硬编码敏感信息 | `Vulnerable.java` 的 `DB_PASSWORD` | 从环境变量或配置中心读取 |
| 空指针异常 | `Vulnerable.java` 的 `nullPointer()` | 使用前判空或 `Optional` |
| 资源泄漏 | `Vulnerable.java` 的 `resourceLeak()` | 使用 try-with-resources |

## 6. 建议的安全编码原则

1. 所有变量先初始化再使用。
2. 所有指针和对象先判空再访问。
3. 所有数组、缓冲区写入前先做边界检查。
4. 所有动态分配/打开的资源必须保证释放或关闭。
5. 禁止使用 `strcpy`、`gets`、`sprintf` 等无界输入函数。
6. Java 数据库访问必须参数化，禁止字符串拼接 SQL。
7. 敏感信息不得硬编码在源码中。

完整实验报告、缺陷对比表和思考题见 [实验报告.md](实验报告.md)。
