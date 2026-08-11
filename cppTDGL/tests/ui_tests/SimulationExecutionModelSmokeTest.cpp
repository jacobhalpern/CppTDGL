#include "Device.hpp"
#include "Hdf5SolutionReader.hpp"
#include "Layer.hpp"
#include "MeshGenerator.hpp"
#include "Polygon2D.hpp"
#include "SimulationExecutionModel.hpp"
#include "SolverOptions.hpp"

#include <filesystem>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

void failIfFalse(bool condition, const std::string& message) {
    if (!condition) {
        throw std::runtime_error(message);
    }
}

cppTDGL::Device makeDevice(
    const std::string& projectName,
    const std::filesystem::path& outputPath,
    bool attachMesh
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
    options.solveTime = 0.3;
    options.initialTimeStep = 0.1;
    options.maximumTimeStep = 0.1;
    options.adaptiveTimeStepping = false;
    options.saveEvery = 3;
    options.includeScreening = false;
    options.outputPath = outputPath.string();
    options.sparseSolverName = "ui-simulation-execution-solver";

    cppTDGL::Device device(
        projectName,
        layer,
        cppTDGL::Polygon2D::rectangle("film", 4.0, 2.0),
        options
    );

    device.addProbePoint({0.0, 0.0});

    if (attachMesh) {
        cppTDGL::MeshGenerationOptions meshOptions;
        meshOptions.xDivisions = 2;
        meshOptions.yDivisions = 1;
        device = cppTDGL::MeshGenerator::attachGeneratedFilmBoundingBoxMesh(
            device,
            meshOptions
        );
    }

    return device;
}

bool containsText(const std::string& value, const std::string& token) {
    return value.find(token) != std::string::npos;
}

} // namespace

