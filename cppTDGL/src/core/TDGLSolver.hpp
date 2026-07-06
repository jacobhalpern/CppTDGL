#pragma once

#include "Device.hpp"
#include "SolverOptions.hpp"
#include "SolverState.hpp"

#include <cstddef>
#include <string>
#include <vector>

namespace cppTDGL {

struct TDGLSolverResult {
    SolverState state;
    std::size_t requestedFrameCount = 0;
    bool completed = false;
    std::vector<std::string> diagnostics;

    [[nodiscard]] std::size_t frameCount() const noexcept;
    [[nodiscard]] const SolverFrame& latestFrame() const;
};

class TDGLSolver {
public:
    [[nodiscard]] static SolverState createInitialState(const Device& device);

    [[nodiscard]] static TDGLSolverResult run(const Device& device);

    [[nodiscard]] static TDGLSolverResult run(
        const Device& device,
        const SolverOptions& options
    );

    [[nodiscard]] static std::size_t plannedFrameCount(const SolverOptions& options);

private:
    static void validateDeviceForSolveOrThrow(const Device& device);
    static void validateOptionsForSolveOrThrow(const SolverOptions& options);

    [[nodiscard]] static SolverFrame makeSkeletonFrame(
        const Mesh& mesh,
        const EdgeMesh& edgeMesh,
        double time,
        double normalizedTime
    );
};

} // namespace cppTDGL
