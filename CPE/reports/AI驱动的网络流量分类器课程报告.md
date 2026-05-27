# AI 驱动的网络流量分类器课程报告

## 第 1 章 引言

### 1.1 选题目标

本课程设计选择“AI 驱动的网络流量分类器”作为实现题目，目标是设计并实现一个能够读取网络抓包文件、解析网络协议字段、按五元组聚合网络流、提取流量统计特征，并对流量类别进行自动判断的实验系统。

系统围绕计算机网络五层体系结构展开，通过 Wireshark 抓取真实 Wi-Fi 网络流量，再由程序解析 Ethernet、IPv4、TCP、UDP、ICMP 等协议字段，提取平均包长、到达间隔方差、传输层协议、端口号、TTL 值等特征，最终输出 Web、DNS、ICMP 等流量类型及置信度。课程要求中“使用 scapy 或 pcap4J 捕获实时数据包或读取 pcap 文件”允许二选一，本项目选择读取 pcap/pcapng 文件的离线分析方式实现核心流程。

分类模块方面，课程建议训练决策树或 KNN 等简单机器学习模型。当前项目已基于真实流量 CSV 数据实现了一个标准库 KNN 分类器，支持训练、保存 JSON 模型、加载模型并预测新抓包文件中的流量类别和置信度。在扩展功能方面，本系统实现了常见攻击流量检测与报警，包括 SYN Flood、TCP Probe 和 Port Scan，并通过合成样本模拟了端口扫描误报场景，用于分析规则检测方法的局限性。

### 1.2 设计内容

本项目主要完成以下内容：

1. 使用 Wireshark 采集真实 Wi-Fi 网络流量，并保存为 pcapng 文件。
2. 使用 Python 实现 pcap/pcapng 文件读取模块。
3. 解析 Ethernet、IPv4、TCP、UDP、ICMP 协议字段。
4. 按双向五元组聚合网络流。
5. 提取网络流统计特征。
6. 使用规则分类方法识别 DNS、Web、ICMP 等流量类别，并输出置信度。
7. 使用带标签 CSV 训练 KNN 模型，保存模型并在预测阶段加载。
8. 检测 SYN Flood、TCP Probe、Port Scan 等攻击流量，并输出 ALERT 报警。
9. 采集一小批真实校园网/Wi-Fi 流量并进行类别标注。
10. 对正常样本和误报样本进行测试与分析。

课程核心要求与当前完成情况如下：

| 课程要求 | 当前完成情况 |
| --- | --- |
| 抓包模块：实时抓包或读取 pcap 文件 | 已完成 pcap/pcapng 读取；已实现 Linux 网卡实时抓包入口 |
| 特征提取：按五元组提取平均包长、间隔方差、协议、端口、TTL 等 | 已完成 |
| 分类模块：训练简单模型或调用 API | 已完成标准库 KNN 训练、保存、加载和预测 |
| 实时显示：选择网卡，抓 100 个包后输出类别和置信度 | 已实现命令行选择/指定网卡和抓包入口；实际运行需要 root 或 CAP_NET_RAW 权限 |
| 报告：HTTP 请求五层封装/解封装分析 | 已在第 3 章说明 |
| 拓展：攻击检测并报警 | 已完成 SYN Flood、TCP Probe、Port Scan 报警 |
| 拓展：真实校园网流量采集并标注 | 已采集 DNS、ICMP、Web 三类真实 Wi-Fi 流量并生成 CSV |
| 拓展：实时图形化界面 | 未完成 |

### 1.3 开发环境与工具

| 项目 | 内容 |
| --- | --- |
| 操作系统 | Linux / Nix 开发环境 |
| 开发语言 | Python 3 |
| 抓包工具 | Wireshark |
| 输入文件格式 | pcap / pcapng |
| 输出文件格式 | CSV |
| 项目目录 | `CPE/traffic_classifier` |
| 主要模块 | pcap 读取、协议解析、流聚合、特征提取、分类与报警 |

项目运行示例：

```bash
cd CPE/traffic_classifier
python -m traffic_classifier samples/web.pcapng --csv output/web_features.csv
```

## 第 2 章 系统设计

### 2.1 总体架构

本系统不是传统的客户端-服务器聊天程序，而是一个网络流量分析程序。课程题目允许“实时捕获数据包”或“读取 pcap 文件”，本项目同时支持两种入口：一是读取 Wireshark 保存的 pcap/pcapng 文件，二是在 Linux 下通过原始套接字选择网卡并抓取实时数据包。系统整体架构如下：

