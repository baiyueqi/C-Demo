# C-Demo

一个 C++ 学习和演示仓库，包含计算机科学基础、安全和数据结构等多个项目。

## 项目概述

本仓库作为 C++ 基础设施的基础，使用 Nix flakes 提供可重现的开发环境。包含多个涵盖计算机科学教育不同方面的子项目。

## 项目结构

```
C-Demo/
├── CourseDesign/          # 课程设计项目
├── CyberspaceSecurity/    # 密码学和安全实现
│   ├── Enigma.cpp        # 恩尼格玛机模拟
│   ├── RSA.cpp           # RSA 加密实现
│   └── ...
├── DataStructures/        # 数据结构实现
├── foundation/            # 基础项目
│   ├── gobang.cpp        # 五子棋游戏
│   └── dbg.h             # 调试工具
├── InSecurityMath/        # 信息安全数学
├── notebook/              # Jupyter 笔记本
├── src/                   # 其他源文件
└── flake.nix             # Nix flake 配置
```

## 开发环境

本项目使用 Nix flakes 提供可重现的开发环境，包含以下工具和库：

### 构建工具
- CMake
- Clang/LLVM 16
- Python 3 及 JupyterLab

### C++ 库
- Boost（带 Python 绑定）
- spdlog（日志库）
- nlohmann_json（JSON 解析）

### 开发工具
- claude-code（AI 驱动的编码助手）
- gh（GitHub 命令行工具）
- JupyterLab 及 ipywidgets、notebook 支持

## 快速开始

### 前置要求

- 启用 flakes 的 Nix

### 设置步骤

1. 克隆仓库：
```bash
git clone <仓库地址>
cd C-Demo
```

2. 进入开发环境：
```bash
nix develop
```

这将自动设置 `flake.nix` 中指定的所有依赖和工具。

### 构建项目

可以使用 CMake 或直接使用 clang 构建项目：

```bash
# 示例：编译 C++ 文件
clang++ -std=c++17 -o output source.cpp

# 或使用 CMake 构建大型项目
cmake -B build
cmake --build build
```

## 涵盖主题

- **密码学**：RSA 加密、恩尼格玛机模拟
- **数据结构**：各种数据结构实现
- **游戏开发**：五子棋游戏
- **信息安全数学**：安全的数学基础
- **系统编程**：底层 C/C++ 编程概念

## 支持平台

- x86_64-linux
- i686-linux
- aarch64-linux
- x86_64-darwin

## 许可证

本项目仅用于教育目的。

## 贡献

这是一个个人学习仓库。欢迎 fork 并用于自己的学习目的。
