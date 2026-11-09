#!/usr/bin/env bash
# 统计 /home 下各用户目录磁盘占用前 N 名，并发送警告或打印提示。
# 开启严格模式
set -euo pipefail

# 允许用第一个参数指定统计前几名，也可通过环境变量调整扫描目录和邮件命令
top_n=${1:-10}
home_root=${HOME_ROOT:-/home}
mail_cmd=${MAIL_CMD:-mail}

# 校验统计数量是否为正整数
if [[ ! "$top_n" =~ ^[0-9]+$ ]]; then
  echo "错误：用户数量必须是正整数。" >&2
  exit 2
fi

# 校验扫描目录是否存在
if [[ ! -d "$home_root" ]]; then
  echo "错误：目录不存在：$home_root" >&2
  exit 1
fi

# 统计每个一级用户目录的磁盘占用，并输出“大小(KB) 用户名”
report=$(
  # 遍历扫描目录下的一级子目录
  for user_dir in "$home_root"/*/; do
    # 跳过不是目录的项
    [[ -d "$user_dir" ]] || continue
    # 提取目录名作为用户名
    user=$(basename "$user_dir")
    # 使用 du 统计该用户目录占用的磁盘空间，单位是 KB
    size_kb=$(du -sk "$user_dir" 2>/dev/null | awk '{print $1}')
    # 统计失败时按 0 KB 处理
    printf '%s %s\n' "${size_kb:-0}" "$user"
  done | sort -nr | head -n "$top_n" || true
)

# 如果没有可统计的用户目录，则直接结束
if [[ -z "$report" ]]; then
  echo "没有可统计的用户目录。"
  exit 0
fi

echo "磁盘占用最大的前 $top_n 个用户："
# 将结果整理成“用户名 大小 KB”的可读表格
echo "$report" | awk '{printf "%-16s %s KB\n", $2, $1}'

# 逐行处理前 N 名用户，准备并发送磁盘空间警告
echo "$report" | while read -r size_kb user; do
  body="警告：用户 $user 的家目录当前占用磁盘空间约 $size_kb KB，请及时清理。"

  # 若系统存在指定邮件命令则发送邮件，否则仅在终端提示
  if command -v "$mail_cmd" >/dev/null 2>&1; then
    printf '%s\n' "$body" | "$mail_cmd" -s "磁盘空间警告" "$user" || true
  else
    echo "mail 命令不可用，未发送邮件给 $user。提示内容：$body"
  fi
done
