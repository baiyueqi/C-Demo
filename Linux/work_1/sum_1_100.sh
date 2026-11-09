#!/usr/bin/env bash
# 计算 1 到 100 的和并输出结果。
# 开启严格模式：命令失败、引用未定义变量或管道失败时立即退出
set -euo pipefail

# 用于保存累加结果的变量
sum=0
# 使用算术 for 循环累加 1 到 100
for ((i = 1; i <= 100; i++)); do
  ((sum += i))
done

# 输出最终求和结果
printf '%d\n' "$sum"
