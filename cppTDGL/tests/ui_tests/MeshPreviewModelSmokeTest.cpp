#include "Device.hpp"
#include "EdgeMesh.hpp"
#include "Layer.hpp"
#include "Mesh.hpp"
#include "MeshOperators.hpp"
#include "MeshPreviewModel.hpp"
#include "Polygon2D.hpp"
#include "SolverOptions.hpp"

#include <cmath>
#include <iostream>
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

cppTDGL::Mesh makeSquareMesh() {
    const std::vector<cppTDGL::Point2D> vertices{
        {0.0, 0.0},
        {1.0, 0.0},
        {1.0, 1.0},
        {0.0, 1.0},
    };

    const std::vector<cppTDGL::Triangle> triangles{
        {0, 1, 2},
        {0, 2, 3},
    };

    const std::vector<std::size_t> boundaryVertices{
        0, 1, 2, 3,
    };

    return cppTDGL::Mesh(vertices, triangles, boundaryVertices);
}

cppTDGL::Device makeDevice(bool attachMesh) {
    cppTDGL::Layer layer(
        0.5,
        2.0,
        0.1,
        1.0,
        1.0,
        "um"
    );

    cppTDGL::SolverOptions options;
    options.solveTime = 10.0;
    options.initialTimeStep = 1.0e-3;
    options.maximumTimeStep = 0.05;
    options.saveEvery = 10;
    options.outputPath = "mesh_preview_model_solution.h5";

    cppTDGL::Device device(
        "mesh_preview_model_smoke_test",
        layer,
        cppTDGL::Polygon2D::rectangle("film", 10.0, 4.0),
        options
    );

    device.addProbePoint({0.0, 0.0});

    if (attachMesh) {
        device.setMesh(makeSquareMesh());
    }

    return device;
}

} // namespace

