#include "SimulationRunner.hpp"

#include "Hdf5SolutionWriter.hpp"
#include "TDGLSolver.hpp"

#include <exception>
#include <stdexcept>
#include <string>
#include <utility>

namespace cppTDGL {
namespace {

void appendDiagnostics(
    std::vector<std::string>& destination,
    const std::vector<std::string>& source
) {
    destination.insert(destination.end(), source.begin(), source.end());
}

} // namespace

bool SimulationRunResult::completed() const noexcept {
    return status == SimulationJobStatus::Completed;
}

bool SimulationRunResult::failed() const noexcept {
    return status == SimulationJobStatus::Failed;
}

const std::string& SimulationRunResult::primaryDiagnostic() const {
    if (diagnostics.empty()) {
        throw std::logic_error("SimulationRunResult has no diagnostics.");
    }

    return diagnostics.front();
}

std::string SimulationRunner::className() const {
    return "SimulationRunner";
}

SimulationRunResult SimulationRunner::run(Device device, std::string jobId) {
    return run(SimulationJob(std::move(jobId), std::move(device)));
}

SimulationRunResult SimulationRunner::run(const SimulationJob& job) {
    SimulationRunResult result;
    result.jobId = job.jobId();
    result.status = SimulationJobStatus::Running;

    const std::vector<std::string> jobErrors = job.validationErrors();
    if (!jobErrors.empty()) {
        result.status = SimulationJobStatus::Failed;
        appendDiagnostics(result.diagnostics, jobErrors);
        return result;
    }

    try {
        const TDGLSolverResult solverResult = TDGLSolver::run(job.device());
        result.outputPath = job.device().solverOptions().outputPath;
        result.solution = buildSolution(job.device(), solverResult);

        Hdf5SolutionWriter::write(result.solution, result.outputPath);
        result.wroteOutput = true;
        result.status = SimulationJobStatus::Completed;
        result.diagnostics = result.solution.diagnostics();
    } catch (const std::exception& error) {
        result.status = SimulationJobStatus::Failed;
        result.diagnostics.push_back(error.what());
    }

    return result;
}

Solution SimulationRunner::buildSolution(
    const Device& device,
    const TDGLSolverResult& solverResult
) {
    Solution solution = Solution::fromSolverState(
        device.projectName(),
        solverResult.state,
        device.solverOptions().outputPath
    );

    solution.setCompleted(solverResult.completed);

    for (const std::string& diagnostic : solverResult.diagnostics) {
        solution.addDiagnostic(diagnostic);
    }

    solution.addDiagnostic("SimulationRunner completed job execution and wrote scaffold solution output.");

    return solution;
}

} // namespace cppTDGL