```text
Wireshark 抓包 / 实时网卡抓包
     |
     v
pcap / pcapng 文件 / RawPacket 列表
     |
     v
抓包文件读取模块
     |
     v
协议解析模块
Ethernet / IPv4 / TCP / UDP / ICMP
     |
     v
五元组流聚合模块
     |
     v
特征提取模块
packet_count / total_bytes / avg_packet_size / duration / interval_variance / ttl / tcp_flags
     |
     v
规则分类 / KNN 模型 / 攻击检测模块
DNS / Web / ICMP / SYN_Flood / TCP_Probe / Port_Scan
     |
     v
终端输出 + CSV 文件
```

实时抓包入口由 `traffic_classifier.live_capture` 提供。程序可以列出系统网卡，也可以通过 `-i` 参数指定网卡，默认抓取 100 个包后输出当前流量类别和置信度。由于 Linux 原始套接字需要权限，实际运行通常需要 root 或 `CAP_NET_RAW`。

系统数据流说明：

1. 用户通过 Wireshark 选择 Wi-Fi 网卡并采集网络数据包，或由程序直接选择网卡进行实时抓包。
2. 离线模式下抓包结果保存为 `.pcapng` 文件；实时模式下数据帧直接进入内存中的 `RawPacket` 列表。
3. 程序读取抓包文件，逐包解析链路层、网络层和传输层字段。
4. 根据源 IP、目的 IP、源端口、目的端口、协议组成五元组，并将双向通信聚合为同一条流。
5. 对每条流提取统计特征。
6. 分类模块可以使用规则分类，也可以加载 KNN 模型进行预测。
7. 若检测到攻击流量，则在终端输出中加入 `ALERT` 标识。
8. 所有特征和分类结果保存为 CSV 文件。

### 2.2 功能模块划分

#### 2.2.1 抓包文件读取模块

对应文件：

```text
traffic_classifier/pcap_reader.py
```

该模块负责读取 `.pcap` 和 `.pcapng` 文件，并提取每个原始数据帧的时间戳和字节内容。课程要求中可以使用 scapy 或 pcap4J 进行实时捕获，也可以读取 pcap 文件。当前实现选择读取 pcap/pcapng 文件，为了降低环境依赖，使用 Python 标准库完成解析，不依赖 Scapy。

主要功能：

- 判断输入文件是 pcap 还是 pcapng。
- 解析 pcap 文件头和数据包记录。
- 解析 pcapng Section Header、Interface Description、Enhanced Packet 等常见块。
- 输出统一的 `RawPacket` 对象。

抓包操作由 Wireshark 完成，具体步骤是选择 Wi-Fi 网卡、开始抓包、执行 DNS 查询或访问网页、停止抓包并保存为 pcapng 文件。

实时抓包模块对应文件：

```text
traffic_classifier/live_capture.py
```

该模块主要功能包括：

- 从 `/sys/class/net` 列出可用网卡。
- 通过命令行参数 `-i/--interface` 指定网卡。
- 使用 Linux `AF_PACKET` 原始套接字捕获以太网帧。
- 默认抓取 100 个包，也可通过 `--count` 指定数量。
- 抓包完成后复用协议解析、流聚合、特征提取和分类模块输出结果。

使用示例：

```bash
python -m traffic_classifier.live_capture --count 100
python -m traffic_classifier.live_capture -i eth0 --count 100 --model models/knn_model.json
```

需要注意，实时抓包通常需要 root 或 `CAP_NET_RAW` 权限。本实验环境中程序可以列出网卡 `eth0`，但实际抓包时系统返回 `Operation not permitted`，说明当前运行环境没有原始套接字权限；在普通 Linux 主机上使用 sudo 或配置权限后即可运行。

#### 2.2.2 协议解析模块

对应文件：

```text
traffic_classifier/parser.py
```

该模块负责从原始以太网帧中解析协议字段。

支持的协议包括：

- Ethernet II
- VLAN
- IPv4
- TCP
- UDP
- ICMP

解析出的关键字段包括：

```text
timestamp
src_ip
dst_ip
src_port
dst_port
protocol
length
ttl
tcp_flags
icmp_type
src_mac
dst_mac
```

#### 2.2.3 五元组流聚合模块

对应文件：

```text
traffic_classifier/flow.py
```

网络流使用五元组表示：

```text
源 IP、目的 IP、源端口、目的端口、传输层协议
```

为了避免同一连接的上下行方向被拆成两条流，本系统使用双向标准化五元组。例如：

```text
172.27.152.109:53209 <-> 202.114.200.251:53 UDP
```

无论数据包方向是客户端到服务器，还是服务器到客户端，都会被归入同一条流。

#### 2.2.4 特征提取模块

对应文件：

```text
traffic_classifier/features.py
```

每条流会被转换成一行特征数据，主要特征包括：

