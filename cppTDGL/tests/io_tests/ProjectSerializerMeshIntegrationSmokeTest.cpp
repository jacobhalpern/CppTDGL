#include "Device.hpp"
#include "EdgeMesh.hpp"
#include "Layer.hpp"
#include "Mesh.hpp"
#include "ProjectSerializer.hpp"
#include "Polygon2D.hpp"
#include "SolverOptions.hpp"

#include <cmath>
#include <filesystem>
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
    options.solveTime = 12.0;
    options.initialTimeStep = 1.0e-3;
    options.maximumTimeStep = 0.05;
    options.saveEvery = 12;
    options.outputPath = "project_serializer_mesh_solution.h5";

    cppTDGL::Polygon2D film = cppTDGL::Polygon2D::rectangle(
        "film",
        10.0,
        4.0
    );

    cppTDGL::Device device(
        "project_serializer_mesh_integration",
        layer,
        film,
        options
    );

    device.addHole(cppTDGL::Polygon2D::circleApproximation("hole", 0.5, 16, {1.0, 0.0}));
    device.addTerminal(cppTDGL::Polygon2D::rectangle("terminal", 1.0, 0.5, {-4.0, 0.0}));
    device.addProbePoint({0.0, 0.0});

    if (attachMesh) {
        device.setMesh(makeSquareMesh());
    }

    return device;
}

} // namespace

int main() {
    try {
        const std::filesystem::path meshProjectPath =
            std::filesystem::temp_directory_path() / "CppTDGL_ProjectSerializerMeshIntegration_WithMesh.cpptdgl";

        const cppTDGL::Device originalWithMesh = makeDevice(true);

        failIfFalse(originalWithMesh.isValid(), "Original device with mesh should be valid.");
        failIfFalse(originalWithMesh.hasMesh(), "Original device should have a mesh.");

        cppTDGL::ProjectSerializer::save(originalWithMesh, meshProjectPath);
        const cppTDGL::Device loadedWithMesh =
            cppTDGL::ProjectSerializer::load(meshProjectPath);

        failIfFalse(loadedWithMesh.isValid(), "Loaded device with mesh should be valid.");
        failIfFalse(loadedWithMesh.hasMesh(), "Loaded device should preserve mesh presence.");
        failIfFalse(loadedWithMesh.projectName() == originalWithMesh.projectName(), "Project name should round trip.");
        failIfFalse(loadedWithMesh.holes().size() == 1, "Hole count should round trip.");
        failIfFalse(loadedWithMesh.terminals().size() == 1, "Terminal count should round trip.");
        failIfFalse(loadedWithMesh.probePoints().size() == 1, "Probe point count should round trip.");

        failIfFalse(loadedWithMesh.mesh().isValid(), "Loaded mesh should be valid.");
        failIfFalse(loadedWithMesh.mesh().vertexCount() == 4, "Loaded mesh vertex count should round trip.");
        failIfFalse(loadedWithMesh.mesh().triangleCount() == 2, "Loaded mesh triangle count should round trip.");
        failIfFalse(loadedWithMesh.mesh().boundaryVertexIndices().size() == 4, "Loaded mesh boundary count should round trip.");
        failIfNotNear(loadedWithMesh.mesh().totalArea(), 1.0, "Loaded mesh area should round trip.");

        const cppTDGL::EdgeMesh edgeMesh = cppTDGL::EdgeMesh::fromMesh(loadedWithMesh.mesh());

        failIfFalse(edgeMesh.edgeCount() == 5, "Loaded mesh should produce 5 unique edges.");
        failIfFalse(edgeMesh.boundaryEdgeCount() == 4, "Loaded mesh should produce 4 boundary edges.");
        failIfFalse(edgeMesh.interiorEdgeCount() == 1, "Loaded mesh should produce 1 interior edge.");

        const std::filesystem::path noMeshProjectPath =
            std::filesystem::temp_directory_path() / "CppTDGL_ProjectSerializerMeshIntegration_NoMesh.cpptdgl";

        const cppTDGL::Device originalWithoutMesh = makeDevice(false);

        failIfFalse(originalWithoutMesh.isValid(), "Original device without mesh should be valid.");
        failIfFalse(!originalWithoutMesh.hasMesh(), "Original device should not have a mesh.");

        cppTDGL::ProjectSerializer::save(originalWithoutMesh, noMeshProjectPath);
        const cppTDGL::Device loadedWithoutMesh =
            cppTDGL::ProjectSerializer::load(noMeshProjectPath);

        failIfFalse(loadedWithoutMesh.isValid(), "Loaded device without mesh should be valid.");
        failIfFalse(!loadedWithoutMesh.hasMesh(), "Loaded device should preserve missing mesh state.");
        failIfFalse(loadedWithoutMesh.projectName() == originalWithoutMesh.projectName(), "No-mesh project name should round trip.");
        failIfFalse(loadedWithoutMesh.holes().size() == 1, "No-mesh hole count should round trip.");
        failIfFalse(loadedWithoutMesh.terminals().size() == 1, "No-mesh terminal count should round trip.");
        failIfFalse(loadedWithoutMesh.probePoints().size() == 1, "No-mesh probe point count should round trip.");

        std::filesystem::remove(meshProjectPath);
        std::filesystem::remove(noMeshProjectPath);

        std::cout << "Project serializer mesh integration smoke test passed.\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "Project serializer mesh integration smoke test exception: " << error.what() << '\n';
        return 1;
    }
}
