#pragma once

#include "Device.hpp"
#include "SimulationJob.hpp"
#include "SimulationRunner.hpp"
#include "Solution.hpp"

#include <filesystem>
#include <optional>
#include <string>
#include <vector>

namespace cppTDGL {

enum class SimulationExecutionStatus {
    Idle,
    Running,
    Completed,
    Failed,
};

[[nodiscard]] std::string simulationExecutionStatusName(SimulationExecutionStatus status);

struct SimulationExecutionSummary {
    SimulationExecutionStatus status = SimulationExecutionStatus::Idle;
    std::string statusText;
    std::string jobId;
    bool hasDevice = false;
    bool canRun = false;
    bool hasResult = false;
    bool completed = false;
    bool failed = false;
    bool wroteOutput = false;
    std::filesystem::path outputPath;
    std::size_t frameCount = 0;
    std::size_t diagnosticCount = 0;
};

class SimulationExecutionModel {
public:
    SimulationExecutionModel() = default;

    [[nodiscard]] static SimulationExecutionModel createForDevice(
        Device device,
        std::string jobId = "ui-simulation-job"
    );

    [[nodiscard]] const std::string& jobId() const noexcept;
    void setJobId(std::string value);

    [[nodiscard]] bool hasDevice() const noexcept;
    [[nodiscard]] const Device& device() const;
    [[nodiscard]] Device& device();
    void setDevice(Device value);
    void clearDevice();

    [[nodiscard]] SimulationExecutionStatus status() const noexcept;
    [[nodiscard]] bool isIdle() const noexcept;
    [[nodiscard]] bool isRunning() const noexcept;
    [[nodiscard]] bool completed() const noexcept;
    [[nodiscard]] bool failed() const noexcept;

    [[nodiscard]] bool hasResult() const noexcept;
    [[nodiscard]] const SimulationRunResult& latestResult() const;

    [[nodiscard]] bool canRun() const;
    [[nodiscard]] std::vector<std::string> validationErrors() const;

    [[nodiscard]] SimulationRunResult run();
    void resetExecution();

    [[nodiscard]] const std::vector<std::string>& diagnostics() const noexcept;
    [[nodiscard]] std::string statusText() const;
    [[nodiscard]] SimulationExecutionSummary summary() const;

private:
    std::string jobId_ = "ui-simulation-job";
    std::optional<Device> device_;
    SimulationExecutionStatus status_ = SimulationExecutionStatus::Idle;
    std::optional<SimulationRunResult> latestResult_;
    std::vector<std::string> diagnostics_;

    [[nodiscard]] SimulationJob buildJob() const;
    void setResult(SimulationRunResult result);
};

} // namespace cppTDGL
