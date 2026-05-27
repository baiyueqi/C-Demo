# AI 驱动的网络流量分类器 5 分钟 PPT 设计方案

## 汇报定位

- 汇报形式：使用 HTML 页面讲解，不现场运行命令，只讲截图证据。
- 汇报时长：5 分钟。
- 页面数量：7 页。
- 记忆主线：抓包 -> 解析 -> 五元组 -> 特征 -> KNN/规则分类 -> 攻击报警。

## 与评委打分相关的要求

- 功能实现与演示效果 40 分：题目二需要分类器能对流量给出分类结果，有主要功能截图，演示不卡顿。
- 计算机网络原理理解 30 分：能说明 TCP/UDP 特性、IP 头部和 TTL、路由转发、MAC/ARP、五层封装/解封装。
- AI 集成 15 分：特征提取合理、模型训练过程完整，能解释分类结果和误差来源。
- 表达与回答问题 15 分：PPT 结构合理，5 分钟内讲清楚，能回答设计决策和基础概念问题。

## 页面设计

| 页码 | 标题 | 时间 | 讲解重点 | 截图 |
| --- | --- | ---: | --- | --- |
| 1 | 项目目标与评分对应 | 35 秒 | 用一句话说明项目链路，并对齐 40/30/15/15 评分项 | 无 |
| 2 | 系统架构 | 45 秒 | 从抓包到分类结果的流水线；模块分工 | `terminal-web-analysis.png`、`project-modules.png` |
| 3 | 基础功能 | 45 秒 | pcap 读取、五元组特征、CSV 输出、100 包样本 | `csv-features.png`、`live-100-predict.png` |
| 4 | 五层原理 | 60 秒 | HTTP/HTTPS 封装与解封装；Socket API 与协议栈关系 | `wireshark-five-layers.png`、`wireshark-web-filter.png` |
| 5 | AI/KNN 分类 | 50 秒 | KNN 训练、保存、加载、预测 | `knn-train.png`、`knn-predict.png` |
| 6 | 拓展与验证 | 55 秒 | SYN Flood、TCP Probe、Port Scan；测试和权限说明 | `alert-port-scan.png`、`tests-passed.png`、`live-capture-permission.png` |
| 7 | 总结 | 50 秒 | 按评分项收束，准备三个答辩问题 | 无 |

## 每页内容

### 第 1 页：项目目标与评分对应

页面内容：

- 项目名称：网络流量分类器。
- 一句话目标：读取 pcap/pcapng 或实时网卡数据，解析协议头部，按五元组聚合网络流，提取统计特征，用规则和 KNN 判断类别，并对攻击流量报警。
- 四个评分块：基础功能、网络原理、AI 集成、拓展功能。

讲解关键词：

- 题目二。
- 抓包、解析、五元组、特征、分类、报警。
- 对齐评分项。

### 第 2 页：系统架构

页面内容：

```text
Wireshark / 实时网卡
  -> pcapng / RawPacket
  -> 协议解析
  -> 五元组聚合
  -> 特征提取
  -> 规则 / KNN / ALERT
```

截图讲解：

- `terminal-web-analysis.png`：Web 样本可完成解析、聚合和分类。
- `project-modules.png`：读取、解析、聚合、特征、分类、KNN 分模块实现。

讲解关键词：

- 离线和实时入口复用同一套分析流程。
- 双向五元组避免请求和响应被拆开。

### 第 3 页：基础功能

页面内容：

- 支持 `.pcap` 和 `.pcapng`。
- 支持 Ethernet、IPv4、TCP、UDP、ICMP。
- 提取包数、字节数、平均包长、间隔方差、TTL、TCP flags。
- 样本结果：DNS 526 包 245 流；ICMP 8 包 1 流；Web 42749 包 96 流；100 包样本 105 包 19 流。

截图讲解：

- `csv-features.png`：一条流对应一行特征和分类结果。
- `live-100-predict.png`：100 包样本输出类别和置信度。

讲解关键词：

- 核心功能正确实现。
- CSV 可作为训练数据。

### 第 4 页：五层原理

页面内容：

- 应用层：HTTP 请求；HTTPS 经过 TLS 加密。
- 传输层：TCP 面向连接、可靠、有序；UDP 无连接、开销小；程序解析端口和 TCP flags。
- 网络层：源/目的 IP、TTL、协议号；路由器按目的 IP 转发，TTL 每跳递减。
- 链路层：源/目的 MAC、以太网类型；ARP 用 IP 查询下一跳 MAC，MAC 不跨路由直达。
- Socket 关系：Socket 是应用使用协议栈的接口；实时抓包用 `AF_PACKET` 从链路层取帧。

截图讲解：

- `wireshark-five-layers.png`：Frame、Ethernet、IPv4、TCP 分层字段。
- `wireshark-web-filter.png`：80/443 端口过滤 Web 流量。

讲解关键词：

- 发送时封装，抓包时解封装。
- HTTPS 内容不可见，但头部和统计行为可见。

### 第 5 页：AI/KNN 分类

页面内容：

- CSV 带标签流作为训练数据。
- 协议 one-hot，数值特征归一化。
- KNN 距离加权投票。
- 当前模型：342 条真实流，DNS/ICMP/Web 三类，`k=3`，保存为 `models/knn_model.json`。

截图讲解：

- `knn-train.png`：训练样本数、类别、k 值。
- `knn-predict.png`：加载模型后输出类别和置信度。

讲解关键词：

- 不是只写规则，也不是黑盒 API。
- 抓包特征真正进入了模型。
- 误差来源：训练类别少，未知流量会归入最接近的已知类别。

### 第 6 页：拓展与验证

页面内容：

- SYN Flood：大量 SYN，ACK 少。
- TCP Probe：短 TCP 探测流。
- Port Scan：同一源目标访问多个端口。
- 实时抓包入口已实现，但当前环境需要 root 或 `CAP_NET_RAW`。

截图讲解：

- `alert-port-scan.png`：攻击报警输出。
- `tests-passed.png`：规则分类、KNN、攻击检测通过验证。
- `live-capture-permission.png`：实时抓包权限限制说明。

讲解关键词：

- 完成选做加分项。
- 主动说明误报和权限限制。

### 第 7 页：总结

页面内容：

- 一句话总结：真实网络包 -> 五元组流 -> 统计特征 -> 类别、置信度、攻击报警。
- 评分证据表：基础功能、网络原理、AI 集成、拓展功能。
- 答辩三问：
  - 为什么用五元组？
  - 为什么 HTTPS 能分类？
  - KNN 的局限是什么？

讲解关键词：

- 主流程完成。
- 证据来自截图。
- 局限和后续优化明确。