| 特征 | 含义 |
| --- | --- |
| packet_count | 流中的数据包数量 |
| total_bytes | 总字节数 |
| avg_packet_size | 平均包长 |
| max_packet_size | 最大包长 |
| min_packet_size | 最小包长 |
| duration | 流持续时间 |
| avg_interval | 平均到达间隔 |
| interval_variance | 到达间隔方差 |
| avg_ttl | 平均 TTL |
| syn_count | TCP SYN 数量 |
| ack_count | TCP ACK 数量 |
| fin_count | TCP FIN 数量 |
| rst_count | TCP RST 数量 |
| psh_count | TCP PSH 数量 |

这些特征能够反映流量在网络层和传输层的基本行为。

#### 2.2.5 分类与报警模块

对应文件：

```text
traffic_classifier/classifier.py
```

当前系统使用透明规则进行分类，便于课程展示和解释。

分类类别包括：

| 类别 | 判断依据 |
| --- | --- |
| DNS | TCP/UDP 端口 53 |
| Web | TCP 端口 80 或 443 |
| ICMP | IP 协议号为 ICMP |
| SSH | TCP 端口 22 |
| SYN_Flood | 大量 SYN 包且 ACK 较少 |
| TCP_Probe | 短 TCP 探测流 |
| Port_Scan | 同一源目标对出现多个目标端口探测 |
| Unknown | 无明显规则匹配 |

攻击流量类别会触发报警输出，例如：

```text
ALERT Port_Scan confidence=0.80
```

#### 2.2.6 CSV 输出模块

对应文件：

```text
traffic_classifier/csv_io.py
```

该模块将每条流的特征、分类标签、置信度和分类原因写入 CSV 文件，便于后续训练机器学习模型或放入报告中展示。

#### 2.2.7 真实流量标注模块

本项目采集了 DNS、ICMP、Web 三类真实 Wi-Fi 流量，并根据采集时的实验行为和端口规则进行标注：

| 样本 | 标注依据 |
| --- | --- |
| `dns.pcapng` | 抓包时执行 DNS 查询，主要流量为 UDP/TCP 53 |
| `icmp.pcapng` | 抓包时执行 ping，协议为 ICMP |
| `web.pcapng` | 抓包时访问网页，主要流量为 TCP 443 |

这些标注结果已经写入对应的 CSV 文件，可作为后续机器学习模型训练数据的初始样本。

### 2.3 自定义应用层协议设计

本项目选择的是题目二“网络流量分类器”，不是题目一“TCP Socket 聊天工具”，因此系统没有自定义聊天应用层协议，也不存在客户端与服务器之间的 TLV 消息格式。

本系统中与“协议设计”对应的是两部分：

1. 网络流五元组格式。
2. CSV 特征输出格式。

#### 2.3.1 五元组流标识格式

流标识格式如下：

```text
<endpoint_a_ip>:<endpoint_a_port> <-> <endpoint_b_ip>:<endpoint_b_port> <protocol>
```

示例：

```text
172.27.152.109:53209 <-> 202.114.200.251:53 UDP
```

#### 2.3.2 CSV 输出格式

CSV 每一行表示一条网络流，主要字段如下：

```text
flow_id,src_ip,dst_ip,src_port,dst_port,protocol,
packet_count,total_bytes,avg_packet_size,max_packet_size,
min_packet_size,duration,avg_interval,interval_variance,
avg_ttl,syn_count,ack_count,fin_count,rst_count,psh_count,
unique_ports,label,confidence,reason
```

#### 2.3.3 核心伪代码

数据包分析流程伪代码如下：

```python
def analyze_capture(path):
    raw_packets = read_capture(path)
    packets = []

    for raw in raw_packets:
        packet = parse_packet(raw)
        if packet is not None:
            packets.append(packet)

    flows = build_flows(packets)
    rows = extract_feature_rows(flows)
    classified_rows = classify_rows(rows)
    return packets, classified_rows
```

五元组聚合伪代码如下：

```python
def build_flows(packets):
    flows = {}
    for packet in packets:
        key = normalize_five_tuple(packet)
        if key not in flows:
            flows[key] = Flow(key)
        flows[key].add(packet)
    return flows
```

端口扫描检测伪代码如下：

```python
def mark_port_scans(rows):
    for each TCP_Probe row:
        group by (src_ip, dst_ip)
        collect dst_port

    if one (src_ip, dst_ip) pair touches at least 5 ports:
        mark related rows as Port_Scan
        set confidence to 0.80
        output ALERT
```

#### 2.3.4 TCP 粘包问题说明

TCP 粘包问题主要出现在自定义 TCP 应用层协议中，即接收端需要通过长度字段区分消息边界。本项目不直接实现 TCP Socket 通信，而是读取 Wireshark 抓到的完整链路层帧，因此不需要在程序中处理应用层粘包。

