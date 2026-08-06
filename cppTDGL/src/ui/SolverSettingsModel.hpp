#pragma once

#include "SolverOptions.hpp"

#include <string>
#include <vector>

namespace cppTDGL {

enum class SolverSettingsField {
    SolveTime,
    InitialTimeStep,
    MaximumTimeStep,
    AdaptiveTimeStepping,
    SaveEvery,
    IncludeScreening,
    OutputPath,
    SparseSolverName,
};

[[nodiscard]] std::string solverSettingsFieldKey(SolverSettingsField field);
[[nodiscard]] std::string solverSettingsFieldLabel(SolverSettingsField field);

struct SolverSettingsFieldInfo {
    SolverSettingsField field = SolverSettingsField::SolveTime;
    std::string key;
    std::string label;
    std::string valueText;
    std::string description;
    bool valid = true;
    std::vector<std::string> validationErrors;
};

class SolverSettingsModel {
public:
    SolverSettingsModel() = default;

    [[nodiscard]] static SolverSettingsModel createDefault();
    [[nodiscard]] static SolverSettingsModel fromSolverOptions(SolverOptions options);

    [[nodiscard]] const SolverOptions& solverOptions() const noexcept;
    void setSolverOptions(SolverOptions value);

    [[nodiscard]] double solveTime() const noexcept;
    void setSolveTime(double value) noexcept;

    [[nodiscard]] double initialTimeStep() const noexcept;
    void setInitialTimeStep(double value) noexcept;

    [[nodiscard]] double maximumTimeStep() const noexcept;
    void setMaximumTimeStep(double value) noexcept;

    [[nodiscard]] bool adaptiveTimeStepping() const noexcept;
    void setAdaptiveTimeStepping(bool value) noexcept;

    [[nodiscard]] int saveEvery() const noexcept;
    void setSaveEvery(int value) noexcept;

    [[nodiscard]] bool includeScreening() const noexcept;
    void setIncludeScreening(bool value) noexcept;

    [[nodiscard]] const std::string& outputPath() const noexcept;
    void setOutputPath(std::string value);

    [[nodiscard]] const std::string& sparseSolverName() const noexcept;
    void setSparseSolverName(std::string value);

    [[nodiscard]] bool isValid() const;
    [[nodiscard]] bool canRun() const;
    [[nodiscard]] std::vector<std::string> validationErrors() const;
    [[nodiscard]] std::vector<std::string> validationErrorsForField(SolverSettingsField field) const;

    [[nodiscard]] std::vector<SolverSettingsFieldInfo> fieldInfos() const;
    [[nodiscard]] std::string summaryText() const;

private:
    SolverOptions solverOptions_{};
};

} // namespace cppTDGL
