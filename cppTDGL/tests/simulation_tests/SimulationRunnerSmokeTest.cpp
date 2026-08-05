#include "Hdf5SolutionReader.hpp"
#include "Layer.hpp"
#include "Mesh.hpp"
#include "Polygon2D.hpp"
#include "SimulationJob.hpp"
#include "SimulationQueue.hpp"
#include "SimulationRunner.hpp"
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

cppTDGL::Device makeDevice(const std::string& projectName, const std::filesystem::path& outputPath) {
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
    options.saveEvery = 4;
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

} // namespace

int main() {
    try {
        const std::filesystem::path outputPath =
            std::filesystem::temp_directory_path() / "CppTDGL_SimulationRunnerSmokeTest.h5";

        std::filesystem::remove(outputPath);

        cppTDGL::Device device = makeDevice(
            "simulation_runner_smoke_test",
            outputPath
        );

        cppTDGL::SimulationJob job("job-001", device);

        failIfFalse(job.className() == "SimulationJob", "SimulationJob class name should be preserved.");
        failIfFalse(job.isPending(), "New SimulationJob should start pending.");
        failIfFalse(job.isValid(), "SimulationJob with valid meshed device should be valid.");
        failIfFalse(cppTDGL::simulationJobStatusName(job.status()) == "Pending", "Pending status name should be readable.");

        cppTDGL::SimulationRunner runner;
        failIfFalse(runner.className() == "SimulationRunner", "SimulationRunner class name should be preserved.");

        const cppTDGL::SimulationRunResult result = cppTDGL::SimulationRunner::run(job);

        failIfFalse(result.completed(), "Simulation run result should complete.");
        failIfFalse(result.status == cppTDGL::SimulationJobStatus::Completed, "Simulation run status should be completed.");
        failIfFalse(result.wroteOutput, "Simulation run should write scaffold output.");
        failIfFalse(std::filesystem::exists(outputPath), "Simulation output file should exist.");
        failIfFalse(result.solution.isValid(), "Simulation result solution should be valid.");
        failIfFalse(result.solution.completed(), "Simulation result solution should be marked complete.");
        failIfFalse(result.solution.hasMesh(), "Simulation result solution should preserve mesh.");
        failIfFalse(result.solution.frameCount() == 5, "Simulation result should contain saveEvery + 1 frames.");
        failIfFalse(!result.diagnostics.empty(), "Simulation result should include diagnostics.");
        failIfNotNear(result.solution.latestFrame().time, 2.0, "Final simulation frame time should equal solveTime.");

        const cppTDGL::Solution loaded = cppTDGL::Hdf5SolutionReader::read(outputPath);

        failIfFalse(loaded.isValid(), "Loaded simulation solution should be valid.");
        failIfFalse(loaded.completed(), "Loaded simulation solution should be complete.");
        failIfFalse(loaded.frameCount() == result.solution.frameCount(), "Loaded solution frame count should match written solution.");
        failIfFalse(loaded.projectName() == result.solution.projectName(), "Loaded solution project name should match written solution.");
        failIfNotNear(loaded.latestFrame().time, 2.0, "Loaded final frame time should match solveTime.");

        const std::filesystem::path queueOutputA =
            std::filesystem::temp_directory_path() / "CppTDGL_SimulationQueueSmokeTest_A.h5";
        const std::filesystem::path queueOutputB =
            std::filesystem::temp_directory_path() / "CppTDGL_SimulationQueueSmokeTest_B.h5";

        std::filesystem::remove(queueOutputA);
        std::filesystem::remove(queueOutputB);

        cppTDGL::SimulationQueue queue;
        failIfFalse(queue.className() == "SimulationQueue", "SimulationQueue class name should be preserved.");
        failIfFalse(queue.empty(), "New SimulationQueue should be empty.");

        queue.enqueue(cppTDGL::SimulationJob("queue-job-a", makeDevice("queue_project_a", queueOutputA)));
        queue.enqueue(cppTDGL::SimulationJob("queue-job-b", makeDevice("queue_project_b", queueOutputB)));

        failIfFalse(queue.size() == 2, "SimulationQueue should report enqueued job count.");
        failIfFalse(queue.front().jobId() == "queue-job-a", "SimulationQueue front should be first enqueued job.");

        const std::vector<cppTDGL::SimulationRunResult> queueResults = queue.runAll();

        failIfFalse(queue.empty(), "SimulationQueue should be empty after runAll().");
        failIfFalse(queueResults.size() == 2, "SimulationQueue runAll() should return one result per job.");
        failIfFalse(queueResults[0].completed(), "First queued simulation should complete.");
        failIfFalse(queueResults[1].completed(), "Second queued simulation should complete.");
        failIfFalse(std::filesystem::exists(queueOutputA), "First queued simulation output should exist.");
        failIfFalse(std::filesystem::exists(queueOutputB), "Second queued simulation output should exist.");

        cppTDGL::Device invalidDevice = makeDevice(
            "invalid_no_mesh_project",
            std::filesystem::temp_directory_path() / "CppTDGL_InvalidSimulationShouldNotWrite.h5"
        );
        invalidDevice.clearMesh();

        const cppTDGL::SimulationRunResult failedResult = cppTDGL::SimulationRunner::run(
            cppTDGL::SimulationJob("invalid-job", invalidDevice)
        );

        failIfFalse(failedResult.failed(), "SimulationRunner should return failed status for invalid jobs.");
        failIfFalse(!failedResult.wroteOutput, "Failed simulation should not write output.");
        failIfFalse(!failedResult.diagnostics.empty(), "Failed simulation should include diagnostics.");

        std::filesystem::remove(outputPath);
        std::filesystem::remove(queueOutputA);
        std::filesystem::remove(queueOutputB);

        std::cout << "Simulation runner smoke test passed.\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "Simulation runner smoke test exception: " << error.what() << '\n';
        return 1;
    }
}
