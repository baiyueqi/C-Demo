#include "data_generator.h"

#include <random>
#include <stdexcept>

DataGenerator::DataGenerator(unsigned int randomSeed)
    : randomSeed_(randomSeed) {}

WtaInstance DataGenerator::generateInstance(int weaponCount, int targetCount) {
    if (weaponCount <= 0 || targetCount <= 0) {
        throw std::invalid_argument("weapon and target counts must be positive");
    }

    std::mt19937 randomEngine(randomSeed_);
    // 目标威胁值和毁伤概率范围来自 WTA 设计文档。
    std::uniform_real_distribution<double> threatDistribution(25.0, 100.0);
    std::uniform_real_distribution<double> probabilityDistribution(0.60, 0.90);

    WtaInstance instance;
    instance.weaponCount = weaponCount;
    instance.targetCount = targetCount;
    instance.targets.reserve(targetCount);
    instance.killProbability.assign(
        weaponCount,
        std::vector<double>(targetCount, 0.0)
    );

    // 生成每个目标的威胁值 v_j。
    for (int targetIndex = 0; targetIndex < targetCount; ++targetIndex) {
        instance.targets.push_back({targetIndex, threatDistribution(randomEngine)});
    }

    // 生成每个武器对每个目标的毁伤概率 p_ij。
    for (int weaponIndex = 0; weaponIndex < weaponCount; ++weaponIndex) {
        for (int targetIndex = 0; targetIndex < targetCount; ++targetIndex) {
            instance.killProbability[weaponIndex][targetIndex] =
                probabilityDistribution(randomEngine);
        }
    }

    return instance;
}
