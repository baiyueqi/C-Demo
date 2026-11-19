# dp_lab.cpp 详细讲解

本文档对 `InformationSecurity/Experiment/dp_lab.cpp` 做逐模块、逐机制的详细讲解，内容包括实验目标、差分隐私核心公式、代码结构、运行输出以及隐私/可用性分析。

---

## 1. 实验概述

`dp_lab.cpp` 是一个 C++ 差分隐私教学程序，用来演示两个最基础的差分隐私机制：

1. **Laplace 机制**：对数值型查询结果添加 Laplace 噪声。
2. **Exponential 机制**：对非数值型候选结果按效用函数进行概率抽样。

实验数据模拟一个包含 20 名患者的医疗数据表，每条记录只有 0 或 1：

- `1` 表示该患者患病；
- `0` 表示该患者未患病。

程序通过 `count(n)` 查询说明一个典型差分攻击：

- 攻击者先查询前 3 行患病总人数 `count(3)`；
- 再查询前 2 行患病总人数 `count(2)`；
- 两者相减，刚好得到第 3 行患者 Jack 的敏感属性。

加入 Laplace 噪声后，`count(3)` 和 `count(2)` 都变成带噪声的实数，攻击者无法再通过简单相减稳定地推断 Jack 的真实患病状态。

---

## 2. 差分隐私关键概念

### 2.1 差分隐私的直观含义

差分隐私希望做到：**查询结果不应该因为某一条记录是否存在于数据集中而发生显著变化**。攻击者即使知道除目标个体之外的所有记录，也难以可靠地推断目标个体的敏感属性。

形式化地，如果一个随机算法 M 对任意相邻数据集 D 与 D'，以及任意输出集合 S 都满足：

```text
Pr[M(D) ∈ S] <= e^ε * Pr[M(D') ∈ S]
```

则称 M 满足 ε-差分隐私。其中：

- ε 为隐私预算；
- ε 越小，隐私保护越强；
- ε 越大，查询结果越接近真实值，但隐私保护越弱。

### 2.2 全局敏感度

对于查询函数 f，其全局敏感度 Δf 定义为：

```text
Δf = max_{D, D'} || f(D) - f(D') ||_1
```

其中 D 和 D' 是只相差一条记录的相邻数据集。

在本实验中，`count(n)` 查询改变任意一条记录时，查询结果最多变化 1，因此：

```text
Δf = 1
```

### 2.3 Laplace 机制

Laplace 机制对数值型查询结果添加 Laplace 噪声：

```text
M(D) = f(D) + Lap(0, Δf / ε)
```

Laplace 分布 Lap(μ, b) 的概率密度函数为：

```text
f(x) = (1 / (2b)) * exp(-|x - μ| / b)
```

在本程序中：

- 位置参数 μ = 0；
- 尺度参数 b = Δf / ε = 1 / ε。

因此，ε 越小，尺度 b 越大，噪声越大；ε 越大，噪声越小，结果越接近真实值。

### 2.4 Exponential 机制

Exponential 机制用于“方案选择”这类不能直接做数值加法的场景。

设候选集合为 O，候选为 o，效用函数为 q(D, o)，效用函数敏感度为 Δq，则输出候选 o 的概率正比于：

```text
exp(ε * q(D, o) / (2 * Δq))
```

程序中的“三个运动项目票选”就是利用该公式，把真实票数作为效用函数值，对候选项目进行加权抽样。

---

## 3. 代码结构总览

| 模块 | 功能 |
| --- | --- |
| `LaplaceMechanism` | 封装 Laplace 噪声生成和“真实值 + 噪声”查询过程 |
| `prefixCount` | 计算前 n 行患病人数，即 `count(n)` |
| `printDataset` | 打印原始数据集及每个位置的前缀计数 |
| `demonstrateLaplaceMechanism` | 演示无噪声差分攻击、Laplace 加噪结果、不同 ε 的平均误差 |
| `demonstrateExponentialMechanism` | 演示 Exponential 机制对非数值候选的概率选择 |
| `main` | 构造数据集并依次调用上述演示函数 |

整个实现位于匿名命名空间中，避免符号泄漏到全局作用域。

---

## 4. 逐模块详解

