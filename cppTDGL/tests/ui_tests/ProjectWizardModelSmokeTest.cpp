#include "EdgeMesh.hpp"
#include "ProjectWizardModel.hpp"

#include <cmath>
#include <iostream>
#include <stdexcept>
#include <string>

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

} // namespace

int main() {
    try {
        cppTDGL::ProjectWizardModel model = cppTDGL::ProjectWizardModel::createDefault();

        failIfFalse(model.isValid(), "Default project wizard model should be valid.");
        failIfFalse(model.projectName() == "cppTDGL_project", "Default project name should be stable.");

        const cppTDGL::Device defaultDevice = model.buildDevice();

        failIfFalse(defaultDevice.isValid(), "Default wizard Device should be valid.");
        failIfFalse(defaultDevice.hasMesh(), "Default wizard Device should include a generated mesh.");
        failIfFalse(defaultDevice.mesh().isValid(), "Default generated mesh should be valid.");
        failIfFalse(defaultDevice.mesh().vertexCount() == 15, "Default 4x2 mesh should have 15 vertices.");
        failIfFalse(defaultDevice.mesh().triangleCount() == 16, "Default 4x2 mesh should have 16 triangles.");
        failIfNotNear(defaultDevice.mesh().totalArea(), 40.0, "Default generated mesh area should match film area.");

        const cppTDGL::EdgeMesh defaultEdges = cppTDGL::EdgeMesh::fromMesh(defaultDevice.mesh());

        failIfFalse(defaultEdges.isValid(), "Default generated mesh should produce a valid EdgeMesh.");
        failIfFalse(defaultEdges.boundaryEdgeCount() == 12, "Default 4x2 mesh should have 12 boundary edges.");

        cppTDGL::ProjectWizardGeometrySettings geometry;
        geometry.filmWidth = 20.0;
        geometry.filmHeight = 5.0;
        geometry.meshXDivisions = 5;
        geometry.meshYDivisions = 1;

        model.setProjectName("custom_wizard_project");
        model.setGeometrySettings(geometry);

        cppTDGL::SolverOptions solver = model.solverOptions();
        solver.solveTime = 2.0;
        solver.initialTimeStep = 0.25;
        solver.maximumTimeStep = 0.25;
        solver.saveEvery = 1;
        solver.outputPath = "custom_wizard_project.h5";
        model.setSolverOptions(solver);

        failIfFalse(model.isValid(), "Customized project wizard model should be valid.");

        const cppTDGL::Device customDevice = model.buildDevice();

        failIfFalse(customDevice.isValid(), "Customized wizard Device should be valid.");
        failIfFalse(customDevice.projectName() == "custom_wizard_project", "Custom project name should round trip into Device.");
        failIfFalse(customDevice.hasMesh(), "Customized wizard Device should include a mesh.");
        failIfFalse(customDevice.mesh().vertexCount() == 12, "5x1 mesh should have 12 vertices.");
        failIfFalse(customDevice.mesh().triangleCount() == 10, "5x1 mesh should have 10 triangles.");
        failIfNotNear(customDevice.mesh().totalArea(), 100.0, "Customized mesh area should match custom film area.");
        failIfFalse(customDevice.solverOptions().solveTime == 2.0, "Solver options should be transferred to Device.");

        cppTDGL::ProjectWizardGeometrySettings noMeshGeometry = geometry;
        noMeshGeometry.attachGeneratedMesh = false;
        model.setGeometrySettings(noMeshGeometry);

        const cppTDGL::Device noMeshDevice = model.buildDevice();

        failIfFalse(noMeshDevice.isValid(), "Wizard Device without generated mesh should still be valid.");
        failIfFalse(!noMeshDevice.hasMesh(), "Wizard Device should omit mesh when attachGeneratedMesh is false.");

        model.setProjectName("");
        failIfFalse(!model.isValid(), "Empty project name should invalidate wizard model.");

        bool invalidProjectThrew = false;
        try {
            static_cast<void>(model.buildDevice());
        } catch (const std::invalid_argument&) {
            invalidProjectThrew = true;
        }

        failIfFalse(invalidProjectThrew, "Building a Device from an invalid wizard model should throw.");

        model.setProjectName("invalid_mesh_options");
        cppTDGL::ProjectWizardGeometrySettings invalidMeshGeometry;
        invalidMeshGeometry.meshXDivisions = 0;
        invalidMeshGeometry.meshYDivisions = 1;
        model.setGeometrySettings(invalidMeshGeometry);

        failIfFalse(!model.isValid(), "Invalid mesh divisions should invalidate wizard model.");

        std::cout << "Project wizard model smoke test passed.\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "Project wizard model smoke test exception: " << error.what() << '\n';
        return 1;
    }
}