对于本系统而言，数据包边界由 pcap/pcapng 文件格式提供，每个抓包记录天然对应一个被捕获的数据帧。

## 第 3 章 五层体系结构原理分析与验证

### 3.1 五层体系结构

计算机网络五层体系结构包括：

| 层次 | 作用 | 本项目中的体现 |
| --- | --- | --- |
| 应用层 | 产生具体应用数据 | DNS 查询、HTTP/HTTPS 访问、ping |
| 传输层 | 端到端通信 | TCP、UDP 端口和 flags |
| 网络层 | 主机到主机寻址 | IPv4、源 IP、目的 IP、TTL |
| 数据链路层 | 局域网帧传输 | Ethernet MAC 地址、以太网类型 |
| 物理层 | 比特流传输 | Wi-Fi 无线信号，由网卡完成 |

### 3.2 HTTP/HTTPS 请求的封装过程

以浏览器访问 HTTPS 网站为例，HTTP 请求从浏览器产生到被本机抓包程序捕获，大致经历以下过程。

#### 3.2.1 发送方向的封装过程

1. 应用层

浏览器根据用户输入的网址生成应用层数据。若访问 HTTPS 网站，浏览器会先进行 DNS 查询获得服务器 IP 地址，然后建立 TCP 连接和 TLS 会话。真正的 HTTP 请求会被 TLS 加密，形成加密后的应用层数据。

该层产生的数据可以理解为：

```text
HTTP 请求数据 / TLS 加密数据
```

2. 传输层

操作系统 TCP 协议栈为应用层数据添加 TCP 头部，形成 TCP 段。TCP 头部中包含：

```text
源端口、目的端口、序号、确认号、窗口大小、校验和、TCP flags
```

访问 HTTPS 网站时，目的端口通常为 443；访问普通 HTTP 网站时，目的端口通常为 80。TCP flags 可体现连接建立、数据传输和连接释放过程，例如 SYN、ACK、PSH、FIN。

3. 网络层

IP 协议为 TCP 段添加 IPv4 头部，形成 IP 数据包。IPv4 头部中包含：

```text
源 IP、目的 IP、TTL、协议号、总长度、头部校验和
```

其中协议号为 6 表示上层协议是 TCP；协议号为 17 表示 UDP；协议号为 1 表示 ICMP。本项目会解析源 IP、目的 IP、TTL、协议号和总长度等字段，并作为特征提取的基础。

4. 数据链路层

网卡驱动为 IP 数据包添加 Ethernet 头部，形成以太网帧。Ethernet 头部中包含：

```text
源 MAC 地址、目的 MAC 地址、以太网类型
```

以太网类型 `0x0800` 表示载荷是 IPv4。本项目在解析时先读取 Ethernet 头部，再判断是否为 IPv4 数据包。

5. 物理层

Wi-Fi 网卡将以太网帧转换为无线信号发送出去。物理层负责比特流传输，具体调制、编码和无线传输过程由网卡硬件完成。

发送方向的封装关系如下：

```text
应用层数据
  -> TCP 头部 + 应用层数据
  -> IP 头部 + TCP 头部 + 应用层数据
  -> Ethernet 头部 + IP 头部 + TCP 头部 + 应用层数据
  -> 物理层比特流
```

#### 3.2.2 被程序捕获时的解封装过程

Wireshark 或抓包库在网卡处捕获到的是链路层帧。程序读取 pcapng 文件后，按以下顺序逐层解析：

1. 读取 pcapng 记录，获得一帧原始字节和时间戳。
2. 解析 Ethernet 头部，得到源 MAC、目的 MAC 和以太网类型。
3. 若以太网类型为 IPv4，则解析 IPv4 头部，得到源 IP、目的 IP、TTL、总长度和协议号。
4. 若协议号为 TCP，则解析 TCP 头部，得到源端口、目的端口和 flags。
5. 若协议号为 UDP，则解析 UDP 头部，得到源端口和目的端口。
6. 若协议号为 ICMP，则解析 ICMP 类型字段。
7. 将解析出的字段封装为 `PacketRecord`，再按五元组聚合为网络流。

解封装关系如下：

```text
Ethernet 帧
  -> 去掉 Ethernet 头部，得到 IP 数据包
  -> 去掉 IP 头部，得到 TCP/UDP/ICMP 数据
  -> 读取传输层字段，形成流量特征
```

因此，本项目虽然不直接处理浏览器内部的 HTTP 明文内容，但能够通过端口号、协议号、包长、时间间隔、TTL、TCP flags 等头部字段判断该流量属于 Web 类流量。

### 3.3 Wireshark 抓包验证

本实验通过 Wireshark 采集了本机 Wi-Fi 网卡上的真实流量，并保存到：