### 4.1 头文件与程序说明

程序包含以下标准库：

- `<algorithm>`：使用 `std::min`；
- `<cmath>`：使用 `std::log`、`std::exp`、`std::fabs`；
- `<iomanip>`：控制浮点数输出格式；
- `<iostream>`：标准输入输出；
- `<numeric>`：使用 `std::accumulate`；
- `<random>`：使用 `std::mt19937`、`std::uniform_real_distribution`、`std::discrete_distribution`；
- `<string>`：使用 `std::string`；
- `<vector>`：使用 `std::vector`。

代码开头的注释清楚说明两个机制的用途，以及数据集攻击原理。

---

### 4.2 `LaplaceMechanism`

这是整个程序的核心类之一。

#### 构造函数

```cpp
explicit LaplaceMechanism(unsigned int seed) : rng_(seed) {}
```

使用固定种子初始化 Mersenne Twister 随机数发生器 `std::mt19937`，目的是让每次运行得到相同的随机序列，方便实验复现和教学演示。

#### `addNoise` 方法

```cpp
double addNoise(double value, double epsilon, double sensitivity = 1.0) {
  const double scale = sensitivity / epsilon;
  return value + noise(scale);
}
```

参数含义：

- `value`：真实查询结果；
- `epsilon`：隐私预算；
- `sensitivity`：查询函数全局敏感度，默认值为 1。

计算过程：

```text
scale = sensitivity / ε
```

随后返回：

```text
noisy_answer = value + Lap(0, scale)
```

由于 `count(n)` 的敏感度为 1，默认参数正合适。

#### `noise` 方法

该方法使用逆变换法生成服从 Lap(0, scale) 的随机噪声。

Laplace 分布 Lap(0, b) 的累积分布函数为：

```text
F(x) = 0.5 * exp(x / b)              , x < 0
F(x) = 1 - 0.5 * exp(-x / b)         , x >= 0
```

令 u 为 [0,1) 上的均匀随机数，其反函数为：

```text
u < 0.5 : x = b * ln(2u)
u >= 0.5: x = -b * ln(2(1-u))
```

对应代码：

```cpp
std::uniform_real_distribution<double> uniform(0.0, 1.0);
const double u = uniform(rng_);

if (u < 0.5) {
  return scale * std::log(2.0 * u);
}
return -scale * std::log(2.0 * (1.0 - u));
```

需要注意，`std::uniform_real_distribution<double>(0.0, 1.0)` 通常不会生成 `0.0`，因此 `std::log(0)` 在正常运行中不会发生；即使生成非常接近 0 的值，也只会产生较大的负噪声，不会使程序崩溃。

---

### 4.3 `prefixCount`

```cpp
int prefixCount(const std::vector<int>& records, int n) {
  if (n <= 0) return 0;
  n = std::min<int>(n, static_cast<int>(records.size()));
  return std::accumulate(records.begin(), records.begin() + n, 0);
}
```

该函数计算数据集前 `n` 行的患病人数。

边界处理：

- 当 `n <= 0` 时，直接返回 0；
- 当 `n` 超过数据集大小时，截断到数据集长度，避免迭代器越界。

在攻击示例中，`count(3)` 表示前 3 行患病数，`count(2)` 表示前 2 行患病数。

---

### 4.4 `printDataset`

该函数打印数据集和前缀计数：

```cpp
std::cout << "序号\t是否患病\t前 n 行患病数 count(n)\n";
int prefix = 0;
for (std::size_t i = 0; i < records.size(); ++i) {
  prefix += records[i];
  std::cout << (i + 1) << "\t" << records[i] << "\t\t" << prefix << "\n";
}
```

`prefix` 逐行累加，用于展示第 `i` 行对应的 `count(i)`。

---

### 4.5 `demonstrateLaplaceMechanism`

该函数是 Laplace 机制的演示核心，分为三个步骤。

#### 第 1 步：无噪声差分攻击

```cpp
const int jackTrueValue = records[static_cast<std::size_t>(jackIndex)];
const int count3 = prefixCount(records, 3);
const int count2 = prefixCount(records, 2);
```

Jack 位于第 3 行，即数组下标 2。

攻击者只知道可以发起 `count(3)` 和 `count(2)` 查询，并计算：

