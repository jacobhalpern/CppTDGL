#pragma once

#include "Device.hpp"
#include "SimulationJob.hpp"
#include "Solution.hpp"
#include "TDGLSolver.hpp"

#include <filesystem>
#include <string>
#include <vector>

namespace cppTDGL {

struct SimulationRunResult {
    std::string jobId;
    SimulationJobStatus status = SimulationJobStatus::Pending;
    Solution solution;
    std::filesystem::path outputPath;
    bool wroteOutput = false;
    std::vector<std::string> diagnostics;

    [[nodiscard]] bool completed() const noexcept;
    [[nodiscard]] bool failed() const noexcept;
    [[nodiscard]] const std::string& primaryDiagnostic() const;
};

class SimulationRunner {
public:
    SimulationRunner() = default;
    virtual ~SimulationRunner() = default;

    [[nodiscard]] std::string className() const;

    [[nodiscard]] static SimulationRunResult run(const SimulationJob& job);
    [[nodiscard]] static SimulationRunResult run(Device device, std::string jobId = "direct-run");

private:
    [[nodiscard]] static Solution buildSolution(
        const Device& device,
        const TDGLSolverResult& solverResult
    );
};

} // namespace cppTDGL
