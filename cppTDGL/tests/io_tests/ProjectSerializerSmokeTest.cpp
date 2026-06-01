#include "Device.hpp"
#include "Layer.hpp"
#include "Polygon2D.hpp"
#include "ProjectSerializer.hpp"
#include "SolverOptions.hpp"

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

void printErrors(const std::vector<std::string>& errors) {
    for (const std::string& error : errors) {
        std::cerr << "  - " << error << '\n';
    }
}

} // namespace

int main() {
    try {
        cppTDGL::Layer layer(
            0.5,
            2.0,
            0.1,
            1.25,
            1.0,
            "um"
        );

        cppTDGL::SolverOptions options;
        options.solveTime = 20.0;
        options.initialTimeStep = 1.0e-3;
        options.maximumTimeStep = 0.05;
        options.adaptiveTimeStepping = true;
        options.saveEvery = 25;
        options.includeScreening = false;
        options.outputPath = "round_trip_solution.h5";
        options.sparseSolverName = "default";

        cppTDGL::Polygon2D film = cppTDGL::Polygon2D::rectangle(
            "film",
            10.0,
            4.0
        );

        cppTDGL::Device original(
            "serializer_smoke_test",
            layer,
            film,
            options
        );

        original.addHole(cppTDGL::Polygon2D::circleApproximation(
            "center_hole",
            0.5,
            32,
            {0.0, 0.0}
        ));

        original.addTerminal(cppTDGL::Polygon2D::rectangle(
            "left_terminal",
            0.5,
            1.5,
            {-4.75, 0.0}
        ));

        original.addProbePoint({2.0, 0.0});

        const std::vector<std::string> originalErrors = original.validationErrors();
        if (!originalErrors.empty()) {
            std::cerr << "Original device validation failed:\n";
            printErrors(originalErrors);
            return 1;
        }

        const std::filesystem::path projectPath =
            std::filesystem::temp_directory_path() / "CppTDGL_ProjectSerializerSmokeTest.cpptdgl";

        cppTDGL::ProjectSerializer::save(original, projectPath);
        cppTDGL::Device loaded = cppTDGL::ProjectSerializer::load(projectPath);

        failIfFalse(loaded.projectName() == original.projectName(), "Project name should round trip.");

        failIfNotNear(loaded.layer().coherenceLength(), original.layer().coherenceLength(), "Coherence length should round trip.");
        failIfNotNear(loaded.layer().londonLambda(), original.layer().londonLambda(), "London lambda should round trip.");
        failIfNotNear(loaded.layer().thickness(), original.layer().thickness(), "Layer thickness should round trip.");
        failIfNotNear(loaded.layer().conductivity(), original.layer().conductivity(), "Conductivity should round trip.");
        failIfNotNear(loaded.layer().gamma(), original.layer().gamma(), "Gamma should round trip.");
        failIfFalse(loaded.layer().units() == original.layer().units(), "Layer units should round trip.");

        failIfNotNear(loaded.solverOptions().solveTime, original.solverOptions().solveTime, "Solve time should round trip.");
        failIfNotNear(loaded.solverOptions().initialTimeStep, original.solverOptions().initialTimeStep, "Initial time step should round trip.");
        failIfNotNear(loaded.solverOptions().maximumTimeStep, original.solverOptions().maximumTimeStep, "Maximum time step should round trip.");
        failIfFalse(loaded.solverOptions().adaptiveTimeStepping == original.solverOptions().adaptiveTimeStepping, "Adaptive flag should round trip.");
        failIfFalse(loaded.solverOptions().saveEvery == original.solverOptions().saveEvery, "Save interval should round trip.");
        failIfFalse(loaded.solverOptions().includeScreening == original.solverOptions().includeScreening, "Screening flag should round trip.");
        failIfFalse(loaded.solverOptions().outputPath == original.solverOptions().outputPath, "Output path should round trip.");
        failIfFalse(loaded.solverOptions().sparseSolverName == original.solverOptions().sparseSolverName, "Sparse solver name should round trip.");

        failIfFalse(loaded.film().name() == original.film().name(), "Film name should round trip.");
        failIfFalse(loaded.film().vertexCount() == original.film().vertexCount(), "Film vertex count should round trip.");
        failIfNotNear(loaded.film().area(), original.film().area(), "Film area should round trip.");

        failIfFalse(loaded.holes().size() == 1, "Hole count should round trip.");
        failIfFalse(loaded.terminals().size() == 1, "Terminal count should round trip.");
        failIfFalse(loaded.probePoints().size() == 1, "Probe point count should round trip.");

        failIfNotNear(loaded.probePoints()[0].x, 2.0, "Probe x coordinate should round trip.");
        failIfNotNear(loaded.probePoints()[0].y, 0.0, "Probe y coordinate should round trip.");

        const std::vector<std::string> loadedErrors = loaded.validationErrors();
        if (!loadedErrors.empty()) {
            std::cerr << "Loaded device validation failed:\n";
            printErrors(loadedErrors);
            return 1;
        }

        std::filesystem::remove(projectPath);

        std::cout << "Project serializer smoke test passed.\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "Project serializer smoke test exception: " << error.what() << '\n';
        return 1;
    }
}
