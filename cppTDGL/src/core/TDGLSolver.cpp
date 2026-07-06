#include "TDGLSolver.hpp"

#include <algorithm>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace cppTDGL {
namespace {

constexpr std::size_t kMaximumSkeletonFrameCount = 1025;

void appendValidationErrors(
    std::vector<std::string>& destination,
    const std::string& prefix,
    const std::vector<std::string>& source
) {
    for (const std::string& error : source) {
        destination.push_back(prefix + error);
    }
}

} // namespace

std::size_t TDGLSolverResult::frameCount() const noexcept {
    return state.frameCount();
}

const SolverFrame& TDGLSolverResult::latestFrame() const {
    return state.latestFrame();
}

SolverState TDGLSolver::createInitialState(const Device& device) {
    validateDeviceForSolveOrThrow(device);
    return SolverState::createUniformInitialState(device.mesh(), 0.0, 1.0, 0.0);
}

TDGLSolverResult TDGLSolver::run(const Device& device) {
    return run(device, device.solverOptions());
}

TDGLSolverResult TDGLSolver::run(
    const Device& device,
    const SolverOptions& options
) {
    validateDeviceForSolveOrThrow(device);
    validateOptionsForSolveOrThrow(options);

    const Mesh& mesh = device.mesh();
    const EdgeMesh edgeMesh = EdgeMesh::fromMesh(mesh);
    const std::size_t frameCount = plannedFrameCount(options);

    SolverState state(mesh);

    for (std::size_t frameIndex = 0; frameIndex < frameCount; ++frameIndex) {
        const double normalizedTime =
            frameCount <= 1
                ? 0.0
                : static_cast<double>(frameIndex) / static_cast<double>(frameCount - 1);

        const double time = options.solveTime * normalizedTime;

        state.appendFrame(
            makeSkeletonFrame(
                mesh,
                edgeMesh,
                time,
                normalizedTime
            )
        );
    }

    std::vector<std::string> diagnostics;
    diagnostics.emplace_back(
        "TDGLSolver skeleton completed deterministic frame generation without physical TDGL integration."
    );

    return TDGLSolverResult{
        std::move(state),
        frameCount,
        true,
        std::move(diagnostics),
    };
}

std::size_t TDGLSolver::plannedFrameCount(const SolverOptions& options) {
    validateOptionsForSolveOrThrow(options);

    const std::size_t requested = static_cast<std::size_t>(options.saveEvery) + 1;
    return std::clamp<std::size_t>(requested, 2, kMaximumSkeletonFrameCount);
}

void TDGLSolver::validateDeviceForSolveOrThrow(const Device& device) {
    std::vector<std::string> errors;

    appendValidationErrors(errors, "Device: ", device.validationErrors());

    if (!device.hasMesh()) {
        errors.emplace_back("Device must have an attached mesh before solver execution.");
    } else {
        appendValidationErrors(errors, "Device mesh: ", device.mesh().validationErrors());
    }

    if (!errors.empty()) {
        throw std::invalid_argument(errors.front());
    }
}

void TDGLSolver::validateOptionsForSolveOrThrow(const SolverOptions& options) {
    const std::vector<std::string> errors = options.validationErrors();

    if (!errors.empty()) {
        throw std::invalid_argument(errors.front());
    }
}

SolverFrame TDGLSolver::makeSkeletonFrame(
    const Mesh& mesh,
    const EdgeMesh& edgeMesh,
    double time,
    double normalizedTime
) {
    SolverFrame frame = SolverFrame::uniformOrderParameterFor(
        mesh,
        edgeMesh,
        time,
        1.0 - 0.01 * normalizedTime,
        0.05 * normalizedTime
    );

    for (double& value : frame.scalarPotential.values) {
        value = normalizedTime;
    }

    for (std::size_t edgeIndex = 0; edgeIndex < edgeMesh.edgeCount(); ++edgeIndex) {
        const MeshEdge& edge = edgeMesh.edges()[edgeIndex];

        frame.vectorPotential.values[edgeIndex] = normalizedTime * edge.length;
        frame.currentDensity.values[edgeIndex] =
            (1.0 - normalizedTime) * (edge.isBoundary ? 0.5 : 1.0);
    }

    return frame;
}

} // namespace cppTDGL
