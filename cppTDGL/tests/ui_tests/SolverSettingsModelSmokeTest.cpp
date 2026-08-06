#include "SolverOptions.hpp"
#include "SolverSettingsModel.hpp"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <limits>
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

bool hasKey(
    const std::vector<cppTDGL::SolverSettingsFieldInfo>& fields,
    const std::string& key
) {
    return std::any_of(
        fields.begin(),
        fields.end(),
        [&key](const cppTDGL::SolverSettingsFieldInfo& field) {
            return field.key == key;
        }
    );
}

bool containsText(const std::string& value, const std::string& token) {
    return value.find(token) != std::string::npos;
}

} // namespace

int main() {
    try {
        cppTDGL::SolverSettingsModel model = cppTDGL::SolverSettingsModel::createDefault();

        failIfFalse(model.isValid(), "Default solver settings model should be valid.");
        failIfFalse(model.canRun(), "Default solver settings model should be runnable.");
        failIfFalse(model.validationErrors().empty(), "Default solver settings should have no validation errors.");

        const std::vector<cppTDGL::SolverSettingsFieldInfo> defaultFields = model.fieldInfos();

        failIfFalse(defaultFields.size() == 8, "Solver settings model should expose 8 UI fields.");
        failIfFalse(hasKey(defaultFields, "solveTime"), "Field list should include solveTime.");
        failIfFalse(hasKey(defaultFields, "initialTimeStep"), "Field list should include initialTimeStep.");
        failIfFalse(hasKey(defaultFields, "maximumTimeStep"), "Field list should include maximumTimeStep.");
        failIfFalse(hasKey(defaultFields, "adaptiveTimeStepping"), "Field list should include adaptiveTimeStepping.");
        failIfFalse(hasKey(defaultFields, "saveEvery"), "Field list should include saveEvery.");
        failIfFalse(hasKey(defaultFields, "includeScreening"), "Field list should include includeScreening.");
        failIfFalse(hasKey(defaultFields, "outputPath"), "Field list should include outputPath.");
        failIfFalse(hasKey(defaultFields, "sparseSolverName"), "Field list should include sparseSolverName.");

        model.setSolveTime(25.0);
        model.setInitialTimeStep(0.01);
        model.setMaximumTimeStep(0.05);
        model.setAdaptiveTimeStepping(false);
        model.setSaveEvery(7);
        model.setIncludeScreening(true);
        model.setOutputPath("solver_settings_solution.h5");
        model.setSparseSolverName("test-sparse-solver");

        failIfFalse(model.isValid(), "Customized solver settings should be valid.");
        failIfNotNear(model.solveTime(), 25.0, "Solve time setter should update model.");
        failIfNotNear(model.initialTimeStep(), 0.01, "Initial time-step setter should update model.");
        failIfNotNear(model.maximumTimeStep(), 0.05, "Maximum time-step setter should update model.");
        failIfFalse(!model.adaptiveTimeStepping(), "Adaptive time-stepping setter should update model.");
        failIfFalse(model.saveEvery() == 7, "Save interval setter should update model.");
        failIfFalse(model.includeScreening(), "Screening setter should update model.");
        failIfFalse(model.outputPath() == "solver_settings_solution.h5", "Output-path setter should update model.");
        failIfFalse(model.sparseSolverName() == "test-sparse-solver", "Sparse-solver setter should update model.");

        const cppTDGL::SolverOptions options = model.solverOptions();

        failIfFalse(options.isValid(), "Customized SolverOptions should be valid.");
        failIfNotNear(options.solveTime, 25.0, "SolverOptions solveTime should match model.");
        failIfFalse(options.outputPath == "solver_settings_solution.h5", "SolverOptions output path should match model.");
        failIfFalse(options.sparseSolverName == "test-sparse-solver", "SolverOptions sparse solver should match model.");

        const std::string summary = model.summaryText();

        failIfFalse(containsText(summary, "solveTime"), "Summary should include solveTime.");
        failIfFalse(containsText(summary, "outputPath"), "Summary should include outputPath.");
        failIfFalse(containsText(summary, "test-sparse-solver"), "Summary should include sparse solver name.");

        cppTDGL::SolverOptions importedOptions;
        importedOptions.solveTime = 11.0;
        importedOptions.initialTimeStep = 0.02;
        importedOptions.maximumTimeStep = 0.08;
        importedOptions.adaptiveTimeStepping = true;
        importedOptions.saveEvery = 4;
        importedOptions.includeScreening = true;
        importedOptions.outputPath = "imported_solution.h5";
        importedOptions.sparseSolverName = "imported-solver";

        cppTDGL::SolverSettingsModel importedModel =
            cppTDGL::SolverSettingsModel::fromSolverOptions(importedOptions);

        failIfFalse(importedModel.isValid(), "Model built from valid SolverOptions should be valid.");
        failIfNotNear(importedModel.solveTime(), 11.0, "Imported solve time should round trip.");
        failIfFalse(importedModel.saveEvery() == 4, "Imported save interval should round trip.");
        failIfFalse(importedModel.outputPath() == "imported_solution.h5", "Imported output path should round trip.");

        cppTDGL::SolverSettingsModel invalidModel = cppTDGL::SolverSettingsModel::createDefault();
        invalidModel.setSolveTime(0.0);
        invalidModel.setInitialTimeStep(0.2);
        invalidModel.setMaximumTimeStep(0.1);
        invalidModel.setSaveEvery(0);
        invalidModel.setOutputPath("");
        invalidModel.setSparseSolverName("");

        failIfFalse(!invalidModel.isValid(), "Invalid solver settings should fail validation.");
        failIfFalse(!invalidModel.canRun(), "Invalid solver settings should not be runnable.");
        failIfFalse(!invalidModel.validationErrors().empty(), "Invalid solver settings should produce validation errors.");
        failIfFalse(
            !invalidModel.validationErrorsForField(cppTDGL::SolverSettingsField::SolveTime).empty(),
            "Invalid solve time should produce field-level validation errors."
        );
        failIfFalse(
            !invalidModel.validationErrorsForField(cppTDGL::SolverSettingsField::MaximumTimeStep).empty(),
            "Invalid maximum time step should produce field-level validation errors."
        );
        failIfFalse(
            !invalidModel.validationErrorsForField(cppTDGL::SolverSettingsField::OutputPath).empty(),
            "Invalid output path should produce field-level validation errors."
        );
        failIfFalse(
            !invalidModel.validationErrorsForField(cppTDGL::SolverSettingsField::SparseSolverName).empty(),
            "Invalid sparse solver name should produce field-level validation errors."
        );

        const std::vector<cppTDGL::SolverSettingsFieldInfo> invalidFields = invalidModel.fieldInfos();
        const auto invalidOutput = std::find_if(
            invalidFields.begin(),
            invalidFields.end(),
            [](const cppTDGL::SolverSettingsFieldInfo& field) {
                return field.field == cppTDGL::SolverSettingsField::OutputPath;
            }
        );

        failIfFalse(invalidOutput != invalidFields.end(), "Invalid field list should include output path.");
        failIfFalse(!invalidOutput->valid, "Invalid output path field should be marked invalid.");
        failIfFalse(!invalidOutput->validationErrors.empty(), "Invalid output path field should carry errors.");

        invalidModel.setSolveTime(std::numeric_limits<double>::infinity());

        failIfFalse(
            !invalidModel.validationErrorsForField(cppTDGL::SolverSettingsField::SolveTime).empty(),
            "Non-finite solve time should fail validation."
        );

        std::cout << "Solver settings model smoke test passed.\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "Solver settings model smoke test exception: " << error.what() << '\n';
        return 1;
    }
}
