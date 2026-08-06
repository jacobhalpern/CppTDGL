#pragma once

#include "Device.hpp"
#include "Layer.hpp"
#include "MeshGenerator.hpp"
#include "SolverOptions.hpp"

#include <cstddef>
#include <string>
#include <vector>

namespace cppTDGL {

struct ProjectWizardMaterialSettings {
    double coherenceLength = 0.5;
    double londonLambda = 2.0;
    double thickness = 0.1;
    double conductivity = 1.0;
    double gamma = 1.0;
    std::string units = "um";

    [[nodiscard]] Layer toLayer() const;
    [[nodiscard]] std::vector<std::string> validationErrors() const;
};

struct ProjectWizardGeometrySettings {
    double filmWidth = 10.0;
    double filmHeight = 4.0;
    bool attachGeneratedMesh = true;
    std::size_t meshXDivisions = 4;
    std::size_t meshYDivisions = 2;

    [[nodiscard]] MeshGenerationOptions meshGenerationOptions() const;
    [[nodiscard]] std::vector<std::string> validationErrors() const;
};

class ProjectWizardModel {
public:
    ProjectWizardModel() = default;

    [[nodiscard]] static ProjectWizardModel createDefault();

    [[nodiscard]] const std::string& projectName() const noexcept;
    void setProjectName(std::string value);

    [[nodiscard]] const ProjectWizardMaterialSettings& materialSettings() const noexcept;
    [[nodiscard]] ProjectWizardMaterialSettings& materialSettings() noexcept;
    void setMaterialSettings(ProjectWizardMaterialSettings value);

    [[nodiscard]] const ProjectWizardGeometrySettings& geometrySettings() const noexcept;
    [[nodiscard]] ProjectWizardGeometrySettings& geometrySettings() noexcept;
    void setGeometrySettings(ProjectWizardGeometrySettings value);

    [[nodiscard]] const SolverOptions& solverOptions() const noexcept;
    [[nodiscard]] SolverOptions& solverOptions() noexcept;
    void setSolverOptions(SolverOptions value);

    [[nodiscard]] bool isValid() const;
    [[nodiscard]] std::vector<std::string> validationErrors() const;

    [[nodiscard]] Device buildDevice() const;

private:
    std::string projectName_ = "cppTDGL_project";
    ProjectWizardMaterialSettings materialSettings_{};
    ProjectWizardGeometrySettings geometrySettings_{};
    SolverOptions solverOptions_{};
};

} // namespace cppTDGL
