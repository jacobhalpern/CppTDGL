#include "SolverOptions.hpp"

namespace cppTDGL {

bool SolverOptions::isValid() const {
    return validationErrors().empty();
}

std::vector<std::string> SolverOptions::validationErrors() const {
    std::vector<std::string> errors;

    if (solveTime <= 0.0) {
        errors.emplace_back("Solver solve time must be greater than zero.");
    }
    if (initialTimeStep <= 0.0) {
        errors.emplace_back("Solver initial time step must be greater than zero.");
    }
    if (maximumTimeStep < initialTimeStep) {
        errors.emplace_back("Solver maximum time step must be greater than or equal to the initial time step.");
    }
    if (saveEvery <= 0) {
        errors.emplace_back("Solver save interval must be greater than zero.");
    }
    if (outputPath.empty()) {
        errors.emplace_back("Solver output path must not be empty.");
    }
    if (sparseSolverName.empty()) {
        errors.emplace_back("Solver sparse solver name must not be empty.");
    }

    return errors;
}

} // namespace cppTDGL