```text
CPE/traffic_classifier/samples/dns.pcapng
CPE/traffic_classifier/samples/icmp.pcapng
CPE/traffic_classifier/samples/web.pcapng
```

建议在报告最终版中插入以下截图：

1. Wireshark 选择 Wi-Fi 网卡并开始抓包的截图。
2. DNS 流量过滤截图，过滤条件为：

```text
dns
```

3. ICMP 流量过滤截图，过滤条件为：

```text
icmp
```

4. Web 流量过滤截图，过滤条件为：

```text
tcp.port == 80 or tcp.port == 443
```

5. 任意一个 TCP 包的分层字段截图，包括 Frame、Ethernet II、Internet Protocol Version 4、Transmission Control Protocol。

### 3.4 抓包结果说明

本次采集到的真实流量如下：

| 样本文件 | 解析包数量 | 聚合流数量 | 分类结果 |
| --- | ---: | ---: | --- |
| dns.pcapng | 526 | 245 | 245 条 DNS |
| icmp.pcapng | 8 | 1 | 1 条 ICMP |
| web.pcapng | 42749 | 96 | 96 条 Web |

示例输出：

```text
capture: samples/dns.pcapng
parsed_packets: 526
flows: 245

1. DNS confidence=0.92 packets=2 bytes=481
```

```text
capture: samples/icmp.pcapng
parsed_packets: 8
flows: 1

1. ICMP confidence=0.95 packets=8 bytes=480
```

```text
capture: samples/web.pcapng
parsed_packets: 42749
flows: 96

1. Web confidence=0.88 packets=15 bytes=2115
```

## 第 4 章 AI 集成实现

### 4.1 AI 方法选择

课程题目要求可以训练简单机器学习模型，也可以调用云端分类 API。常见方案包括使用 UNSW-NB15、ISCX VPN-nonVPN 等公开数据集训练决策树、KNN 或随机森林模型，然后将模型保存并在预测阶段加载。

当前项目实现了两套分类方式：

1. 规则分类器：根据协议、端口和 TCP flags 给出可解释的类别。
2. KNN 分类器：读取带标签 CSV，训练 KNN 模型，保存为 JSON 文件，并在预测阶段加载模型输出类别和置信度。

KNN 方案没有依赖 scikit-learn，而是使用 Python 标准库实现距离归一化和距离加权投票。这样做可以减少环境依赖，便于在课程演示环境中直接运行。

当前规则分类器的优势是：

1. 不依赖第三方机器学习库。
2. 分类依据清晰，便于解释。
3. 适合在课程演示中展示协议字段与分类结果之间的关系。
4. 可将 CSV 输出作为机器学习训练集。

不足是：

1. 对复杂流量类型的识别能力有限。
2. 对未知应用流量泛化能力不足。
3. 攻击检测规则可能产生误报。
4. 对复杂业务语义的判断能力弱。

KNN 分类器使用的特征包括：

```text
protocol_tcp, protocol_udp, protocol_icmp,
src_port, dst_port, packet_count, total_bytes,
avg_packet_size, max_packet_size, min_packet_size,
duration, avg_interval, interval_variance, avg_ttl,
syn_count, ack_count, fin_count, rst_count, psh_count,
unique_ports
```

训练命令如下。为了避免合成误报样本影响普通流量识别，当前 KNN 普通分类模型只使用真实 DNS、ICMP、Web 三类样本训练；`false_positive_port_scan.pcap` 继续用于攻击报警和误报分析，不放入普通 KNN 训练集。

```bash
python -m traffic_classifier.train output/dns_features.csv output/icmp_features.csv output/web_features.csv --model models/knn_model.json
```

训练结果：

```text
model: models/knn_model.json
samples: 342
labels: DNS, ICMP, Web
k: 3
```

预测命令如下：

```bash
python -m traffic_classifier.predict samples/web.pcapng --model models/knn_model.json
```

### 4.2 分类规则实现

核心分类逻辑如下：

```python
if protocol == "ICMP":
    return "ICMP"

if 53 in ports and protocol in {"UDP", "TCP"}:
    return "DNS"

if protocol == "TCP" and syn_count >= 20 and syn_count / max(ack_count, 1) >= 4:
    return "SYN_Flood"

if protocol == "TCP" and 22 in ports:
    return "SSH"

if protocol == "TCP" and (80 in ports or 443 in ports):
    return "Web"

if protocol == "TCP" and packet_count <= 3 and syn_count > 0:
    return "TCP_Probe"
```

端口扫描检测基于多条 TCP 探测流进行判断：

```text
同一源 IP -> 同一目的 IP
短时间内访问 5 个及以上不同目标端口
=> Port_Scan
```

### 4.3 攻击检测与报警

系统支持以下攻击或异常流量检测：

