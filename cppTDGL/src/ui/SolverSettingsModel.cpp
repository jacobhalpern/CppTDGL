#include "SolverSettingsModel.hpp"

#include <cmath>
#include <iomanip>
#include <sstream>
#include <string>
#include <utility>

namespace cppTDGL {
namespace {

[[nodiscard]] std::string doubleToText(double value) {
    std::ostringstream stream;
    stream << std::setprecision(12) << value;
    return stream.str();
}

[[nodiscard]] std::string boolToText(bool value) {
    return value ? "true" : "false";
}

[[nodiscard]] std::string fieldDescription(SolverSettingsField field) {
    switch (field) {
    case SolverSettingsField::SolveTime:
        return "Total simulated time for the run.";
    case SolverSettingsField::InitialTimeStep:
        return "Initial time step used by the solver skeleton and future adaptive stepping.";
    case SolverSettingsField::MaximumTimeStep:
        return "Upper time-step bound; must be greater than or equal to the initial time step.";
    case SolverSettingsField::AdaptiveTimeStepping:
        return "Whether future solver implementations may adapt the time step.";
    case SolverSettingsField::SaveEvery:
        return "Frame-save cadence expressed in solver steps.";
    case SolverSettingsField::IncludeScreening:
        return "Whether magnetic screening terms are requested.";
    case SolverSettingsField::OutputPath:
        return "Path to the solution output file.";
    case SolverSettingsField::SparseSolverName:
        return "Name of the requested sparse linear solver backend.";
    }

    return "Unknown solver setting.";
}

[[nodiscard]] std::string fieldValueText(const SolverOptions& options, SolverSettingsField field) {
    switch (field) {
    case SolverSettingsField::SolveTime:
        return doubleToText(options.solveTime);
    case SolverSettingsField::InitialTimeStep:
        return doubleToText(options.initialTimeStep);
    case SolverSettingsField::MaximumTimeStep:
        return doubleToText(options.maximumTimeStep);
    case SolverSettingsField::AdaptiveTimeStepping:
        return boolToText(options.adaptiveTimeStepping);
    case SolverSettingsField::SaveEvery:
        return std::to_string(options.saveEvery);
    case SolverSettingsField::IncludeScreening:
        return boolToText(options.includeScreening);
    case SolverSettingsField::OutputPath:
        return options.outputPath;
    case SolverSettingsField::SparseSolverName:
        return options.sparseSolverName;
    }

    return {};
}

[[nodiscard]] std::vector<SolverSettingsField> allFields() {
    return {
        SolverSettingsField::SolveTime,
        SolverSettingsField::InitialTimeStep,
        SolverSettingsField::MaximumTimeStep,
        SolverSettingsField::AdaptiveTimeStepping,
        SolverSettingsField::SaveEvery,
        SolverSettingsField::IncludeScreening,
        SolverSettingsField::OutputPath,
        SolverSettingsField::SparseSolverName,
    };
}

} // namespace

std::string solverSettingsFieldKey(SolverSettingsField field) {
    switch (field) {
    case SolverSettingsField::SolveTime:
        return "solveTime";
    case SolverSettingsField::InitialTimeStep:
        return "initialTimeStep";
    case SolverSettingsField::MaximumTimeStep:
        return "maximumTimeStep";
    case SolverSettingsField::AdaptiveTimeStepping:
        return "adaptiveTimeStepping";
    case SolverSettingsField::SaveEvery:
        return "saveEvery";
    case SolverSettingsField::IncludeScreening:
        return "includeScreening";
    case SolverSettingsField::OutputPath:
        return "outputPath";
    case SolverSettingsField::SparseSolverName:
        return "sparseSolverName";
    }

    return "unknown";
}

std::string solverSettingsFieldLabel(SolverSettingsField field) {
    switch (field) {
    case SolverSettingsField::SolveTime:
        return "Solve time";
    case SolverSettingsField::InitialTimeStep:
        return "Initial time step";
    case SolverSettingsField::MaximumTimeStep:
        return "Maximum time step";
    case SolverSettingsField::AdaptiveTimeStepping:
        return "Adaptive time stepping";
    case SolverSettingsField::SaveEvery:
        return "Save every";
    case SolverSettingsField::IncludeScreening:
        return "Include screening";
    case SolverSettingsField::OutputPath:
        return "Output path";
    case SolverSettingsField::SparseSolverName:
        return "Sparse solver";
    }

    return "Unknown";
}

SolverSettingsModel SolverSettingsModel::createDefault() {
    return SolverSettingsModel{};
}

SolverSettingsModel SolverSettingsModel::fromSolverOptions(SolverOptions options) {
    SolverSettingsModel model;
    model.setSolverOptions(std::move(options));
    return model;
}

const SolverOptions& SolverSettingsModel::solverOptions() const noexcept {
    return solverOptions_;
}

void SolverSettingsModel::setSolverOptions(SolverOptions value) {
    solverOptions_ = std::move(value);
}

double SolverSettingsModel::solveTime() const noexcept {
    return solverOptions_.solveTime;
}

void SolverSettingsModel::setSolveTime(double value) noexcept {
    solverOptions_.solveTime = value;
}

double SolverSettingsModel::initialTimeStep() const noexcept {
    return solverOptions_.initialTimeStep;
}

void SolverSettingsModel::setInitialTimeStep(double value) noexcept {
    solverOptions_.initialTimeStep = value;
}

double SolverSettingsModel::maximumTimeStep() const noexcept {
    return solverOptions_.maximumTimeStep;
}

void SolverSettingsModel::setMaximumTimeStep(double value) noexcept {
    solverOptions_.maximumTimeStep = value;
}

bool SolverSettingsModel::adaptiveTimeStepping() const noexcept {
    return solverOptions_.adaptiveTimeStepping;
}

void SolverSettingsModel::setAdaptiveTimeStepping(bool value) noexcept {
    solverOptions_.adaptiveTimeStepping = value;
}

int SolverSettingsModel::saveEvery() const noexcept {
    return solverOptions_.saveEvery;
}

void SolverSettingsModel::setSaveEvery(int value) noexcept {
    solverOptions_.saveEvery = value;
}

bool SolverSettingsModel::includeScreening() const noexcept {
    return solverOptions_.includeScreening;
}

void SolverSettingsModel::setIncludeScreening(bool value) noexcept {
    solverOptions_.includeScreening = value;
}

const std::string& SolverSettingsModel::outputPath() const noexcept {
    return solverOptions_.outputPath;
}

void SolverSettingsModel::setOutputPath(std::string value) {
    solverOptions_.outputPath = std::move(value);
}

const std::string& SolverSettingsModel::sparseSolverName() const noexcept {
    return solverOptions_.sparseSolverName;
}

void SolverSettingsModel::setSparseSolverName(std::string value) {
    solverOptions_.sparseSolverName = std::move(value);
}

bool SolverSettingsModel::isValid() const {
    return validationErrors().empty();
}

bool SolverSettingsModel::canRun() const {
    return isValid();
}

std::vector<std::string> SolverSettingsModel::validationErrors() const {
    std::vector<std::string> errors;

    for (const SolverSettingsField field : allFields()) {
        const std::vector<std::string> fieldErrors = validationErrorsForField(field);
        errors.insert(errors.end(), fieldErrors.begin(), fieldErrors.end());
    }

    return errors;
}

std::vector<std::string> SolverSettingsModel::validationErrorsForField(SolverSettingsField field) const {
    std::vector<std::string> errors;

    switch (field) {
    case SolverSettingsField::SolveTime:
        if (!std::isfinite(solverOptions_.solveTime) || solverOptions_.solveTime <= 0.0) {
            errors.emplace_back("Solve time must be a finite value greater than zero.");
        }
        break;
    case SolverSettingsField::InitialTimeStep:
        if (!std::isfinite(solverOptions_.initialTimeStep) || solverOptions_.initialTimeStep <= 0.0) {
            errors.emplace_back("Initial time step must be a finite value greater than zero.");
        }
        if (std::isfinite(solverOptions_.maximumTimeStep) &&
            std::isfinite(solverOptions_.initialTimeStep) &&
            solverOptions_.maximumTimeStep < solverOptions_.initialTimeStep) {
            errors.emplace_back("Initial time step must be less than or equal to maximum time step.");
        }
        break;
    case SolverSettingsField::MaximumTimeStep:
        if (!std::isfinite(solverOptions_.maximumTimeStep) || solverOptions_.maximumTimeStep <= 0.0) {
            errors.emplace_back("Maximum time step must be a finite value greater than zero.");
        }
        if (std::isfinite(solverOptions_.maximumTimeStep) &&
            std::isfinite(solverOptions_.initialTimeStep) &&
            solverOptions_.maximumTimeStep < solverOptions_.initialTimeStep) {
            errors.emplace_back("Maximum time step must be greater than or equal to initial time step.");
        }
        break;
    case SolverSettingsField::AdaptiveTimeStepping:
        break;
    case SolverSettingsField::SaveEvery:
        if (solverOptions_.saveEvery <= 0) {
            errors.emplace_back("Save interval must be greater than zero.");
        }
        break;
    case SolverSettingsField::IncludeScreening:
        break;
    case SolverSettingsField::OutputPath:
        if (solverOptions_.outputPath.empty()) {
            errors.emplace_back("Output path must not be empty.");
        }
        break;
    case SolverSettingsField::SparseSolverName:
        if (solverOptions_.sparseSolverName.empty()) {
            errors.emplace_back("Sparse solver name must not be empty.");
        }
        break;
    }

    return errors;
}

std::vector<SolverSettingsFieldInfo> SolverSettingsModel::fieldInfos() const {
    std::vector<SolverSettingsFieldInfo> fields;

    for (const SolverSettingsField field : allFields()) {
        std::vector<std::string> errors = validationErrorsForField(field);
        fields.push_back({
            field,
            solverSettingsFieldKey(field),
            solverSettingsFieldLabel(field),
            fieldValueText(solverOptions_, field),
            fieldDescription(field),
            errors.empty(),
            std::move(errors),
        });
    }

    return fields;
}

std::string SolverSettingsModel::summaryText() const {
    std::ostringstream stream;
    stream << "solveTime=" << doubleToText(solverOptions_.solveTime)
           << ", initialTimeStep=" << doubleToText(solverOptions_.initialTimeStep)
           << ", maximumTimeStep=" << doubleToText(solverOptions_.maximumTimeStep)
           << ", saveEvery=" << solverOptions_.saveEvery
           << ", outputPath=" << solverOptions_.outputPath
           << ", sparseSolver=" << solverOptions_.sparseSolverName;

    return stream.str();
}

} // namespace cppTDGL
