#include "babai.hpp"

#include <cassert>
#include <cmath>
#include <iostream>

int main() {
    const lll::Matrix basis = {
        {2, 0},
        {0, 2},
    };
    const lll::RealVector target = {3.1L, -2.7L};

    const lll::BabaiResult babai = lll::babaiNearestPlane(basis, target);
    assert(babai.latticeVector.size() == 2);
    assert(babai.latticeVector[0] == 4);
    assert(babai.latticeVector[1] == -2);
    assert(babai.candidateCount == 1);

    const lll::BabaiResult exact = lll::exactClosestByEnumeration(basis, target, 3);
    assert(exact.latticeVector == babai.latticeVector);
    assert(std::fabs(static_cast<double>(exact.distance - babai.distance)) < 1e-12);
    assert(exact.candidateCount == 49);

    lll::RandomizedBabaiConfig randomConfig;
    randomConfig.rounds = 16;
    randomConfig.seed = 7;
    const lll::BabaiResult randomized =
        lll::randomizedBabaiNearestPlane(basis, target, randomConfig);
    assert(randomized.candidateCount == 16);
    assert(randomized.distance >= 0.0L);

    lll::TailEnumerationConfig tailConfig;
    tailConfig.tailBlockSize = 2;
    tailConfig.windowRadius = 1;
    tailConfig.maxCandidates = 9;
    const lll::BabaiResult tail = lll::tailEnumerationBabai(basis, target, tailConfig);
    assert(tail.latticeVector == exact.latticeVector);
    assert(tail.candidateCount == 9);

    lll::LocalSearchConfig localConfig;
    localConfig.searchVectorCount = 2;
    localConfig.windowRadius = 1;
    localConfig.maxCandidates = 9;
    const lll::BabaiResult local = lll::localSearchBabai(basis, target, localConfig);
    assert(local.latticeVector == exact.latticeVector);
    assert(local.candidateCount == 9);

    std::cout << "test_babai_small passed\n";
    return 0;
}
