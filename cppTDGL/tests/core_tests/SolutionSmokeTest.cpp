#include "EdgeMesh.hpp"
#include "Mesh.hpp"
#include "Solution.hpp"
#include "SolverState.hpp"

#include <cmath>
#include <filesystem>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

constexpr double kTolerance = 1.0e-12;

void failIfFalse(bool condition, const std::string& message) {
    if (!condition) {
        throw std::runtime_error(message);
    }
}

void failIfNotNear(double actual, double expected, const std::string& message) {
    if (std::abs(actual - expected) > kTolerance) {
        throw std::runtime_error(
            message + " Expected " + std::to_string(expected) +
            ", got " + std::to_string(actual) + "."
        );
    }
}

cppTDGL::Mesh makeSquareMesh() {
    const std::vector<cppTDGL::Point2D> vertices{
        {0.0, 0.0},
        {1.0, 0.0},
        {1.0, 1.0},
        {0.0, 1.0},
    };

    const std::vector<cppTDGL::Triangle> triangles{
        {0, 1, 2},
        {0, 2, 3},
    };

    const std::vector<std::size_t> boundaryVertices{
        0, 1, 2, 3,
    };

    return cppTDGL::Mesh(vertices, triangles, boundaryVertices);
}

} // namespace

int main() {
    try {
        cppTDGL::Solution legacy(
            "legacy_solution",
            std::filesystem::path("legacy_solution.h5"),
            3
        );

        failIfFalse(legacy.projectName() == "legacy_solution", "Legacy project name should be preserved.");
        failIfFalse(legacy.hasResultPath(), "Legacy solution should report a result path.");
        failIfFalse(legacy.savedFrameCount() == 3, "Legacy saved frame count should be preserved.");
        failIfFalse(legacy.frameCount() == 0, "Legacy metadata-only solution should not own frames.");
        failIfFalse(!legacy.hasMesh(), "Legacy metadata-only solution should not own a mesh.");
        failIfFalse(legacy.isValid(), "Legacy metadata-only solution should be valid.");

        const cppTDGL::Mesh mesh = makeSquareMesh();
        const cppTDGL::EdgeMesh edgeMesh = cppTDGL::EdgeMesh::fromMesh(mesh);

        cppTDGL::SolverState state = cppTDGL::SolverState::createUniformInitialState(
            mesh,
            0.0,
            1.0,
            0.0
        );

        state.appendFrame(cppTDGL::SolverFrame::zerosFor(state.mesh(), state.edgeMesh(), 0.5));

        cppTDGL::Solution solution = cppTDGL::Solution::fromSolverState(
            "solver_solution",
            state,
            std::filesystem::path("solver_solution.h5")
        );

        failIfFalse(solution.projectName() == "solver_solution", "Solution project name should be set from factory.");
        failIfFalse(solution.hasResultPath(), "Solution should report result path from factory.");
        failIfFalse(solution.hasMesh(), "Solution created from SolverState should own a mesh.");
        failIfFalse(solution.mesh().isValid(), "Solution mesh should be valid.");
        failIfFalse(solution.frameCount() == 2, "Solution should copy SolverState frames.");
        failIfFalse(solution.savedFrameCount() == 2, "Solution saved frame count should reflect owned frames.");
        failIfFalse(solution.completed(), "Solution created from populated SolverState should be marked completed.");
        failIfFalse(solution.isValid(), "Solution created from SolverState should be valid.");
        failIfNotNear(solution.latestFrame().time, 0.5, "Solution latest frame time should round trip from SolverState.");

        solution.setCompleted(false);
        failIfFalse(!solution.completed(), "Solution completed flag should be mutable.");
        solution.setCompleted(true);
        failIfFalse(solution.completed(), "Solution completed flag should be resettable.");

        solution.addDiagnostic("diagnostic one");
        failIfFalse(solution.diagnostics().size() == 1, "Solution diagnostics should store messages.");
        solution.clearDiagnostics();
        failIfFalse(solution.diagnostics().empty(), "Solution diagnostics should clear messages.");

        cppTDGL::SolverFrame appendedFrame = cppTDGL::SolverFrame::uniformOrderParameterFor(
            solution.mesh(),
            edgeMesh,
            1.0,
            0.25,
            0.75
        );

        solution.appendFrame(appendedFrame);

        failIfFalse(solution.frameCount() == 3, "Appending a valid frame should increase frame count.");
        failIfFalse(solution.savedFrameCount() == 3, "Saved frame count should track owned frames after append.");
        failIfNotNear(solution.latestFrame().time, 1.0, "Latest frame should be appended frame.");
        failIfNotNear(solution.frame(2).orderParameter.real.front(), 0.25, "Appended order parameter real component should be preserved.");

        bool outOfRangeThrew = false;
        try {
            static_cast<void>(solution.frame(100));
        } catch (const std::out_of_range&) {
            outOfRangeThrew = true;
        }

        failIfFalse(outOfRangeThrew, "Out-of-range Solution frame access should throw.");

        cppTDGL::Solution noMeshSolution;

        bool appendWithoutMeshThrew = false;
        try {
            noMeshSolution.appendFrame(cppTDGL::SolverFrame::zerosFor(mesh, edgeMesh, 0.0));
        } catch (const std::logic_error&) {
            appendWithoutMeshThrew = true;
        }

        failIfFalse(appendWithoutMeshThrew, "Appending a frame without a Solution mesh should throw.");

        cppTDGL::Solution invalidFrameSolution("invalid_frame_solution", mesh);
        cppTDGL::SolverFrame invalidFrame = cppTDGL::SolverFrame::zerosFor(mesh, edgeMesh, 0.0);
        invalidFrame.scalarPotential.values.pop_back();

        bool invalidFrameThrew = false;
        try {
            invalidFrameSolution.appendFrame(invalidFrame);
        } catch (const std::invalid_argument&) {
            invalidFrameThrew = true;
        }

        failIfFalse(invalidFrameThrew, "Appending an incompatible frame should throw.");

        cppTDGL::Solution clearMeshSolution("clear_mesh_solution", mesh);
        clearMeshSolution.appendFrame(cppTDGL::SolverFrame::zerosFor(mesh, edgeMesh, 0.0));

        bool clearMeshWithFramesThrew = false;
        try {
            clearMeshSolution.clearMesh();
        } catch (const std::logic_error&) {
            clearMeshWithFramesThrew = true;
        }

        failIfFalse(clearMeshWithFramesThrew, "Clearing mesh while frames exist should throw.");

        clearMeshSolution.clearFrames();
        clearMeshSolution.clearMesh();

        failIfFalse(!clearMeshSolution.hasFrames(), "clearFrames should remove frames.");
        failIfFalse(!clearMeshSolution.hasMesh(), "clearMesh should remove mesh after frames are cleared.");
        failIfFalse(clearMeshSolution.isValid(), "Solution should remain valid after clearing frames and mesh.");

        std::cout << "Solution smoke test passed.\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "Solution smoke test exception: " << error.what() << '\n';
        return 1;
    }
}
