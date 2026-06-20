#include "lll.hpp"

#include <cassert>
#include <iostream>

int main() {
    lll::LllConfig config;
    config.delta = 0.99L;
    config.eta = 0.51L;
    config.updateStrategy = lll::GsoUpdateStrategy::Suffix;

    const lll::Matrix smallBasis = {
        {4, 1},
        {7, 2},
    };
    const lll::LllResult smallResult = lll::reduceLll(smallBasis, config);
    assert(smallResult.stats.validReduced);
    assert(lll::isLllReduced(smallResult.reducedBasis, config));
    assert(smallResult.stats.dimension == 2);
    assert(smallResult.stats.determinantEstimate > 0.0L);

    const lll::Matrix randomBasis = lll::randomIntegerBasis(6, 8, 2026);
    const lll::LllResult randomResult = lll::reduceLll(randomBasis, config);
    assert(randomResult.stats.validReduced);
    assert(randomResult.stats.rootHermiteFactor > 0.0L);
    assert(randomResult.stats.orthogonalityDefect >= 1.0L);

    config.updateStrategy = lll::GsoUpdateStrategy::Full;
    const lll::LllResult fullResult = lll::reduceLll(randomBasis, config);
    assert(fullResult.stats.validReduced);
    assert(lll::isLllReduced(fullResult.reducedBasis, config));

    std::cout << "test_lll_small passed\n";
    return 0;
}
