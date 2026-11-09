#!/usr/bin/env bash
# 根据输入的文件名，从当前目录或 SEARCH_ROOT 指定目录查找文件。
# 开启严格模式
set -euo pipefail

# 从标准输入读取要查找的文件名
read -r -p "请输入要查找的文件名：" filename

# 文件名为空时提示错误
if [[ -z "$filename" ]]; then
  echo "错误：文件名不能为空。" >&2
  exit 1
fi

# 从当前目录或 SEARCH_ROOT 指定的目录开始递归查找文件
results=$(find "${SEARCH_ROOT:-.}" -type f -name "$filename" 2>/dev/null || true)

# 未找到匹配文件时提示并退出
if [[ -z "$results" ]]; then
  echo "未找到文件：$filename"
  exit 1
fi

echo "找到的路径："
# 逐行输出所有匹配路径
printf '%s\n' "$results"