| 攻击类别 | 检测依据 | 报警方式 |
| --- | --- | --- |
| SYN_Flood | 大量 SYN，ACK 很少 | 输出 ALERT |
| TCP_Probe | 短 TCP 探测流 | 输出 ALERT |
| Port_Scan | 同一主机访问多个端口 | 输出 ALERT |

报警输出示例：

```text
1. ALERT Port_Scan confidence=0.80 packets=1 bytes=40
```

### 4.4 误报模拟与分析

为了验证规则检测的局限性，实验构造了一个误报样本：

```text
CPE/traffic_classifier/samples/false_positive_port_scan.pcap
```

该样本模拟一个正常客户端在短时间内连接同一服务器的多个端口：

```text
8001, 8002, 8003, 8004, 8005
```

程序输出：

```text
capture: samples/false_positive_port_scan.pcap
parsed_packets: 5
flows: 5

1. ALERT Port_Scan confidence=0.80
2. ALERT Port_Scan confidence=0.80
3. ALERT Port_Scan confidence=0.80
4. ALERT Port_Scan confidence=0.80
5. ALERT Port_Scan confidence=0.80
```

该结果说明，基于规则的端口扫描检测虽然简单有效，但如果正常业务程序确实需要连接多个服务端口，也可能被误判为端口扫描。因此，后续可以结合时间窗口、连接成功率、应用白名单和机器学习模型降低误报率。

### 4.5 KNN 模型训练与后续扩展

当前系统已经生成如下 CSV 数据：

```text
output/dns_features.csv
output/icmp_features.csv
output/web_features.csv
output/false_positive_port_scan_features.csv
```

这些 CSV 文件包含 `label` 字段，可作为小规模标注数据集。当前普通 KNN 分类模型使用 DNS、ICMP、Web 三类真实样本训练；合成的 `false_positive_port_scan_features.csv` 只用于攻击规则和误报分析，不加入普通模型训练：

1. 人工确认并标注每条流的真实类别。
2. 使用协议 one-hot、端口、包数量、总字节数、平均包长、持续时间、到达间隔方差、TTL、TCP flags 等特征作为输入。
3. 对数值特征进行均值和标准差归一化。
4. 使用 KNN 距离加权投票进行分类。
5. 将模型保存为 `models/knn_model.json`。
6. 在预测阶段加载模型，对新 pcap 文件中的每条流输出 `ml_label` 和 `ml_confidence`。

当前 KNN 训练流程如下：

```text
多个 CSV 文件
     |
     v
合并样本并选择数值特征
     |
     v
计算均值和标准差进行归一化
     |
     v
保存训练样本、特征列、归一化参数和 k 值
     |
     v
加载 JSON 模型
     |
     v
预测新 pcap 文件中的流量类别和置信度
```

使用模型预测已有样本的结果示例：

```text
DNS 样本：ML=DNS，confidence=1.00
ICMP 样本：ML=ICMP，confidence=1.00
Web 样本：ML=Web，confidence=1.00
```

KNN 支持多分类。新增类别时，只需要采集对应类别的 pcapng 文件，生成 CSV 后加入训练命令即可。如果 CSV 中原有 `label` 不符合目标类别，可以用 `--label` 按文件覆盖标签，例如：

```bash
python -m traffic_classifier.train output/dns_features.csv output/web_features.csv output/video_features.csv \
  --label video_features.csv=Video \
  --model models/knn_model.json
```

对 100 包样本 `live_100.pcapng` 重新预测后，当前 KNN 输出为：

```text
parsed_packets: 105
flows: 19
rule labels: {'Web': 10, 'Unknown': 3, 'Small_UDP': 5, 'DNS': 1}
ml labels: {'Web': 15, 'DNS': 4}
```

前 10 条预测中主要结果置信度为 `confidence=1.00`。但需要注意，当前 KNN 只训练了 DNS、ICMP、Web 三类，因此 Unknown 和 Small_UDP 这类未训练类别会被归入最相近的已知类别，后续应通过新增类别样本扩展模型。

后续如果继续完善，可以将当前标准库 KNN 替换为 scikit-learn 的 DecisionTreeClassifier 或 RandomForestClassifier，并增加训练集/测试集划分、准确率、召回率和混淆矩阵等评估指标。

## 第 5 章 系统测试与验证

### 5.1 功能测试

#### 5.1.1 DNS 流量测试

测试命令：

```bash
python -m traffic_classifier samples/dns.pcapng --csv output/dns_features.csv
```

测试结果：

```text
parsed_packets: 526
flows: 245
分类结果：245 条 DNS
```

说明：DNS 样本中的流量均被识别为 DNS，符合预期。

#### 5.1.2 ICMP 流量测试

测试命令：

