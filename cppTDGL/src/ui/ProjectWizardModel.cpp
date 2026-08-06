#include "ProjectWizardModel.hpp"

#include "Polygon2D.hpp"

#include <stdexcept>
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

Layer ProjectWizardMaterialSettings::toLayer() const {
    return Layer(
        coherenceLength,
        londonLambda,
        thickness,
        conductivity,
        gamma,
        units
    );
}

std::vector<std::string> ProjectWizardMaterialSettings::validationErrors() const {
    return toLayer().validationErrors();
}

MeshGenerationOptions ProjectWizardGeometrySettings::meshGenerationOptions() const {
    MeshGenerationOptions options;
    options.xDivisions = meshXDivisions;
    options.yDivisions = meshYDivisions;
    options.includeBoundaryVertices = true;
    return options;
}

std::vector<std::string> ProjectWizardGeometrySettings::validationErrors() const {
    std::vector<std::string> errors;

    if (filmWidth <= 0.0) {
        errors.emplace_back("Film width must be greater than zero.");
    }

    if (filmHeight <= 0.0) {
        errors.emplace_back("Film height must be greater than zero.");
    }

    if (attachGeneratedMesh) {
        appendErrors(errors, "Mesh: ", meshGenerationOptions().validationErrors());
    }

    return errors;
}

ProjectWizardModel ProjectWizardModel::createDefault() {
    return ProjectWizardModel{};
}

const std::string& ProjectWizardModel::projectName() const noexcept {
    return projectName_;
}

void ProjectWizardModel::setProjectName(std::string value) {
    projectName_ = std::move(value);
}

const ProjectWizardMaterialSettings& ProjectWizardModel::materialSettings() const noexcept {
    return materialSettings_;
}

ProjectWizardMaterialSettings& ProjectWizardModel::materialSettings() noexcept {
    return materialSettings_;
}

void ProjectWizardModel::setMaterialSettings(ProjectWizardMaterialSettings value) {
    materialSettings_ = std::move(value);
}

const ProjectWizardGeometrySettings& ProjectWizardModel::geometrySettings() const noexcept {
    return geometrySettings_;
}

ProjectWizardGeometrySettings& ProjectWizardModel::geometrySettings() noexcept {
    return geometrySettings_;
}

void ProjectWizardModel::setGeometrySettings(ProjectWizardGeometrySettings value) {
    geometrySettings_ = std::move(value);
}

const SolverOptions& ProjectWizardModel::solverOptions() const noexcept {
    return solverOptions_;
}

SolverOptions& ProjectWizardModel::solverOptions() noexcept {
    return solverOptions_;
}

void ProjectWizardModel::setSolverOptions(SolverOptions value) {
    solverOptions_ = std::move(value);
}

bool ProjectWizardModel::isValid() const {
    return validationErrors().empty();
}

std::vector<std::string> ProjectWizardModel::validationErrors() const {
    std::vector<std::string> errors;

    if (projectName_.empty()) {
        errors.emplace_back("Project name must not be empty.");
    }

    appendErrors(errors, "Material: ", materialSettings_.validationErrors());
    appendErrors(errors, "Geometry: ", geometrySettings_.validationErrors());
    appendErrors(errors, "Solver: ", solverOptions_.validationErrors());

    return errors;
}

Device ProjectWizardModel::buildDevice() const {
    const std::vector<std::string> errors = validationErrors();

    if (!errors.empty()) {
        throw std::invalid_argument(errors.front());
    }

    Polygon2D film = Polygon2D::rectangle(
        "film",
        geometrySettings_.filmWidth,
        geometrySettings_.filmHeight
    );

    Device device(
        projectName_,
        materialSettings_.toLayer(),
        film,
        solverOptions_
    );

    device.addProbePoint({0.0, 0.0});

    if (geometrySettings_.attachGeneratedMesh) {
        device = MeshGenerator::attachGeneratedFilmBoundingBoxMesh(
            std::move(device),
            geometrySettings_.meshGenerationOptions()
        );
    }

    return device;
}

} // namespace cppTDGL
