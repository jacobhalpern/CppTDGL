#pragma once

#include <string>
#include <vector>

namespace cppTDGL {

class SolverOptions {
public:
    double solveTime = 100.0;
    double initialTimeStep = 1.0e-3;
    double maximumTimeStep = 0.1;
    bool adaptiveTimeStepping = true;
    int saveEvery = 100;
    bool includeScreening = false;
    std::string outputPath = "solution.h5";
    std::string sparseSolverName = "default";

    [[nodiscard]] bool isValid() const;
    [[nodiscard]] std::vector<std::string> validationErrors() const;
};

} // namespace cppTDGL
