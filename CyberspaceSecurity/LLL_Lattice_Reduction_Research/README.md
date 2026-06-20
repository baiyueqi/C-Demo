# LLL 格基约化算法分析与优化研究

本目录用于“项目 4：LLL 格基约化算法分析与优化研究”。

当前内容：

- `docs/`：课程文档与报告。
  - [设计文档.md](docs/设计文档.md)：研究目标、算法原理、Babai 改进思路、LLL 参数优化方案、BKZ/DeepLLL/PotLLL 对比评测设计、后续代码结构建议。
  - [实验报告.md](docs/实验报告.md)：实验环境、系统界面截图标注、测试数据、测试结果、源码解释和调试记录。
- `src/`：C++17 原型实现，包括 LLL、Gram-Schmidt、Babai 改进、参数扫描、mini-BKZ 和 DeepLLL/PotLLL 风格后处理。
- `tests/`：小维度正确性测试，覆盖 LLL 输出条件、Babai 与穷举 CVP 对照、高级约化输出校验。
- `Makefile`：目录内独立构建入口，不依赖仓库根目录 CMake。

目录分类：

```text
LLL_Lattice_Reduction_Research/
├── docs/       设计文档和实验报告
├── src/        算法实现与命令行实验入口
├── tests/      小维度正确性测试
├── Makefile    独立构建、测试和演示入口
└── README.md   项目说明
```

`build/` 和 `results/` 是本地生成目录，已在 `.gitignore` 中忽略，不提交到仓库。需要结果文件时可按下方命令重新生成。

## 构建与测试

在仓库根目录进入开发环境后执行：

```bash
nix develop
make -C CyberspaceSecurity/LLL_Lattice_Reduction_Research
```

默认 `make` 会构建实验程序，并直接在终端展示一组小规模结果：

1. LLL 单次约化指标。
2. 参数扫描推荐和部分扫描行。
3. Babai 在原始基与 LLL 约化基上的优化前后对比。
4. LLL 与 mini-BKZ、DeepLLL/PotLLL post-pass 的高级约化对比。

只构建实验程序，不运行展示：

```bash
make -C CyberspaceSecurity/LLL_Lattice_Reduction_Research build
```

只运行测试：

```bash
make -C CyberspaceSecurity/LLL_Lattice_Reduction_Research test
```

运行单次 LLL：

```bash
cd CyberspaceSecurity/LLL_Lattice_Reduction_Research
./build/lll_experiment --algorithm lll --dimension 20 --bits 8 --delta 0.99 --eta 0.51 --strategy suffix
```

运行第二阶段参数扫描并输出 CSV：

```bash
cd CyberspaceSecurity/LLL_Lattice_Reduction_Research
./build/lll_experiment --algorithm scan --dimension 20 --bits 8 --repeat 3 --output results/lll_parameter_scan.csv
```

运行 Babai 基线演示：

```bash
cd CyberspaceSecurity/LLL_Lattice_Reduction_Research
./build/lll_experiment --algorithm babai --dimension 6 --bits 8
```

运行第三阶段 Babai 改进对比：

```bash
cd CyberspaceSecurity/LLL_Lattice_Reduction_Research
./build/lll_experiment --algorithm babai-compare --dimension 6 --bits 8 --repeat 5 --output results/babai_comparison.csv
```

运行第四阶段高级约化对比：

```bash
cd CyberspaceSecurity/LLL_Lattice_Reduction_Research
./build/lll_experiment --algorithm advanced --dimension 12 --bits 8 --repeat 2 --output results/advanced_comparison.csv
```

生成第五阶段 Markdown 汇总报告和配套 CSV：

```bash
cd CyberspaceSecurity/LLL_Lattice_Reduction_Research
./build/lll_experiment --algorithm report --dimension 8 --bits 6 --repeat 1 --output results/summary_report.md
```

## 阶段进度

第一阶段已实现：

1. LLL、Gram-Schmidt、Babai 最近平面基础实现。
2. 固定随机种子的小维度整数格生成器。
3. LLL reduced 条件校验。
4. Babai 与小维度穷举 CVP 对照测试。

第二阶段已实现：

1. `delta`、`eta` 参数扫描。
2. `full` 与 `suffix` 两种 GSO 更新策略对比。
3. CSV 输出运行时间、交换次数、尺寸约化次数、GSO 更新量、首向量范数、行列式估计、根 Hermite 因子、正交缺陷和最终校验结果。
4. 扫描结束后输出质量优先和速度优先推荐参数。

第三阶段已实现：

1. 基础 Babai 最近平面算法。
2. 固定随机种子的随机舍入 Babai。
3. 尾部小块枚举 Babai，支持 `tailBlockSize`、`windowRadius`、`maxCandidates`。
4. 残差局部搜索 Babai，支持搜索向量数和窗口半径。
5. `babai-compare` 输出原始基与 LLL 约化基上的距离、近似比、精确命中和候选数量。

第四阶段已实现：

1. `mini-bkz`：小块枚举替换块首向量，并用 LLL 后处理。
2. `deep-lll-postpass`：受限深插入后处理，用于观察质量/时间权衡。
3. `pot-lll-postpass`：基于 GSO 势能下降阈值的受限插入后处理。
4. `advanced` 输出 LLL 与三个高级原型的运行时间、候选数、插入次数、RHF、正交缺陷和最终校验结果。

第五阶段已实现：

1. `report` 一键生成参数扫描、Babai 对比、高级约化对比 CSV。
2. 同步生成 Markdown 报告骨架，记录实验配置、输出文件和结果解读要点。

说明：第四阶段的 DeepLLL/PotLLL 是课程实验用的 bounded insertion post-pass，不是 fplll 级完整实现。若要做真实密码分析级 BKZ 对照，应接入 fplll/fpylll。

后续可继续补充：

```text
data/       固定随机种子的实验输入
results/    参数扫描与算法对比结果
notebooks/  实验可视化和报告图表
```