```text
count(3) - count(2)
```

因为数据集前 3 行与前 2 行唯一差异就是第 3 行 Jack，所以：

```text
count(3) - count(2) = records[2] = 1
```

程序直接打印该结果，说明在没有差分隐私时，单个个体的敏感属性会被准确推断出来。

#### 第 2 步：展示 Laplace 加噪结果

```cpp
LaplaceMechanism demoRng(20241107u);
for (int trial = 0; trial < 10; ++trial) {
  const double noisy3 = demoRng.addNoise(count3, 1.0);
  const double noisy2 = demoRng.addNoise(count2, 1.0);
  ...
}
```

这里固定 ε = 1.0，对 `count(3)` 和 `count(2)` 各加一次噪声，重复 10 次。

从输出可以看到：

- `noisy_count(3)` 不再恒等于 2；
- `noisy_count(2)` 不再恒等于 1；
- 攻击者用 `noisy_count(3) - noisy_count(2)` 得到的结果会在较大范围内波动，不再稳定等于 1。

#### 第 3 步：统计不同 ε 下的平均误差

```cpp
const std::vector<double> epsilonList = {0.1, 0.5, 1.0, 2.0};
const int trials = 5000;
```

对每个 ε 重复 5000 次，统计三类平均绝对误差：

1. `count(3)` 加噪结果与真实值的绝对误差；
2. `count(2)` 加噪结果与真实值的绝对误差；
3. 攻击差分误差：

```text
E_attack = | noisy3 - noisy2 - JackTrueValue |
```

这样能够直观看到隐私预算与数据可用性之间的权衡。

---

### 4.6 `demonstrateExponentialMechanism`

该函数演示非数值型候选选择。

```cpp
const std::vector<std::string> candidates = {"足球", "篮球", "排球"};
const std::vector<int> utilities = {12, 10, 7};
const int sensitivity = 1;
```

三个候选的真实票数分别为：

| 候选 | 真实票数 |
| --- | --- |
| 足球 | 12 |
| 篮球 | 10 |
| 排球 | 7 |

这里效用函数 q(D, o) 就是候选项目的票数，敏感度 Δq = 1，因为改变一条投票记录时，某个候选的票数最多变化 1。

对每个 ε，程序计算指数机制权重：

```cpp
weights.push_back(
    std::exp(epsilon * utility / (2.0 * sensitivity)));
```

即：

```text
w_i = exp(ε * q(D, o_i) / (2 * Δq))
```

然后使用 `std::discrete_distribution` 根据这些权重进行抽样：

```cpp
std::discrete_distribution<int> distribution(weights.begin(), weights.end());
```

最后进行 20000 次抽样，用频率估计每个候选被选中的概率。

从输出可以看出：

- 票数最高的“足球”始终拥有最大的选中概率；
- ε 越小，输出分布越接近均匀，越能隐藏真实票数差异；
- ε 越大，输出分布越偏向票数最高的候选，结果越接近真实选择结果。

---

### 4.7 `main`

```cpp
const std::vector<int> records = {
    0, 1, 1, 0, 1, 0, 0, 1, 1, 0,
    0, 1, 0, 1, 1, 0, 1, 0, 0, 1};
const int jackIndex = 2;
```

数据集中：

- Jack 位于下标 2，即第 3 行，值为 1；
- 攻击者可以通过 `count(3) - count(2)` 得到 1。

`main` 的流程为：

1. 打印数据集；
2. 演示无噪声差分攻击与 Laplace 机制；
3. 演示 Exponential 机制。

---

## 5. 实际运行输出

使用以下命令编译并运行：

```bash
clang++ -std=c++17 -O2 InformationSecurity/Experiment/dp_lab.cpp -o /tmp/dp_lab
/tmp/dp_lab
```

程序输出如下：

