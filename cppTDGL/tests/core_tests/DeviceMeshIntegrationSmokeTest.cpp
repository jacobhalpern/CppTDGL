#include "Device.hpp"
#include "EdgeMesh.hpp"
#include "Layer.hpp"
#include "Mesh.hpp"
#include "Polygon2D.hpp"
#include "SolverOptions.hpp"

#include <algorithm>
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

cppTDGL::Device makeDevice() {
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
    options.outputPath = "device_mesh_integration_solution.h5";

    cppTDGL::Polygon2D film = cppTDGL::Polygon2D::rectangle(
        "film",
        10.0,
        4.0
    );

    cppTDGL::Device device(
        "device_mesh_integration_smoke_test",
        layer,
        film,
        options
    );

    device.addProbePoint({0.0, 0.0});

    return device;
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

cppTDGL::Mesh makeInvalidMesh() {
    const std::vector<cppTDGL::Point2D> vertices{
        {0.0, 0.0},
        {1.0, 0.0},
        {0.0, 1.0},
    };

    const std::vector<cppTDGL::Triangle> triangles{
        {0, 0, 1},
    };

    const std::vector<std::size_t> boundaryVertices{
        0, 1, 2,
    };

    return cppTDGL::Mesh(vertices, triangles, boundaryVertices);
}

bool containsPrefix(const std::vector<std::string>& values, const std::string& prefix) {
    return std::any_of(
        values.begin(),
        values.end(),
        [&prefix](const std::string& value) {
            return value.rfind(prefix, 0) == 0;
        }
    );
}

} // namespace

int main() {
    try {
        cppTDGL::Device device = makeDevice();

        failIfFalse(device.isValid(), "Device should be valid before mesh attachment.");
        failIfFalse(device.hasValidGeometry(), "Device geometry should be valid before mesh attachment.");
        failIfFalse(!device.hasMesh(), "Device should start without an attached mesh.");

        bool meshAccessThrew = false;
        try {
            static_cast<void>(device.mesh());
        } catch (const std::logic_error&) {
            meshAccessThrew = true;
        }

        failIfFalse(meshAccessThrew, "Accessing a missing mesh should throw std::logic_error.");

        device.setMesh(makeSquareMesh());

        failIfFalse(device.hasMesh(), "Device should report an attached mesh after setMesh().");
        failIfFalse(device.mesh().isValid(), "Attached mesh should be valid.");
        failIfFalse(device.isValid(), "Device should remain valid with a valid attached mesh.");
        failIfFalse(device.hasValidGeometry(), "Device geometry should remain valid with a valid attached mesh.");
        failIfFalse(device.mesh().vertexCount() == 4, "Attached mesh should preserve vertex count.");
        failIfFalse(device.mesh().triangleCount() == 2, "Attached mesh should preserve triangle count.");
        failIfNotNear(device.mesh().totalArea(), 1.0, "Attached square mesh area should be 1.");

        const cppTDGL::Device& constDevice = device;
        failIfFalse(constDevice.mesh().vertexCount() == 4, "Const mesh accessor should return the attached mesh.");

        const cppTDGL::EdgeMesh edgeMesh = cppTDGL::EdgeMesh::fromMesh(device.mesh());

        failIfFalse(edgeMesh.edgeCount() == 5, "Attached square mesh should produce 5 unique edges.");
        failIfFalse(edgeMesh.boundaryEdgeCount() == 4, "Attached square mesh should produce 4 boundary edges.");
        failIfFalse(edgeMesh.interiorEdgeCount() == 1, "Attached square mesh should produce 1 interior edge.");

        device.clearMesh();

        failIfFalse(!device.hasMesh(), "Device should not report a mesh after clearMesh().");
        failIfFalse(device.isValid(), "Device should remain valid after mesh removal.");
        failIfFalse(device.hasValidGeometry(), "Device geometry should remain valid after mesh removal.");

        cppTDGL::Device invalidMeshDevice = makeDevice();
        invalidMeshDevice.setMesh(makeInvalidMesh());

        failIfFalse(invalidMeshDevice.hasMesh(), "Invalid-mesh device should still report mesh ownership.");
        failIfFalse(!invalidMeshDevice.mesh().isValid(), "Invalid attached mesh should be invalid.");
        failIfFalse(!invalidMeshDevice.isValid(), "Device validation should fail with an invalid attached mesh.");
        failIfFalse(!invalidMeshDevice.hasValidGeometry(), "Device geometry validation should fail with an invalid attached mesh.");

        const std::vector<std::string> errors = invalidMeshDevice.validationErrors();
        failIfFalse(containsPrefix(errors, "Mesh: "), "Invalid attached mesh should produce mesh-prefixed validation errors.");

        std::cout << "Device mesh integration smoke test passed.\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "Device mesh integration smoke test exception: " << error.what() << '\n';
        return 1;
    }
}