```bash
python -m traffic_classifier samples/icmp.pcapng --csv output/icmp_features.csv
```

测试结果：

```text
parsed_packets: 8
flows: 1
分类结果：1 条 ICMP
```

说明：ping 产生的 ICMP 流量被正确识别。

#### 5.1.3 Web 流量测试

测试命令：

```bash
python -m traffic_classifier samples/web.pcapng --csv output/web_features.csv
```

测试结果：

```text
parsed_packets: 42749
flows: 96
分类结果：96 条 Web
```

说明：访问网页产生的 TCP 443 流量被识别为 Web。

#### 5.1.4 攻击检测测试

系统通过构造测试覆盖了以下攻击类别：

```text
SYN_Flood
TCP_Probe
Port_Scan
```

测试结果：

```text
pipeline tests passed
```

#### 5.1.5 误报测试

测试命令：

```bash
python -m traffic_classifier samples/false_positive_port_scan.pcap --csv output/false_positive_port_scan_features.csv
```

测试结果：

```text
parsed_packets: 5
flows: 5
分类结果：5 条 Port_Scan
```

说明：该样本模拟正常程序连接多个端口，但被规则误判为端口扫描，说明规则检测方法存在误报风险。

#### 5.1.6 真实校园网/Wi-Fi 流量标注测试

本实验采集的三类真实流量均来自本机 Wi-Fi 网络环境：

| 文件 | 采集方式 | 人工标注 |
| --- | --- | --- |
| `dns.pcapng` | 抓包时执行 DNS 查询 | DNS |
| `icmp.pcapng` | 抓包时执行 ping | ICMP |
| `web.pcapng` | 抓包时访问网页 | Web |

程序分析后生成：

```text
output/dns_features.csv
output/icmp_features.csv
output/web_features.csv
```

CSV 中的 `label` 字段可作为后续机器学习训练的类别标签。该部分对应拓展要求中的“自己采集一小批真实校园网流量并标注”，但当前只完成了小规模采集和规则标注，尚未完成基于这些数据的模型微调。

#### 5.1.7 实时显示要求说明

课程核心要求中提到“程序运行时可选择网卡，抓取 100 个包后输出当前流量类别及置信度”。当前项目已经实现命令行实时抓包入口：

```bash
python -m traffic_classifier.live_capture --count 100
python -m traffic_classifier.live_capture -i eth0 --count 100 --model models/knn_model.json
```

程序运行时可列出网卡并让用户选择，也可以直接用 `-i` 指定网卡。本实验环境中检测到的网卡为：

```text
eth0
```

验证实时抓包时，程序能够进入抓包流程，但当前环境没有原始套接字权限，系统返回：

```text
error: [Errno 1] Operation not permitted
hint: live capture usually needs root or CAP_NET_RAW permission
```

因此，代码层面已经完成“选择网卡、抓取指定数量包并输出类别和置信度”的实现；实际运行需要在具备 root 或 `CAP_NET_RAW` 权限的 Linux 环境中执行。

#### 5.1.8 100 包样本读取与 KNN 更新测试

由于当前实验环境没有原始套接字权限，实时抓包无法直接执行，因此使用 Wireshark 抓取约 100 个包并保存为：

```text
samples/live_100.pcapng
```

读取命令：

```bash
python -m traffic_classifier.predict samples/live_100.pcapng --model models/knn_model.json --limit 10
```

读取结果：

```text
parsed_packets: 105
flows: 19
```

规则分类统计：

```text
Web        10
Small_UDP   5
Unknown     3
DNS         1
```

首次将合成的 `false_positive_port_scan_features.csv` 加入 KNN 训练后，`live_100.pcapng` 中多条正常 Web 流被误判为 `Port_Scan`。因此重新训练 KNN 模型时，只保留真实 DNS、ICMP、Web 三类样本，训练结果为：

```text
model: models/knn_model.json
samples: 342
labels: DNS, ICMP, Web
k: 3
```

重新预测 `live_100.pcapng` 后，KNN 分类统计为：

```text
Web  15
DNS   4
```

前 10 条输出中主要分类置信度为：

```text
confidence=1.00
```

该结果说明，去掉合成误报样本后，KNN 不再把普通流量误判为 Port_Scan；但由于模型当前只有 DNS、ICMP、Web 三类，Unknown 和 Small_UDP 会被归入最接近的已知类别。

### 5.2 输出 CSV 验证

CSV 文件中包含流量特征、分类标签、置信度和分类原因。DNS 样本输出示例：

```text
flow_id,src_ip,dst_ip,src_port,dst_port,protocol,packet_count,total_bytes,...
172.27.152.109:53209 <-> 202.114.200.251:53 UDP,...,DNS,0.92,port 53 DNS traffic
```