```text
差分隐私数据保护实验（Laplace + Exponential）
================================================
序号	是否患病	前 n 行患病数 count(n)
1	0		0
2	1		1
3	1		2
4	0		2
5	1		3
6	0		3
7	0		3
8	1		4
9	1		5
10	0		5
11	0		5
12	1		6
13	0		6
14	1		7
15	1		8
16	0		8
17	1		9
18	0		9
19	0		9
20	1		10

========== 1. 无噪声差分攻击 ==========
Jack 位于第 3 行，真实患病值 = 1
攻击者计算 count(3) - count(2) = 2 - 1 = 1
因此，无差分隐私保护时，Jack 的敏感属性被直接推断出来。

========== 2. Laplace 机制加噪后的效果 ==========
示例：epsilon = 1.0 时，count(3) 和 count(2) 的加噪结果。
试验	count(3)	noisy_count(3)	count(2)	noisy_count(2)
1	2	1.904		1	-0.095
2	2	1.225		1	1.517
3	2	2.133		1	1.674
4	2	2.242		1	-1.572
5	2	3.805		1	0.777
6	2	1.795		1	0.993
7	2	3.005		1	3.494
8	2	1.025		1	0.456
9	2	7.532		1	0.178
10	2	1.201		1	0.487

========== 3. 不同 epsilon 下的平均误差 ==========
对每个 epsilon 重复 5000 次，统计平均绝对误差。
epsilon	count(3)误差	count(2)误差	攻击差分误差
0.100	10.1920		10.0059		15.0813
0.5000	2.0384		2.0012		3.0163
1.0000	1.0192		1.0006		1.5081
2.0000	0.5096		0.5003		0.7541

========== 4. 指数机制：非数值候选选择 ==========

epsilon = 0.5000
候选	真实票数	采样次数	估计概率
足球	12		10505		0.525
篮球	10		6386		0.319
排球	7		3109		0.155

epsilon = 1.000
候选	真实票数	采样次数	估计概率
足球	12		13764		0.688
篮球	10		5114		0.256
排球	7		1122		0.056

epsilon = 2.000
候选	真实票数	采样次数	估计概率
足球	12		17446		0.872
篮球	10		2432		0.122
排球	7		122		0.006
```

> 由于随机数种子固定，示例中的加噪结果和采样次数在相同编译环境下可复现。

---

## 6. 结果分析

### 6.1 无噪声攻击为什么成立

数据集第 3 行为 Jack：

```text
records[0] = 0
records[1] = 1
records[2] = 1
```

所以：

```text
count(3) = 0 + 1 + 1 = 2
```

```text
count(2) = 0 + 1 = 1
```

两者相减：

```text
2 - 1 = 1
```

正好泄露 Jack 的患病值。这是差分隐私希望防御的典型场景。

### 6.2 Laplace 机制如何破坏攻击

加入噪声后：

```text
noisy_count(3) = 2 + η1
```

```text
noisy_count(2) = 1 + η2
```

攻击者仍然可以计算：

```text
noisy_count(3) - noisy_count(2) = 1 + (η1 - η2)
```

其中 η1 和 η2 是两个独立 Laplace 噪声。它们的差不再是固定的 0，因此攻击者无法稳定得到 Jack 的真实值。

### 6.3 ε 与误差的关系

Laplace 尺度参数为：

```text
b = 1 / ε
```

从运行结果看：

| ε | `count(3)` 平均误差 | `count(2)` 平均误差 | 攻击差分平均误差 |
| --- | ---: | ---: | ---: |
| 0.1 | 10.1920 | 10.0059 | 15.0813 |
| 0.5 | 2.0384 | 2.0012 | 3.0163 |
| 1.0 | 1.0192 | 1.0006 | 1.5081 |
| 2.0 | 0.5096 | 0.5003 | 0.7541 |

规律很清晰：

- ε 越小，噪声尺度越大，单个查询误差和攻击差分误差都越大，隐私保护越强；
- ε 越大，噪声尺度越小，查询结果越接近真实值，但隐私保护越弱。

这就是差分隐私中的“隐私-效用权衡”。

### 6.4 Exponential 机制的结果分析

指数机制的权重公式为：

```text
w_i = exp(ε * q_i / 2)
```

当 ε = 0.5 时，三个权重分别为：

```text
w_足球 = e^3.0 ≈ 20.09
```

```text
w_篮球 = e^2.5 ≈ 12.18
```

```text
w_排球 = e^1.75 ≈ 5.75
```

归一化后，“足球”的估计概率约为 0.525，“篮球”约为 0.319，“排球”约为 0.155。

