#include "experiment_runner.h"

#include <exception>
#include <iostream>
#include <vector>

int main() {
    const std::vector<ExperimentCase> cases = {
        {"WTA2", 10, 10},
        {"WTA6", 50, 50},
        {"WTA11", 100, 100},
        {"WTA12", 200, 200}
    };

    try {
        ExperimentRunner runner("WTA/results");
        runner.run(cases);
        std::cout << "Experiment finished. Results written to WTA/results.\n";
    } catch (const std::exception& exception) {
        std::cerr << "Experiment failed: " << exception.what() << '\n';
        return 1;
    }

    return 0;
}
