#!/usr/bin/env bash
# 管理指定目录，统计一级文件、最大文件并合并 .log 文件。
# 开启严格模式
set -euo pipefail

# 必须且只能提供一个非空目录路径
if [[ $# -ne 1 || -z "$1" ]]; then
  echo "用法：$0 <目录路径>" >&2
  exit 2
fi

dir=$1

# 如果同名路径已经存在但不是目录，则报错退出
if [[ -e "$dir" && ! -d "$dir" ]]; then
  echo "错误：$dir 已存在但不是目录。" >&2
  exit 1
fi

# 目录不存在时创建它
if [[ ! -d "$dir" ]]; then
  mkdir -p "$dir"
  echo "目录不存在，已创建：$dir"
fi

# 统计该目录下的一级文件数量，不递归统计子目录，并排除已生成的 merged.log
file_count=$(find "$dir" -maxdepth 1 -type f ! -name 'merged.log' | wc -l || true)
echo "目录：$dir"
echo "文件数量（不含子目录）：$file_count"

echo "最大的 3 个文件："
# 查找一级目录中最大的 3 个文件，并排除已生成的 merged.log，输出“字节数\t路径”
largest=$(find "$dir" -maxdepth 1 -type f ! -name 'merged.log' -printf '%s\t%p\n' 2>/dev/null | sort -nr | head -n 3 || true)

if [[ -z "$largest" ]]; then
  echo "（没有文件）"
else
  # 逐行读取字节数和路径，并转换为人类可读的大小
  echo "$largest" | while IFS=$'\t' read -r size path; do
    human=$(du -h "$path" 2>/dev/null | awk '{print $1}')
    printf '%s\t%s\n' "${human:-${size}B}" "$path"
  done
fi

# 定义合并日志文件路径，并先清空该文件
merged="$dir/merged.log"
: > "$merged"

# 按文件名顺序逐个追加当前目录下的 .log 文件，并在每段前加上来源标记
while IFS= read -r -d '' logfile; do
  {
    printf '# ===== 来源文件: %s =====\n' "$(basename "$logfile")"
    cat "$logfile"
    printf '\n'
  } >> "$merged"
done < <(find "$dir" -maxdepth 1 -type f -name '*.log' ! -name 'merged.log' -print0 | sort -z)

# 设置合并日志文件权限为 644
chmod 644 "$merged"
echo "已合并 .log 文件到：$merged（权限 644）"
