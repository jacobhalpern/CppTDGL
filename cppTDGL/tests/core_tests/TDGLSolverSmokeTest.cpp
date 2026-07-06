#include "Device.hpp"
#include "Layer.hpp"
#include "MeshGenerator.hpp"
#include "Polygon2D.hpp"
#include "SolverOptions.hpp"
#include "TDGLSolver.hpp"

#include <cmath>
#include <iostream>
#include <stdexcept>
#include <string>

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

cppTDGL::Device makeDevice(bool attachMesh) {
    cppTDGL::Layer layer(
        0.5,
        2.0,
        0.1,
        1.0,
        1.0,
        "um"
    );

    cppTDGL::SolverOptions options;
    options.solveTime = 2.0;
    options.initialTimeStep = 1.0e-3;
    options.maximumTimeStep = 0.05;
    options.adaptiveTimeStepping = true;
    options.saveEvery = 4;
    options.includeScreening = false;
    options.outputPath = "tdgl_solver_skeleton_solution.h5";
    options.sparseSolverName = "skeleton";

    cppTDGL::Polygon2D film = cppTDGL::Polygon2D::rectangle(
        "film",
        10.0,
        4.0
    );

    cppTDGL::Device device(
        "tdgl_solver_skeleton_smoke_test",
        layer,
        film,
        options
    );

    device.addProbePoint({0.0, 0.0});

    if (attachMesh) {
        cppTDGL::MeshGenerationOptions meshOptions;
        meshOptions.xDivisions = 2;
        meshOptions.yDivisions = 1;
        device = cppTDGL::MeshGenerator::attachGeneratedFilmBoundingBoxMesh(device, meshOptions);
    }

    return device;
}

} // namespace

int main() {
    try {
        const cppTDGL::Device device = makeDevice(true);

        failIfFalse(device.isValid(), "Meshed device should be valid before solver execution.");
        failIfFalse(device.hasMesh(), "Meshed device should have an attached mesh.");
        failIfFalse(device.mesh().vertexCount() == 6, "Generated 2x1 device mesh should have 6 vertices.");

        const std::size_t plannedFrameCount =
            cppTDGL::TDGLSolver::plannedFrameCount(device.solverOptions());

        failIfFalse(plannedFrameCount == 5, "saveEvery=4 should produce 5 skeleton frames.");

        cppTDGL::SolverState initialState = cppTDGL::TDGLSolver::createInitialState(device);

        failIfFalse(initialState.isValid(), "Initial solver state should be valid.");
        failIfFalse(initialState.frameCount() == 1, "Initial solver state should contain one frame.");
        failIfNotNear(initialState.latestFrame().time, 0.0, "Initial solver frame time should be zero.");
        failIfNotNear(initialState.latestFrame().orderParameter.real.front(), 1.0, "Initial order parameter real value should be 1.");
        failIfNotNear(initialState.latestFrame().orderParameter.imaginary.front(), 0.0, "Initial order parameter imaginary value should be 0.");

        const cppTDGL::TDGLSolverResult result = cppTDGL::TDGLSolver::run(device);

        failIfFalse(result.completed, "TDGL solver skeleton result should report completion.");
        failIfFalse(!result.diagnostics.empty(), "TDGL solver skeleton should provide a diagnostic message.");
        failIfFalse(result.requestedFrameCount == 5, "TDGL solver skeleton requested frame count should be 5.");
        failIfFalse(result.frameCount() == 5, "TDGL solver skeleton should generate 5 frames.");
        failIfFalse(result.state.isValid(), "TDGL solver skeleton state should be valid.");

        const cppTDGL::SolverFrame& firstFrame = result.state.frames().front();
        const cppTDGL::SolverFrame& latestFrame = result.latestFrame();

        failIfNotNear(firstFrame.time, 0.0, "First solver frame time should be zero.");
        failIfNotNear(latestFrame.time, 2.0, "Latest solver frame time should equal solveTime.");

        failIfNotNear(firstFrame.orderParameter.real.front(), 1.0, "First frame order parameter real value should be 1.");
        failIfNotNear(latestFrame.orderParameter.real.front(), 0.99, "Latest frame order parameter real value should follow skeleton update.");
        failIfNotNear(latestFrame.orderParameter.imaginary.front(), 0.05, "Latest frame imaginary value should follow skeleton update.");

        failIfNotNear(firstFrame.scalarPotential.values.front(), 0.0, "First frame scalar potential should be zero.");
        failIfNotNear(latestFrame.scalarPotential.values.front(), 1.0, "Latest frame scalar potential should be normalized time 1.");

        failIfFalse(latestFrame.vectorPotential.size() == result.state.edgeMesh().edgeCount(), "Vector potential should be edge-sized.");
        failIfFalse(latestFrame.currentDensity.size() == result.state.edgeMesh().edgeCount(), "Current density should be edge-sized.");

        for (double value : latestFrame.currentDensity.values) {
            failIfNotNear(value, 0.0, "Latest frame current density should decay to zero in skeleton model.");
        }

        cppTDGL::Device deviceWithoutMesh = makeDevice(false);

        bool missingMeshThrew = false;
        try {
            static_cast<void>(cppTDGL::TDGLSolver::run(deviceWithoutMesh));
        } catch (const std::invalid_argument&) {
            missingMeshThrew = true;
        }

        failIfFalse(missingMeshThrew, "Running solver without an attached mesh should throw.");

        cppTDGL::SolverOptions invalidOptions = device.solverOptions();
        invalidOptions.saveEvery = 0;

        bool invalidOptionsThrew = false;
        try {
            static_cast<void>(cppTDGL::TDGLSolver::run(device, invalidOptions));
        } catch (const std::invalid_argument&) {
            invalidOptionsThrew = true;
        }

        failIfFalse(invalidOptionsThrew, "Running solver with invalid SolverOptions should throw.");

        std::cout << "TDGL solver skeleton smoke test passed.\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "TDGL solver skeleton smoke test exception: " << error.what() << '\n';
        return 1;
    }
}