int main() {
    try {
        cppTDGL::MeshPreviewModel emptyModel;

        failIfFalse(!emptyModel.hasMesh(), "Empty preview model should not have a mesh.");
        failIfFalse(!emptyModel.isValid(), "Empty preview model should be invalid.");
        failIfFalse(emptyModel.summary().validationErrorCount == 1, "Empty preview model should report one summary error.");
        failIfFalse(emptyModel.vertexInfos().empty(), "Empty preview model should not expose vertex info.");
        failIfFalse(emptyModel.diagnosticSummary() == "No mesh loaded.", "Empty preview diagnostic should be stable.");

        bool emptyMeshThrew = false;
        try {
            static_cast<void>(emptyModel.mesh());
        } catch (const std::logic_error&) {
            emptyMeshThrew = true;
        }

        failIfFalse(emptyMeshThrew, "Accessing a missing preview mesh should throw.");

        cppTDGL::MeshPreviewModel model = cppTDGL::MeshPreviewModel::fromMesh(makeSquareMesh());

        failIfFalse(model.hasMesh(), "Preview model should report an attached mesh.");
        failIfFalse(model.isValid(), "Preview model with a valid mesh should be valid.");
        failIfFalse(model.diagnosticSummary() == "Mesh valid: 4 vertices, 2 triangles, 5 edges.", "Valid mesh diagnostic should be stable.");

        const cppTDGL::MeshPreviewSummary summary = model.summary();

        failIfFalse(summary.hasMesh, "Summary should report mesh presence.");
        failIfFalse(summary.isValid, "Summary should report valid mesh.");
        failIfFalse(summary.vertexCount == 4, "Square mesh should have 4 vertices.");
        failIfFalse(summary.triangleCount == 2, "Square mesh should have 2 triangles.");
        failIfFalse(summary.edgeCount == 5, "Square mesh should have 5 unique edges.");
        failIfFalse(summary.boundaryEdgeCount == 4, "Square mesh should have 4 boundary edges.");
        failIfFalse(summary.interiorEdgeCount == 1, "Square mesh should have 1 interior edge.");
        failIfNotNear(summary.totalArea, 1.0, "Square mesh total area should be 1.");
        failIfNotNear(summary.totalBoundaryLength, 4.0, "Square mesh boundary length should be 4.");
        failIfNotNear(summary.boundingBox.width(), 1.0, "Square mesh bounding-box width should be 1.");
        failIfNotNear(summary.boundingBox.height(), 1.0, "Square mesh bounding-box height should be 1.");

        const std::vector<cppTDGL::MeshPreviewVertexInfo> vertices = model.vertexInfos();
        failIfFalse(vertices.size() == 4, "Preview model should expose 4 vertex records.");

        double controlVolumeSum = 0.0;
        for (const cppTDGL::MeshPreviewVertexInfo& vertex : vertices) {
            failIfFalse(vertex.isBoundary, "All square mesh vertices should be boundary vertices.");
            controlVolumeSum += vertex.controlVolume;
        }

        failIfNotNear(controlVolumeSum, 1.0, "Vertex control volumes should sum to mesh area.");

        const std::vector<cppTDGL::MeshPreviewTriangleInfo> triangles = model.triangleInfos();
        failIfFalse(triangles.size() == 2, "Preview model should expose 2 triangle records.");
        failIfNotNear(triangles[0].area, 0.5, "First square mesh triangle area should be 0.5.");
        failIfNotNear(triangles[0].centroid.x, 2.0 / 3.0, "First triangle centroid x should be 2/3.");
        failIfNotNear(triangles[0].centroid.y, 1.0 / 3.0, "First triangle centroid y should be 1/3.");

        const std::vector<cppTDGL::MeshPreviewEdgeInfo> edges = model.edgeInfos();
        failIfFalse(edges.size() == 5, "Preview model should expose 5 edge records.");
        failIfFalse(model.boundaryEdgeInfos().size() == 4, "Preview model should expose 4 boundary edge records.");

        model.selectVertex(2);
        failIfFalse(model.selection().kind == cppTDGL::MeshPreviewSelectionKind::Vertex, "Vertex selection should set vertex kind.");
        failIfFalse(model.selection().index == 2, "Vertex selection index should be preserved.");

        model.selectTriangle(1);
        failIfFalse(model.selection().kind == cppTDGL::MeshPreviewSelectionKind::Triangle, "Triangle selection should set triangle kind.");
        failIfFalse(model.selection().index == 1, "Triangle selection index should be preserved.");

        model.selectEdge(4);
        failIfFalse(model.selection().kind == cppTDGL::MeshPreviewSelectionKind::Edge, "Edge selection should set edge kind.");
        failIfFalse(model.selection().index == 4, "Edge selection index should be preserved.");

        model.clearSelection();
        failIfFalse(!model.selection().hasSelection, "clearSelection() should clear the preview selection.");

        bool invalidSelectionThrew = false;
        try {
            model.selectVertex(100);
        } catch (const std::out_of_range&) {
            invalidSelectionThrew = true;
        }

        failIfFalse(invalidSelectionThrew, "Selecting an out-of-range vertex should throw.");

        const cppTDGL::Device deviceWithMesh = makeDevice(true);
        const cppTDGL::MeshPreviewModel fromDevice = cppTDGL::MeshPreviewModel::fromDevice(deviceWithMesh);

        failIfFalse(fromDevice.hasMesh(), "Preview model should load mesh from meshed Device.");
        failIfFalse(fromDevice.isValid(), "Preview model loaded from meshed Device should be valid.");
        failIfFalse(fromDevice.summary().edgeCount == 5, "Device mesh preview should preserve edge count.");

        const cppTDGL::Device deviceWithoutMesh = makeDevice(false);
        const cppTDGL::MeshPreviewModel fromNoMeshDevice = cppTDGL::MeshPreviewModel::fromDevice(deviceWithoutMesh);

        failIfFalse(!fromNoMeshDevice.hasMesh(), "Preview model from unmeshed Device should not have a mesh.");
        failIfFalse(!fromNoMeshDevice.isValid(), "Preview model from unmeshed Device should be invalid until mesh is attached.");

        model.clearMesh();
        failIfFalse(!model.hasMesh(), "clearMesh() should remove the preview mesh.");
        failIfFalse(!model.selection().hasSelection, "clearMesh() should clear the preview selection.");

        std::cout << "Mesh preview model smoke test passed.\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "Mesh preview model smoke test exception: " << error.what() << '\n';
        return 1;
    }
}
