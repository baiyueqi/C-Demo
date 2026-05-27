# AI 网络流量分类器

这是题目二“AI 驱动的网络流量分类器”的程序框架。当前版本不依赖第三方 Python 包，支持离线 pcap 分析、KNN 模型训练/预测，以及 Linux 网卡实时抓包：

```text
pcap/pcapng 文件或实时网卡 -> IPv4/TCP/UDP/ICMP 解析 -> 五元组聚合 -> 特征提取 -> 规则分类 / KNN 分类 -> CSV
```

## 功能

- 读取 `.pcap` 和常见 `.pcapng` 文件。
- 解析 Ethernet + IPv4 + TCP/UDP/ICMP。
- 按双向五元组聚合网络流。
- 提取包数量、总字节数、平均包长、到达间隔方差、TTL、TCP flags 等特征。
- 使用透明规则识别 `Web`、`DNS`、`SSH`、`ICMP`、`SYN_Flood`、`Small_UDP`、`TCP_Probe`、`Unknown`。
- 可导出 CSV，作为机器学习训练数据。
- 可用带标签 CSV 训练标准库 KNN 模型并保存为 JSON。
- Linux 下可选择网卡，实时抓取指定数量的数据包后输出类别和置信度。

## 使用方式

把 Wireshark 抓到的文件放到 `samples/`，然后运行：

```bash
python -m traffic_classifier samples/web.pcapng --csv output/web_features.csv
```

在仓库根目录运行时：

```bash
python -m CPE.traffic_classifier.traffic_classifier CPE/traffic_classifier/samples/web.pcapng \
  --csv CPE/traffic_classifier/output/web_features.csv
```

更推荐进入本目录运行：

```bash
cd CPE/traffic_classifier
python -m traffic_classifier samples/web.pcapng --csv output/web_features.csv
```

## 推荐样本

每类单独抓一个文件，文件名直接体现标签：

```text
samples/dns.pcapng
samples/web.pcapng
samples/icmp.pcapng
samples/ssh.pcapng
```

## 后续机器学习扩展

使用已生成 CSV 训练 KNN 模型：

```bash
python -m traffic_classifier.train output/dns_features.csv output/icmp_features.csv output/web_features.csv \
  --model models/knn_model.json
```

KNN 支持多分类。新增类别时，先采集对应 pcap，生成 CSV，再加入训练命令即可。如果 CSV 里的 `label` 需要按文件整体覆盖，可以使用 `--label`：

```bash
python -m traffic_classifier.train output/dns_features.csv output/web_features.csv output/video_features.csv \
  --label video_features.csv=Video \
  --model models/knn_model.json
```

当前模型使用真实 DNS、ICMP、Web 样本训练。不要把误报演示样本加入普通流量模型，否则新流量容易被误判成 `Port_Scan`。

加载模型预测新抓包：

```bash
python -m traffic_classifier.predict samples/web.pcapng --model models/knn_model.json
```

实时选择网卡并抓取 100 个包：

```bash
python -m traffic_classifier.live_capture --count 100
```

实时抓包并使用 KNN 模型输出类别和置信度：

```bash
python -m traffic_classifier.live_capture --count 100 --model models/knn_model.json
```

实时抓包通常需要 root 或 `CAP_NET_RAW` 权限。
