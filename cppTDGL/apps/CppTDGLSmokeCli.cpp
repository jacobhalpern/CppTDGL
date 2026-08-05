#include "Hdf5SolutionReader.hpp"
#include "Layer.hpp"
#include "Mesh.hpp"
#include "Polygon2D.hpp"
#include "SimulationJob.hpp"
#include "SimulationRunner.hpp"
#include "Solution.hpp"
#include "SolverOptions.hpp"

#include <cmath>
#include <filesystem>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

constexpr double kTolerance = 1.0e-12;

void printUsage(const char* executableName) {
    std::cout
        << "Usage: " << executableName << " [--output <path>] [--project-name <name>] [--keep-output]\n"
        << "\n"
        << "Runs a deterministic CppTDGL smoke simulation through SimulationRunner.\n"
        << "If --output is omitted, a temporary scaffold output file is used and removed.\n";
}

bool nearlyEqual(double left, double right) {
    return std::abs(left - right) <= kTolerance;
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

cppTDGL::Device makeDevice(
    const std::string& projectName,
    const std::filesystem::path& outputPath
) {
    cppTDGL::Layer layer(
        0.5,
        2.0,
        0.1,
        1.0,
        1.0,
        "um"
    );

    cppTDGL::SolverOptions options;
    options.solveTime = 1.0;
    options.initialTimeStep = 1.0e-3;
    options.maximumTimeStep = 0.05;
    options.saveEvery = 3;
    options.outputPath = outputPath.string();

    cppTDGL::Polygon2D film = cppTDGL::Polygon2D::rectangle(
        "film",
        10.0,
        4.0
    );

    cppTDGL::Device device(
        projectName,
        layer,
        film,
        options
    );

    device.addProbePoint({0.0, 0.0});
    device.setMesh(makeSquareMesh());

    return device;
}

struct CliOptions {
    std::filesystem::path outputPath =
        std::filesystem::temp_directory_path() / "CppTDGL_CliSmokeOutput.h5";
    std::string projectName = "cppTDGL_cli_smoke";
    bool keepOutput = false;
    bool showHelp = false;
};

CliOptions parseArguments(int argc, char** argv) {
    CliOptions options;

    for (int i = 1; i < argc; ++i) {
        const std::string argument = argv[i];

        if (argument == "--help" || argument == "-h") {
            options.showHelp = true;
            continue;
        }

        if (argument == "--keep-output") {
            options.keepOutput = true;
            continue;
        }

        if (argument == "--output") {
            if (i + 1 >= argc) {
                throw std::invalid_argument("--output requires a path argument.");
            }

            options.outputPath = argv[++i];
            options.keepOutput = true;
            continue;
        }

        if (argument == "--project-name") {
            if (i + 1 >= argc) {
                throw std::invalid_argument("--project-name requires a name argument.");
            }

            options.projectName = argv[++i];
            continue;
        }

        throw std::invalid_argument("Unknown argument: " + argument);
    }

    return options;
}

int runSmokeSimulation(const CliOptions& options) {
    std::filesystem::remove(options.outputPath);

    cppTDGL::Device device = makeDevice(options.projectName, options.outputPath);
    cppTDGL::SimulationJob job("cli-smoke-job", device);

    const cppTDGL::SimulationRunResult result = cppTDGL::SimulationRunner::run(job);

    if (!result.completed()) {
        std::cerr << "Simulation failed.\n";
        for (const std::string& diagnostic : result.diagnostics) {
            std::cerr << "  " << diagnostic << '\n';
        }
        return 2;
    }

    if (!result.wroteOutput || !std::filesystem::exists(options.outputPath)) {
        std::cerr << "Simulation did not create expected output file: "
                  << options.outputPath.string() << '\n';
        return 3;
    }

    const cppTDGL::Solution loaded = cppTDGL::Hdf5SolutionReader::read(options.outputPath);

    if (!loaded.isValid()) {
        std::cerr << "Loaded solution failed validation.\n";
        for (const std::string& error : loaded.validationErrors()) {
            std::cerr << "  " << error << '\n';
        }
        return 4;
    }

    if (!loaded.completed()) {
        std::cerr << "Loaded solution is not marked completed.\n";
        return 5;
    }

    if (loaded.frameCount() != result.solution.frameCount()) {
        std::cerr << "Loaded frame count does not match simulation result.\n";
        return 6;
    }

    if (!nearlyEqual(loaded.latestFrame().time, device.solverOptions().solveTime)) {
        std::cerr << "Loaded final frame time does not match solveTime.\n";
        return 7;
    }

    std::cout << "CppTDGL smoke simulation completed.\n";
    std::cout << "Project: " << loaded.projectName() << '\n';
    std::cout << "Frames: " << loaded.frameCount() << '\n';
    std::cout << "Output: " << options.outputPath.string() << '\n';

    if (!options.keepOutput) {
        std::filesystem::remove(options.outputPath);
    }

    return 0;
}

} // namespace

int main(int argc, char** argv) {
    try {
        const CliOptions options = parseArguments(argc, argv);

        if (options.showHelp) {
            printUsage(argv[0]);
            return 0;
        }

        return runSmokeSimulation(options);
    } catch (const std::exception& error) {
        std::cerr << "CppTDGL smoke CLI error: " << error.what() << '\n';
        printUsage(argv[0]);
        return 1;
    }
}
