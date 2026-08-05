#include "SimulationJob.hpp"

#include <string>
#include <utility>

namespace cppTDGL {
namespace {

void appendErrors(
    std::vector<std::string>& destination,
    const std::string& prefix,
    const std::vector<std::string>& source
) {
    for (const std::string& error : source) {
        destination.push_back(prefix + error);
    }
}

} // namespace

std::string simulationJobStatusName(SimulationJobStatus status) {
    switch (status) {
    case SimulationJobStatus::Pending:
        return "Pending";
    case SimulationJobStatus::Running:
        return "Running";
    case SimulationJobStatus::Completed:
        return "Completed";
    case SimulationJobStatus::Failed:
        return "Failed";
    }

    return "Unknown";
}

SimulationJob::SimulationJob(std::string jobId, Device device)
    : jobId_(std::move(jobId)),
      device_(std::move(device)) {}

std::string SimulationJob::className() const {
    return "SimulationJob";
}

const std::string& SimulationJob::jobId() const noexcept {
    return jobId_;
}

void SimulationJob::setJobId(std::string value) {
    jobId_ = std::move(value);
}

const Device& SimulationJob::device() const noexcept {
    return device_;
}

Device& SimulationJob::device() noexcept {
    return device_;
}

void SimulationJob::setDevice(Device value) {
    device_ = std::move(value);
}

SimulationJobStatus SimulationJob::status() const noexcept {
    return status_;
}

void SimulationJob::setStatus(SimulationJobStatus value) noexcept {
    status_ = value;
}

const std::vector<std::string>& SimulationJob::diagnostics() const noexcept {
    return diagnostics_;
}

void SimulationJob::addDiagnostic(std::string diagnostic) {
    diagnostics_.push_back(std::move(diagnostic));
}

void SimulationJob::clearDiagnostics() {
    diagnostics_.clear();
}

bool SimulationJob::isPending() const noexcept {
    return status_ == SimulationJobStatus::Pending;
}

bool SimulationJob::isRunning() const noexcept {
    return status_ == SimulationJobStatus::Running;
}

bool SimulationJob::isCompleted() const noexcept {
    return status_ == SimulationJobStatus::Completed;
}

bool SimulationJob::isFailed() const noexcept {
    return status_ == SimulationJobStatus::Failed;
}

bool SimulationJob::isValid() const {
    return validationErrors().empty();
}

std::vector<std::string> SimulationJob::validationErrors() const {
    std::vector<std::string> errors;

    if (jobId_.empty()) {
        errors.emplace_back("SimulationJob id must not be empty.");
    }

    appendErrors(errors, "SimulationJob device: ", device_.validationErrors());

    if (!device_.hasMesh()) {
        errors.emplace_back("SimulationJob device must have an attached mesh.");
    } else {
        appendErrors(errors, "SimulationJob device mesh: ", device_.mesh().validationErrors());
    }

    return errors;
}

} // namespace cppTDGL