当 ε 增大到 2.0 时，指数项差距被放大，“足球”几乎主导输出，估计概率约为 0.872。这说明：

- ε 越小，候选选择越随机，越难通过输出反推真实票数；
- ε 越大，输出越接近效用最高的候选。

---

## 7. 代码中的细节与教学价值

### 7.1 固定随机种子

程序多处使用 `20241107u` 作为随机数种子，这是为了让实验输出可以复现。教学演示中，可复现性比随机性更重要，因为教师和学生可以比较相同输入下的输出。

### 7.2 使用逆变换法生成 Laplace 噪声

代码没有调用第三方统计库，而是直接根据累积分布函数的反函数，从均匀分布生成 Laplace 分布。这样可以帮助理解：

- 如何从 `[0,1)` 均匀分布变换到目标分布；
- Laplace 分布的分位数特性；
- 为什么 `scale = sensitivity / epsilon` 是隐私机制中的关键参数。

### 7.3 默认敏感度参数

`addNoise` 将 `sensitivity` 默认设为 1，是因为 `count(n)` 查询满足：

```text
Δf = 1
```

如果更换成其他查询，例如平均值、求和或其他聚合函数，需要重新计算并传入敏感度。

### 7.4 指数机制用票数作为效用函数

Exponential 机制的经典应用场景是“在多个候选方案中选择一个输出”。这里的“票数”是真实效用值，`std::discrete_distribution` 负责按权重抽样。它展示了：

- 指数机制中的概率比例公式；
- 如何用抽样频率估计真实输出概率；
- ε 如何控制输出的随机程度。

---

## 8. 局限性与进一步讨论

### 8.1 攻击误差不等于隐私保证的完整证明

本实验用“平均绝对误差”衡量攻击难度，能直观展示噪声影响，但差分隐私的正式保证需要从概率分布层面证明，而不是只看平均误差。

### 8.2 多次查询需要组合隐私预算

程序中演示了 `count(3)` 和 `count(2)` 两个查询。实际应用中，如果允许攻击者发起多次查询，需要按差分隐私的组合定理累计隐私预算，否则隐私保护可能被削弱。

### 8.3 输出为实数，原始查询为整数

`count(n)` 的真实值是整数，但 Laplace 机制输出实数。真实发布系统通常还需要做后处理，例如四舍五入、截断到合法范围等。差分隐私具有后处理不变性，这类后处理不会增加隐私风险，但会改变结果的可用性。

### 8.4 固定数据集规模较小

本程序使用 20 条记录，且攻击目标 Jack 的位置固定。它适合教学演示，但不代表真实医疗数据发布系统的安全强度。

### 8.5 Exponential 机制中的敏感度假设

程序将效用函数敏感度设为 1，隐含假设一条记录的变化至多使某个候选票数变化 1。对于真实多选项投票场景，需要结合具体效用函数重新分析敏感度。

---

## 9. 可扩展实验方向

在理解当前程序的基础上，可以继续做以下扩展：

1. **实现 Gaussian 机制**：比较 Gaussian 噪声与 Laplace 噪声在 (ε, δ) 差分隐私下的差异。
2. **实现多种聚合查询**：如 `sum`、`avg`、`max`，并计算各自的全局敏感度。
3. **模拟组合查询攻击**：让攻击者连续发起多个查询，观察隐私预算累加对推断成功率的影响。
4. **加入后处理**：将 Laplace 输出四舍五入为整数，观察误差和可用性变化。
5. **动态修改数据集**：随机改变 Jack 的记录，验证输出分布是否仍然难以区分。
6. **用图表展示隐私-效用权衡**：把不同 ε 下的误差和指数机制概率绘制成曲线。

---

## 10. 小结

`dp_lab.cpp` 用非常小的代码规模，完整演示了差分隐私的两个基础机制：

- 对数值型 `count(n)` 查询，使用 Laplace 机制加噪；
- 对非数值型候选选择，使用 Exponential 机制进行概率抽样；
- 通过无噪声差分攻击展示隐私泄露，通过不同 ε 下的误差展示隐私与效用的权衡。

该程序适合作为信息安全、隐私保护或数据发布课程的入门实验，帮助建立对差分隐私基本概念和实现方式的理解。