这说明程序不仅能给出分类结果，还能导出结构化数据，便于后续训练模型和撰写实验分析。

### 5.3 性能测试

本次 Web 样本规模最大：

```text
parsed_packets: 42749
flows: 96
```

程序能够完成解析、聚合、特征提取和分类，说明当前实现可以处理数万级别的数据包样本。由于系统采用离线分析方式，主要性能瓶颈在文件读取和逐包解析。

### 5.4 测试结论

测试结果表明：

1. 系统能够读取真实 Wi-Fi 抓包文件。
2. 系统能够正确解析 IPv4、TCP、UDP、ICMP 流量。
3. 系统能够按五元组聚合网络流。
4. 系统能够提取流量统计特征。
5. 系统能够识别 DNS、ICMP、Web 等常见流量。
6. 系统能够检测 SYN Flood、TCP Probe、Port Scan 并报警。
7. 系统已经采集并标注一小批真实 Wi-Fi 流量，可作为后续机器学习训练数据。
8. 系统已经训练 KNN 模型，并能加载模型预测 DNS、ICMP、Web 样本。
9. 系统已经实现实时网卡选择和抓包入口，但实际抓包需要 root 或 `CAP_NET_RAW` 权限。
10. 系统已读取 `live_100.pcapng`，完成约 100 包样本的模型预测演示。
11. 规则检测存在误报，需要在后续工作中优化。
12. 实时图形化柱状图显示仍未完成。

## 第 6 章 总结与展望

### 6.1 总结

本课程设计实现了一个 AI 驱动的网络流量分类器原型系统。系统通过 Wireshark 采集真实 Wi-Fi 流量，使用 Python 程序读取 pcap/pcapng 文件，解析 Ethernet、IPv4、TCP、UDP、ICMP 等协议字段，并按五元组聚合网络流。该实现满足“抓包模块可读取 pcap 文件”和“按五元组提取特征”的核心要求。

在此基础上，系统提取包数量、总字节数、平均包长、到达间隔方差、TTL、TCP flags 等特征，并使用规则分类方法识别 DNS、Web、ICMP 等流量类别。系统还实现了 SYN Flood、TCP Probe、Port Scan 等攻击检测，并在检测到攻击流量时输出 ALERT 报警，完成了拓展要求中的攻击检测与报警功能。

通过真实样本测试，系统成功识别：

```text
245 条 DNS 流
1 条 ICMP 流
96 条 Web 流
```

通过合成样本测试，系统验证了端口扫描报警功能，同时也模拟出了误报场景，说明简单规则方法虽然可解释性强，但仍需要进一步优化。

从课程要求对照来看，本项目已经完成 pcap 文件读取、五元组特征提取、KNN 模型训练与加载预测、类别与置信度输出、HTTP 五层封装/解封装分析、真实流量采集标注、攻击检测与报警等内容。程序内实时选择网卡和抓取 100 个包的入口已经实现，但实际抓包需要 root 或 `CAP_NET_RAW` 权限；实时图形化柱状图显示尚未完成。

### 6.2 不足

当前系统仍存在以下不足：

1. KNN 模型使用的小规模自采样本训练，样本类别和数量仍然有限。
2. 当前 KNN 为标准库实现，缺少训练集/测试集划分、准确率、召回率和混淆矩阵等正式评估。
3. 实时抓包依赖 Linux 原始套接字权限，在无 root 或 `CAP_NET_RAW` 的环境中无法实际捕获数据包。
4. 实时图形化界面尚未实现。
5. 流量类别较少，尚未覆盖 P2P、VoIP、游戏等复杂应用。
6. 攻击检测规则较简单，可能产生误报。
7. 未引入时间窗口、连接成功率等更复杂的行为特征。

### 6.3 展望

后续可以从以下方向继续改进：

1. 引入 scikit-learn，使用决策树或随机森林替换当前标准库 KNN，并输出更完整的评估指标。
2. 增加训练集/测试集划分、混淆矩阵和分类报告。
3. 在真实 Linux 主机上配置抓包权限，完整验证实时抓包 100 个包后的分类输出。
4. 实现图形化界面，用柱状图展示流量类别随时间的变化。
5. 扩展攻击检测规则，例如识别 UDP Flood、ICMP Flood、横向扫描等。
6. 使用更多校园网真实流量样本进行标注和训练，提高分类准确率。
7. 对误报场景进行优化，例如增加白名单、时间窗口和连接成功率判断。

综上，本项目完成了网络流量分类器的主要流程，并结合真实抓包数据验证了协议解析、特征提取、规则分类、KNN 模型预测和攻击报警功能。后续若补充图形化界面、更大规模数据集和更完整的模型评估，即可进一步提升系统完整性和展示效果。
