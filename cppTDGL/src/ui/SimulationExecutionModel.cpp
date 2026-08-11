#include "SimulationExecutionModel.hpp"

#include <stdexcept>
#include <string>
#include <utility>

namespace cppTDGL {
namespace {

void appendErrors(
    std::vector<std::string>& destination,
    const std::vector<std::string>& source
) {
    destination.insert(destination.end(), source.begin(), source.end());
}

} // namespace

std::string simulationExecutionStatusName(SimulationExecutionStatus status) {
    switch (status) {
    case SimulationExecutionStatus::Idle:
        return "Idle";
    case SimulationExecutionStatus::Running:
        return "Running";
    case SimulationExecutionStatus::Completed:
        return "Completed";
    case SimulationExecutionStatus::Failed:
        return "Failed";
    }

    return "Unknown";
}

SimulationExecutionModel SimulationExecutionModel::createForDevice(
    Device device,
    std::string jobId
) {
    SimulationExecutionModel model;
    model.setJobId(std::move(jobId));
    model.setDevice(std::move(device));
    return model;
}

const std::string& SimulationExecutionModel::jobId() const noexcept {
    return jobId_;
}

void SimulationExecutionModel::setJobId(std::string value) {
    jobId_ = std::move(value);
}

bool SimulationExecutionModel::hasDevice() const noexcept {
    return device_.has_value();
}

const Device& SimulationExecutionModel::device() const {
    if (!device_.has_value()) {
        throw std::logic_error("SimulationExecutionModel does not contain a Device.");
    }

    return *device_;
}

Device& SimulationExecutionModel::device() {
    if (!device_.has_value()) {
        throw std::logic_error("SimulationExecutionModel does not contain a Device.");
    }

    return *device_;
}

void SimulationExecutionModel::setDevice(Device value) {
    device_ = std::move(value);
    resetExecution();
}

void SimulationExecutionModel::clearDevice() {
    device_.reset();
    resetExecution();
}

SimulationExecutionStatus SimulationExecutionModel::status() const noexcept {
    return status_;
}

bool SimulationExecutionModel::isIdle() const noexcept {
    return status_ == SimulationExecutionStatus::Idle;
}

bool SimulationExecutionModel::isRunning() const noexcept {
    return status_ == SimulationExecutionStatus::Running;
}

bool SimulationExecutionModel::completed() const noexcept {
    return status_ == SimulationExecutionStatus::Completed;
}

bool SimulationExecutionModel::failed() const noexcept {
    return status_ == SimulationExecutionStatus::Failed;
}

bool SimulationExecutionModel::hasResult() const noexcept {
    return latestResult_.has_value();
}

const SimulationRunResult& SimulationExecutionModel::latestResult() const {
    if (!latestResult_.has_value()) {
        throw std::logic_error("SimulationExecutionModel has no latest result.");
    }

    return *latestResult_;
}

bool SimulationExecutionModel::canRun() const {
    return validationErrors().empty();
}

std::vector<std::string> SimulationExecutionModel::validationErrors() const {
    std::vector<std::string> errors;

    if (jobId_.empty()) {
        errors.emplace_back("Simulation execution job id must not be empty.");
    }

    if (!device_.has_value()) {
        errors.emplace_back("Simulation execution requires a Device.");
        return errors;
    }

    appendErrors(errors, buildJob().validationErrors());
    return errors;
}

SimulationRunResult SimulationExecutionModel::run() {
    diagnostics_.clear();
    status_ = SimulationExecutionStatus::Running;

    if (!device_.has_value()) {
        SimulationRunResult result;
        result.jobId = jobId_;
        result.status = SimulationJobStatus::Failed;
        result.diagnostics.push_back("Simulation execution requires a Device.");
        setResult(std::move(result));
        return latestResult();
    }

    SimulationRunResult result = SimulationRunner::run(buildJob());
    setResult(std::move(result));
    return latestResult();
}

void SimulationExecutionModel::resetExecution() {
    status_ = SimulationExecutionStatus::Idle;
    latestResult_.reset();
    diagnostics_.clear();
}

const std::vector<std::string>& SimulationExecutionModel::diagnostics() const noexcept {
    return diagnostics_;
}

std::string SimulationExecutionModel::statusText() const {
    if (!diagnostics_.empty()) {
        return simulationExecutionStatusName(status_) + ": " + diagnostics_.front();
    }

    if (status_ == SimulationExecutionStatus::Idle && canRun()) {
        return "Ready";
    }

    if (status_ == SimulationExecutionStatus::Idle && !canRun()) {
        return "Not ready";
    }

    return simulationExecutionStatusName(status_);
}

SimulationExecutionSummary SimulationExecutionModel::summary() const {
    SimulationExecutionSummary value;
    value.status = status_;
    value.statusText = statusText();
    value.jobId = jobId_;
    value.hasDevice = hasDevice();
    value.canRun = canRun();
    value.hasResult = hasResult();
    value.diagnosticCount = diagnostics_.size();

    if (latestResult_.has_value()) {
        value.completed = latestResult_->completed();
        value.failed = latestResult_->failed();
        value.wroteOutput = latestResult_->wroteOutput;
        value.outputPath = latestResult_->outputPath;
        value.frameCount = latestResult_->solution.frameCount();
    }

    return value;
}

SimulationJob SimulationExecutionModel::buildJob() const {
    if (!device_.has_value()) {
        throw std::logic_error("Cannot build SimulationJob without a Device.");
    }

    return SimulationJob(jobId_, *device_);
}

void SimulationExecutionModel::setResult(SimulationRunResult result) {
    diagnostics_ = result.diagnostics;

    if (result.completed()) {
        status_ = SimulationExecutionStatus::Completed;
    } else if (result.failed()) {
        status_ = SimulationExecutionStatus::Failed;
    } else {
        status_ = SimulationExecutionStatus::Idle;
    }

    latestResult_ = std::move(result);
}

} // namespace cppTDGL
