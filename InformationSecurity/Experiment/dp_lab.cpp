#include <algorithm>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <random>
#include <string>
#include <vector>

// 本程序用于演示差分隐私中的两个基础机制：
//   1. Laplace 机制：对数值型查询结果添加 Laplace 噪声；
//   2. Exponential 机制：对非数值型候选结果按效用函数进行概率抽样。
//
// 实验数据集模拟一个医疗数据表，每行表示一个患者是否患病。
// 通过 count(n) 查询说明：
//   - 不加噪声时，攻击者可以用 count(3) - count(2) 推断单个个体的敏感属性；
//   - 加入 Laplace 噪声后，攻击者无法稳定获得该属性值。
namespace {

// LaplaceMechanism 封装 Laplace 噪声的生成和“真实值 + 噪声”的查询过程。
//
// Laplace(0, b) 的概率密度函数为：
//   f(x) = (1 / (2b)) * exp(-|x| / b)
//
// 在差分隐私中，位置参数 mu 取 0，尺度参数 b 取：
//   b = sensitivity / epsilon
//
// 其中：
//   - sensitivity：查询函数的全局敏感度；
//   - epsilon：隐私预算。
class LaplaceMechanism {
 public:
  // 使用固定种子初始化随机数发生器，保证程序多次运行时结果可复现。
  explicit LaplaceMechanism(unsigned int seed) : rng_(seed) {}

  // 对真实查询结果 value 添加差分隐私噪声。
  //
  // sensitivity 默认为 1，因为 count(n) 查询改变任意一条记录时，
  // 查询结果最多变化 1。
  double addNoise(double value, double epsilon, double sensitivity = 1.0) {
    // Laplace 机制的噪声尺度：
    //   scale = sensitivity / epsilon
    //
    // epsilon 越小，scale 越大，噪声越大，隐私保护越强；
    // epsilon 越大，scale 越小，查询结果越接近真实值。
      const double  scalb= sensitivity/epsilon;
      return value+noise(scalb);            
  }

 private:
  // Mersenne Twister 伪随机数发生器。
  std::mt19937 rng_;

