#ifndef DATA_GENERATOR_H
#define DATA_GENERATOR_H

#include "wta_instance.h"

// 按设计文档的数据范围生成可复现实验实例。
class DataGenerator {
public:
    // randomSeed 固定后，同一规模会生成相同数据，方便复现实验结果。
    explicit DataGenerator(unsigned int randomSeed);

    // 生成一个 weaponCount x targetCount 的 WTA 实例。
    WtaInstance generateInstance(int weaponCount, int targetCount);

private:
    unsigned int randomSeed_;
};

#endif
