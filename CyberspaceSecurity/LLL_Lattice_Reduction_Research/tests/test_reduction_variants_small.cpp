#include "reduction_variants.hpp"

#include <cassert>
#include <iostream>

int main() {
    const lll::Matrix basis = lll::randomIntegerBasis(5, 6, 2027);

    lll::AdvancedReductionConfig config;
    config.lllConfig.delta = 0.99L;
    config.lllConfig.eta = 0.51L;
    config.blockSize = 3;
    config.enumerationRadius = 1;
    config.maxInsertDepth = 3;
    config.maxPasses = 1;
    config.maxCandidates = 256;

    const lll::AdvancedReductionResult miniBkz = lll::reduceMiniBkz(basis, config);
    assert(miniBkz.stats.validReduced);
    assert(miniBkz.stats.candidateCount > 0);
    assert(lll::isLllReduced(miniBkz.reducedBasis, config.lllConfig));

    const lll::AdvancedReductionResult deep = lll::reduceDeepLllPostPass(basis, config);
    assert(deep.stats.validReduced);
    assert(deep.stats.candidateCount > 0);
    assert(lll::isLllReduced(deep.reducedBasis, config.lllConfig));

    const lll::AdvancedReductionResult pot = lll::reducePotLllPostPass(basis, config);
    assert(pot.stats.validReduced);
    assert(pot.stats.candidateCount > 0);
    assert(lll::isLllReduced(pot.reducedBasis, config.lllConfig));

    std::cout << "test_reduction_variants_small passed\n";
    return 0;
}