int main() {
    try {
        const std::filesystem::path outputPath =
            std::filesystem::temp_directory_path() / "CppTDGL_SimulationExecutionModelSmokeTest.h5";

        std::filesystem::remove(outputPath);

        cppTDGL::SimulationExecutionModel emptyModel;

        failIfFalse(emptyModel.isIdle(), "Default execution model should start idle.");
        failIfFalse(!emptyModel.hasDevice(), "Default execution model should not have a Device.");
        failIfFalse(!emptyModel.canRun(), "Default execution model should not be runnable.");
        failIfFalse(!emptyModel.validationErrors().empty(), "Default execution model should report missing Device.");
        failIfFalse(!emptyModel.hasResult(), "Default execution model should have no result.");
        failIfFalse(containsText(emptyModel.statusText(), "Not ready"), "Default execution status should be not ready.");

        const cppTDGL::SimulationRunResult missingDeviceResult = emptyModel.run();

        failIfFalse(missingDeviceResult.failed(), "Running without a Device should fail.");
        failIfFalse(emptyModel.failed(), "Model status should become failed after missing-Device run.");
        failIfFalse(emptyModel.hasResult(), "Failed run should still store a result.");
        failIfFalse(!emptyModel.diagnostics().empty(), "Failed run should expose diagnostics.");

        cppTDGL::Device validDevice = makeDevice(
            "simulation_execution_model_project",
            outputPath,
            true
        );

        cppTDGL::SimulationExecutionModel model =
            cppTDGL::SimulationExecutionModel::createForDevice(validDevice, "ui-execution-job");

        failIfFalse(model.hasDevice(), "Execution model should store the configured Device.");
        failIfFalse(model.jobId() == "ui-execution-job", "Execution model should preserve job id.");
        failIfFalse(model.isIdle(), "Execution model should start idle before run.");
        failIfFalse(model.canRun(), "Execution model with valid meshed Device should be runnable.");
        failIfFalse(model.validationErrors().empty(), "Execution model with valid meshed Device should have no validation errors.");
        failIfFalse(containsText(model.statusText(), "Ready"), "Runnable idle execution model should report ready status.");

        const cppTDGL::SimulationRunResult result = model.run();

        failIfFalse(result.completed(), "Execution model run should complete for valid Device.");
        failIfFalse(result.wroteOutput, "Execution model run should write output.");
        failIfFalse(model.completed(), "Model status should be completed after successful run.");
        failIfFalse(model.hasResult(), "Model should retain latest run result.");
        failIfFalse(model.latestResult().completed(), "Latest result should be completed.");
        failIfFalse(model.latestResult().solution.isValid(), "Latest result Solution should be valid.");
        failIfFalse(model.latestResult().solution.completed(), "Latest result Solution should be marked completed.");
        failIfFalse(model.latestResult().solution.frameCount() == 4, "Execution model should preserve expected solver frame count.");
        failIfFalse(std::filesystem::exists(outputPath), "Successful execution should create output file.");

        const cppTDGL::Solution loaded = cppTDGL::Hdf5SolutionReader::read(outputPath);

        failIfFalse(loaded.isValid(), "Execution output should load as a valid Solution.");
        failIfFalse(loaded.completed(), "Execution output should preserve completion flag.");
        failIfFalse(loaded.frameCount() == model.latestResult().solution.frameCount(), "Loaded output should preserve frame count.");

        const cppTDGL::SimulationExecutionSummary summary = model.summary();

        failIfFalse(summary.status == cppTDGL::SimulationExecutionStatus::Completed, "Summary should report completed status.");
        failIfFalse(summary.hasDevice, "Summary should report Device presence.");
        failIfFalse(summary.canRun, "Summary should report runnable configuration.");
        failIfFalse(summary.hasResult, "Summary should report result presence.");
        failIfFalse(summary.completed, "Summary should report completed result.");
        failIfFalse(summary.wroteOutput, "Summary should report written output.");
        failIfFalse(summary.frameCount == 4, "Summary should report frame count.");
        failIfFalse(summary.outputPath == outputPath, "Summary should report output path.");

        model.resetExecution();

        failIfFalse(model.isIdle(), "resetExecution() should restore idle state.");
        failIfFalse(!model.hasResult(), "resetExecution() should clear latest result.");
        failIfFalse(model.diagnostics().empty(), "resetExecution() should clear diagnostics.");
        failIfFalse(model.canRun(), "resetExecution() should preserve runnable Device configuration.");

        cppTDGL::Device invalidDevice = makeDevice(
            "simulation_execution_invalid_project",
            std::filesystem::temp_directory_path() / "CppTDGL_SimulationExecutionInvalidShouldNotWrite.h5",
            false
        );

        cppTDGL::SimulationExecutionModel invalidModel =
            cppTDGL::SimulationExecutionModel::createForDevice(invalidDevice, "invalid-ui-job");

        failIfFalse(!invalidModel.canRun(), "Execution model with unmeshed Device should not be runnable.");
        failIfFalse(!invalidModel.validationErrors().empty(), "Unmeshed Device should produce validation errors.");

        const cppTDGL::SimulationRunResult invalidResult = invalidModel.run();

        failIfFalse(invalidResult.failed(), "Invalid execution model run should return failed result.");
        failIfFalse(invalidModel.failed(), "Invalid execution model should enter failed state.");
        failIfFalse(!invalidModel.diagnostics().empty(), "Invalid execution should preserve diagnostics.");
        failIfFalse(!invalidResult.wroteOutput, "Invalid execution should not write output.");

        invalidModel.clearDevice();

        failIfFalse(!invalidModel.hasDevice(), "clearDevice() should remove Device.");
        failIfFalse(invalidModel.isIdle(), "clearDevice() should reset status to idle.");
        failIfFalse(!invalidModel.canRun(), "Model without Device should not be runnable.");

        std::filesystem::remove(outputPath);

        std::cout << "Simulation execution model smoke test passed.\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "Simulation execution model smoke test exception: " << error.what() << '\n';
        return 1;
    }
}
