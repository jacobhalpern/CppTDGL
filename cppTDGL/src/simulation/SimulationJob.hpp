#pragma once

#include "Device.hpp"

#include <string>
#include <vector>

namespace cppTDGL {

enum class SimulationJobStatus {
    Pending,
    Running,
    Completed,
    Failed,
};

[[nodiscard]] std::string simulationJobStatusName(SimulationJobStatus status);

class SimulationJob {
public:
    SimulationJob() = default;
    SimulationJob(std::string jobId, Device device);
    virtual ~SimulationJob() = default;

    [[nodiscard]] std::string className() const;

    [[nodiscard]] const std::string& jobId() const noexcept;
    void setJobId(std::string value);

    [[nodiscard]] const Device& device() const noexcept;
    [[nodiscard]] Device& device() noexcept;
    void setDevice(Device value);

    [[nodiscard]] SimulationJobStatus status() const noexcept;
    void setStatus(SimulationJobStatus value) noexcept;

    [[nodiscard]] const std::vector<std::string>& diagnostics() const noexcept;
    void addDiagnostic(std::string diagnostic);
    void clearDiagnostics();

    [[nodiscard]] bool isPending() const noexcept;
    [[nodiscard]] bool isRunning() const noexcept;
    [[nodiscard]] bool isCompleted() const noexcept;
    [[nodiscard]] bool isFailed() const noexcept;

    [[nodiscard]] bool isValid() const;
    [[nodiscard]] std::vector<std::string> validationErrors() const;

private:
    std::string jobId_ = "untitled-job";
    Device device_{};
    SimulationJobStatus status_ = SimulationJobStatus::Pending;
    std::vector<std::string> diagnostics_;
};

} // namespace cppTDGL