  // 使用逆变换法生成服从 Laplace(0, scale) 的随机噪声。
  //
  // Laplace(0, scale) 的累积分布函数为：
  //   F(x) = 0.5 * exp(x / scale)        , x < 0
  //   F(x) = 1 - 0.5 * exp(-x / scale)   , x >= 0
  //
  // 反函数为：
  //   u < 0.5 时，x = scale * ln(2u)
  //   u >= 0.5 时，x = -scale * ln(2(1 - u))
  double noise(double scale) {
    // 首先生成一个 [0, 1) 区间内的均匀分布随机数。
    std::uniform_real_distribution<double> uniform(0.0, 1.0);
    const double u = uniform(rng_);

    // 根据 Laplace 分布的逆累积分布函数，把均匀随机数变换为 Laplace 噪声。
    if (u < 0.5) {
      return scale * std::log(2.0 * u);
    }
    return -scale * std::log(2.0 * (1.0 - u));
  }
};

// 计算医疗数据集 records 中前 n 行的患病人数，即 count(n)。
//
// n 表示查询前几行。如果 n 超过数据集长度，则只统计实际存在的记录数。
int prefixCount(const std::vector<int>& records, int n) {
  if (n <= 0) {
    return 0;
  }

  // 避免 n 越界，确保后续 accumulate 使用的迭代器范围合法。
  n = std::min<int>(n, static_cast<int>(records.size()));

  // 将 [begin, begin + n) 范围内的记录求和，得到 count(n)。
  return std::accumulate(records.begin(), records.begin() + n, 0);
}

// 打印医疗数据集的原始内容，便于观察每条记录以及前缀计数 count(n)。
void printDataset(const std::vector<int>& records) {
  std::cout << "序号\t是否患病\t前 n 行患病数 count(n)\n";

  // prefix 用于逐步累加前 i 行的患病记录数。
  int prefix = 0;
  for (std::size_t i = 0; i < records.size(); ++i) {
    prefix += records[i];
    std::cout << (i + 1) << "\t" << records[i] << "\t\t" << prefix << "\n";
  }
}

// 演示无差分隐私时的攻击，以及 Laplace 机制对 count(n) 查询的保护效果。
//
// 攻击原理：
//   Jack 位于第 3 行，其患病状态为 records[jackIndex]。
//   count(3) 是前 3 行的患病人数，count(2) 是前 2 行的患病人数。
//   因此 count(3) - count(2) 正好等于第 3 行 Jack 的患病状态。
//
// 加入 Laplace 噪声后，count(3) 和 count(2) 都变为随机实数，
// 攻击者无法再通过简单的减法稳定推断出 Jack 的 0/1 属性。
void demonstrateLaplaceMechanism(const std::vector<int>& records,
                                 int jackIndex) {
  // Jack 的真实敏感属性值。
  const int jackTrueValue = records[static_cast<std::size_t>(jackIndex)];

  // 攻击者能够获得的两个查询结果。
  const int count3 = prefixCount(records, 3);
  const int count2 = prefixCount(records, 2);

  // 第一步：展示无噪声时，差分攻击如何直接泄露单个个体信息。
  std::cout << "\n========== 1. 无噪声差分攻击 ==========\n";
  std::cout << "Jack 位于第 3 行，真实患病值 = " << jackTrueValue << "\n";
  std::cout << "攻击者计算 count(3) - count(2) = " << count3 << " - "
            << count2 << " = " << (count3 - count2) << "\n";
  std::cout << "因此，无差分隐私保护时，Jack 的敏感属性被直接推断出来。\n";

  // 第二步：展示 epsilon = 1.0 时，Laplace 机制生成的若干组加噪查询结果。
  std::cout << "\n========== 2. Laplace 机制加噪后的效果 ==========\n";
  std::cout << "示例：epsilon = 1.0 时，count(3) 和 count(2) 的加噪结果。\n";

  // 使用固定种子创建演示用的 LaplaceMechanism，保证示例输出可复现。
  LaplaceMechanism demoRng(20241107u);
  std::cout << "试验\tcount(3)\tnoisy_count(3)\tcount(2)\tnoisy_count(2)\n";

  for (int trial = 0; trial < 10; ++trial) {
    // 分别对两个真实计数查询结果添加 Laplace 噪声。
    const double noisy3 = demoRng.addNoise(static_cast<double>(count3), 1.0);
    const double noisy2 = demoRng.addNoise(static_cast<double>(count2), 1.0);

    std::cout << (trial + 1) << "\t" << count3 << "\t" << std::fixed
              << std::setprecision(3) << noisy3 << "\t\t" << count2 << "\t"
              << noisy2 << "\n";
  }

  // 第三步：比较不同 epsilon 下的平均误差，观察隐私保护和数据可用性的权衡。
  std::cout << "\n========== 3. 不同 epsilon 下的平均误差 ==========\n";
  std::cout << "对每个 epsilon 重复 5000 次，统计平均绝对误差。\n";
  std::cout << "epsilon\tcount(3)误差\tcount(2)误差\t攻击差分误差\n";

  // 测试不同的隐私预算。
  const std::vector<double> epsilonList = {0.1, 0.5, 1.0, 2.0};

  // 每个 epsilon 重复运行的次数。重复次数越多，平均误差越稳定。
  const int trials = 5000;

  for (double epsilon : epsilonList) {
    // 每个 epsilon 使用同一个种子，保证不同 epsilon 之间只改变噪声尺度。
    LaplaceMechanism mech(20241107u);

    // 分别累加三个误差：
    //   error3       : count(3) 加噪结果与真实值的绝对误差；
    //   error2       : count(2) 加噪结果与真实值的绝对误差；
    //   errorAttack  : 攻击者使用加噪结果计算 count(3) - count(2) 的误差。
    double error3 = 0.0;
    double error2 = 0.0;
    double errorAttack = 0.0;

    for (int trial = 0; trial < trials; ++trial) {
      // 对 count(3) 和 count(2) 分别添加噪声。
      const double noisy3 = mech.addNoise(static_cast<double>(count3), epsilon);
      const double noisy2 = mech.addNoise(static_cast<double>(count2), epsilon);

      // 统计本次实验的绝对误差。
      error3 += std::fabs(noisy3 - count3);
      error2 += std::fabs(noisy2 - count2);

      // 攻击者仍然尝试计算 noisy3 - noisy2，并比较它与 Jack 真实值的差距。
      errorAttack += std::fabs((noisy3 - noisy2) - jackTrueValue);
    }

    // 输出该 epsilon 下的平均绝对误差。
    std::cout << epsilon << "\t" << std::fixed << std::setprecision(4)
              << error3 / trials << "\t\t" << error2 / trials << "\t\t"
              << errorAttack / trials << "\n";
  }
}

// 演示 Exponential 机制如何对非数值型候选结果进行概率选择。
//
// 指数机制适用于“方案选择”这类不能直接做数值加法的查询结果。
// 设候选集合为 O，候选为 o，效用函数为 q(D, o)，敏感度为 Δq，
// 则输出 o 的概率正比于：
//   exp(epsilon * q(D, o) / (2 * Δq))
void demonstrateExponentialMechanism() {
  std::cout << "\n========== 4. 指数机制：非数值候选选择 ==========\n";

  // 三个非数值型候选项目。
  const std::vector<std::string> candidates = {"足球", "篮球", "排球"};

  // 每个候选项目的真实票数，作为效用函数 q(D, o) 的取值。
  const std::vector<int> utilities = {12, 10, 7};

  // 效用函数的敏感度。这里改变一条投票记录，某个候选的票数最多变化 1。
  const int sensitivity = 1;

  // 比较不同 epsilon 对输出概率分布的影响。
  const std::vector<double> epsilonList = {0.5, 1.0, 2.0};

  // 每个 epsilon 下的抽样次数，用于估计真实概率分布。
  const int samples = 20000;

  for (double epsilon : epsilonList) {
    // 为不同 epsilon 使用略有差异的种子，便于观察不同参数下的抽样结果。
    std::mt19937 rng(20241107u + static_cast<unsigned int>(epsilon * 100));

    // weights[i] 表示候选 i 被选中的相对权重，对应指数机制中的概率比例。
    std::vector<double> weights;
    weights.reserve(utilities.size());

    for (int utility : utilities) {
      // 指数机制的权重公式：
      //   weight_i = exp(epsilon * utility_i / (2 * sensitivity))
      weights.push_back(
          std::exp(epsilon * static_cast<double>(utility) /
                   (2.0 * static_cast<double>(sensitivity))));
    }

    // std::discrete_distribution 会根据 weights 中的相对权重进行抽样。
    std::discrete_distribution<int> distribution(weights.begin(),
                                                 weights.end());

    // 统计每个候选被抽中的次数。
    std::vector<int> counts(candidates.size(), 0);
    for (int i = 0; i < samples; ++i) {
      counts[static_cast<std::size_t>(distribution(rng))]++;
    }

    // 输出当前 epsilon 下各候选的估计概率。
    std::cout << "\nepsilon = " << epsilon << "\n";
    std::cout << "候选\t真实票数\t采样次数\t估计概率\n";
    for (std::size_t i = 0; i < candidates.size(); ++i) {
      // 用抽样次数除以总抽样次数，估计真实选择概率。
      const double probability =
          static_cast<double>(counts[i]) / static_cast<double>(samples);

      std::cout << candidates[i] << "\t" << utilities[i] << "\t\t" << counts[i]
                << "\t\t" << std::fixed << std::setprecision(3) << probability
                << "\n";
    }
  }
}

}  // namespace

int main() {
  std::cout << "差分隐私数据保护实验（Laplace + Exponential）\n";
  std::cout << "================================================\n";

  // 构造一个包含 20 条记录的医疗数据集：
  //   - 1 表示该患者患病；
  //   - 0 表示该患者未患病。
  //
  // Jack 放在第 3 行，对应数组下标 2，值为 1。
  // 无差分隐私保护时，攻击者通过 count(3) - count(2) 可以得到 1。
  const std::vector<int> records = {
      0, 1, 1, 0, 1, 0, 0, 1, 1, 0,
      0, 1, 0, 1, 1, 0, 1, 0, 0, 1};

  // Jack 在 records 中的下标，对应第 3 行。
  const int jackIndex = 2;

  // 1. 打印原始医疗数据集，展示 count(n) 查询的物理含义。
  printDataset(records);

  // 2. 演示无噪声差分攻击和 Laplace 机制保护效果。
  demonstrateLaplaceMechanism(records, jackIndex);

  // 3. 演示 Exponential 机制在非数值型候选选择中的应用。
  demonstrateExponentialMechanism();

  return 0;
}
