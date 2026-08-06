#include "EdgeMesh.hpp"
#include "GeometryEditorModel.hpp"
#include "MeshGenerator.hpp"

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
        cppTDGL::GeometryEditorModel model = cppTDGL::GeometryEditorModel::createDefault();

        failIfFalse(model.isValid(), "Default geometry editor model should be valid.");
        failIfFalse(model.projectName() == "cppTDGL_geometry_editor_project", "Default project name should be stable.");
        failIfFalse(model.holeCount() == 0, "Default model should start without holes.");
        failIfFalse(model.terminalCount() == 0, "Default model should start without terminals.");
        failIfFalse(model.probePointCount() == 0, "Default model should start without probe points.");
        failIfFalse(!model.selection().hasSelection, "Default model should start without a selection.");
        failIfNotNear(model.film().area(), 40.0, "Default film area should be 40.");

        model.selectFilm();
        failIfFalse(model.selection().hasSelection, "Selecting the film should produce a selection.");
        failIfFalse(
            model.selection().kind == cppTDGL::GeometryEditorSelectionKind::Film,
            "Film selection kind should be Film."
        );

        model.addRectangularHole("hole_a", 1.0, 0.5, {1.0, 0.0});
        model.addCircularHole("hole_b", 0.25, 16, {-1.0, 0.0});

        failIfFalse(model.holeCount() == 2, "Two holes should have been added.");
        failIfFalse(
            model.selection().kind == cppTDGL::GeometryEditorSelectionKind::Hole && model.selection().index == 1,
            "Adding a hole should select the added hole."
        );

        model.removeHole(1);
        failIfFalse(model.holeCount() == 1, "Removing a hole should reduce hole count.");
        failIfFalse(!model.selection().hasSelection, "Removing a selected item should clear selection.");

        model.addRectangularTerminal("terminal_left", 1.0, 0.5, {-4.0, 0.0});
        model.addProbePoint({0.0, 1.0});

        failIfFalse(model.terminalCount() == 1, "One terminal should have been added.");
        failIfFalse(model.probePointCount() == 1, "One probe point should have been added.");
        failIfFalse(model.isValid(), "Model with valid geometry items should remain valid.");

        model.selectTerminal(0);
        failIfFalse(
            model.selection().kind == cppTDGL::GeometryEditorSelectionKind::Terminal && model.selection().index == 0,
            "Terminal selection should preserve index."
        );

        model.selectProbePoint(0);
        failIfFalse(
            model.selection().kind == cppTDGL::GeometryEditorSelectionKind::ProbePoint && model.selection().index == 0,
            "Probe selection should preserve index."
        );

        const cppTDGL::Device noMeshDevice = model.buildDevice(false);

        failIfFalse(noMeshDevice.isValid(), "Geometry editor should build a valid Device without mesh.");
        failIfFalse(!noMeshDevice.hasMesh(), "No-mesh build should not attach a mesh.");
        failIfFalse(noMeshDevice.holes().size() == 1, "Hole count should transfer into Device.");
        failIfFalse(noMeshDevice.terminals().size() == 1, "Terminal count should transfer into Device.");
        failIfFalse(noMeshDevice.probePoints().size() == 1, "Probe point count should transfer into Device.");

        cppTDGL::MeshGenerationOptions meshOptions;
        meshOptions.xDivisions = 2;
        meshOptions.yDivisions = 1;

        const cppTDGL::Device meshedDevice = model.buildDevice(true, meshOptions);

        failIfFalse(meshedDevice.isValid(), "Geometry editor should build a valid meshed Device.");
        failIfFalse(meshedDevice.hasMesh(), "Meshed build should attach a generated mesh.");
        failIfFalse(meshedDevice.mesh().vertexCount() == 6, "2x1 generated mesh should have 6 vertices.");
        failIfFalse(meshedDevice.mesh().triangleCount() == 4, "2x1 generated mesh should have 4 triangles.");
        failIfNotNear(meshedDevice.mesh().totalArea(), 40.0, "Generated mesh area should match film area.");

        const cppTDGL::EdgeMesh edgeMesh = cppTDGL::EdgeMesh::fromMesh(meshedDevice.mesh());
        failIfFalse(edgeMesh.isValid(), "Generated mesh should produce a valid EdgeMesh.");

        cppTDGL::GeometryEditorModel fromDevice = cppTDGL::GeometryEditorModel::fromDevice(noMeshDevice);
        failIfFalse(fromDevice.isValid(), "Model reconstructed from Device should be valid.");
        failIfFalse(fromDevice.projectName() == noMeshDevice.projectName(), "Project name should round trip from Device.");
        failIfFalse(fromDevice.holeCount() == noMeshDevice.holes().size(), "Hole count should round trip from Device.");
        failIfFalse(fromDevice.terminalCount() == noMeshDevice.terminals().size(), "Terminal count should round trip from Device.");
        failIfFalse(fromDevice.probePointCount() == noMeshDevice.probePoints().size(), "Probe count should round trip from Device.");

        bool removeHoleThrew = false;
        try {
            model.removeHole(5);
        } catch (const std::out_of_range&) {
            removeHoleThrew = true;
        }

        failIfFalse(removeHoleThrew, "Removing an out-of-range hole should throw.");

        cppTDGL::GeometryEditorModel invalidProbeModel = cppTDGL::GeometryEditorModel::createDefault();
        invalidProbeModel.addRectangularHole("center_hole", 1.0, 1.0, {0.0, 0.0});
        invalidProbeModel.addProbePoint({0.0, 0.0});

        failIfFalse(!invalidProbeModel.isValid(), "Probe point inside a hole should invalidate the model.");

        bool invalidBuildThrew = false;
        try {
            static_cast<void>(invalidProbeModel.buildDevice(false));
        } catch (const std::invalid_argument&) {
            invalidBuildThrew = true;
        }

        failIfFalse(invalidBuildThrew, "Building from invalid geometry should throw.");

        invalidProbeModel.clearHoles();
        invalidProbeModel.clearProbePoints();
        invalidProbeModel.setProjectName("");

        failIfFalse(!invalidProbeModel.isValid(), "Empty project name should invalidate the editor model.");

        std::cout << "Geometry editor model smoke test passed.\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "Geometry editor model smoke test exception: " << error.what() << '\n';
        return 1;
    }
}
