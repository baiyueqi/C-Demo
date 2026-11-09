#!/usr/bin/env bash
# 定期检查指定用户是否登录，登录后通过终端、wall 和 logger 提示。
# 开启严格模式
set -euo pipefail

# 必须至少提供一个用户名，否则提示用法并退出
if [[ $# -lt 1 ]]; then
  echo "用法：$0 <用户名> [检查间隔秒数]" >&2
  exit 2
fi

# 第一个参数是要监控的用户名，第二个参数是检查间隔，默认 30 秒
user=$1
interval=${2:-30}

# 检查间隔必须是非负整数
if [[ ! "$interval" =~ ^[0-9]+$ ]]; then
  echo "错误：检查间隔必须是非负整数。" >&2
  exit 2
fi

echo "开始监控用户 $user，每 $interval 秒检查一次。"

# 持续检查，直到目标用户登录
while true; do
  # 从 who 输出中提取第一列用户名，并精确匹配目标用户
  if who | awk '{print $1}' | grep -qx "$user"; then
    msg="用户 $user 已登录系统（$(date '+%F %T')）"
    # 先输出到当前终端
    echo "$msg"

    # 如果系统存在 wall 命令，则向所有终端广播提示
    if command -v wall >/dev/null 2>&1; then
      echo "$msg" | wall 2>/dev/null || true
    fi

    # 如果系统存在 logger 命令，则把提示写入系统日志
    if command -v logger >/dev/null 2>&1; then
      logger -t wait_user_login "$msg" 2>/dev/null || true
    fi

    # 发现用户登录后退出循环
    break
  fi

  # 等待指定时间后继续下一次检查
  sleep "$interval"
done
