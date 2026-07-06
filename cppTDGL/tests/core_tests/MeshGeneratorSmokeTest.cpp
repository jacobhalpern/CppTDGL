#include "Device.hpp"
#include "EdgeMesh.hpp"
#include "Layer.hpp"
#include "Mesh.hpp"
#include "MeshGenerator.hpp"
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
    options.outputPath = "mesh_generator_solution.h5";

    cppTDGL::Polygon2D film = cppTDGL::Polygon2D::rectangle(
        "film",
        10.0,
        4.0
    );

    cppTDGL::Device device(
        "mesh_generator_smoke_test",
        layer,
        film,
        options
    );

    device.addProbePoint({0.0, 0.0});

    return device;
}

} // namespace

int main() {
    try {
        cppTDGL::MeshGenerationOptions options;
        options.xDivisions = 2;
        options.yDivisions = 3;

        failIfFalse(options.isValid(), "Valid mesh generation options should pass validation.");

        const cppTDGL::Mesh mesh = cppTDGL::MeshGenerator::generateRectangle(
            4.0,
            6.0,
            options,
            {1.0, -2.0}
        );

        failIfFalse(mesh.isValid(), "Generated rectangle mesh should be valid.");
        failIfFalse(mesh.vertexCount() == 12, "2x3 cell mesh should have 12 vertices.");
        failIfFalse(mesh.triangleCount() == 12, "2x3 cell mesh should have 12 triangles.");
        failIfFalse(mesh.boundaryVertexIndices().size() == 10, "2x3 cell mesh should have 10 boundary vertices.");
        failIfNotNear(mesh.totalArea(), 24.0, "Generated 4x6 rectangle mesh area should be 24.");

        const cppTDGL::BoundingBox bounds = mesh.boundingBox();

        failIfFalse(bounds.isValid(), "Generated mesh bounding box should be valid.");
        failIfNotNear(bounds.minimum.x, -1.0, "Generated mesh minimum x should match centered rectangle.");
        failIfNotNear(bounds.maximum.x, 3.0, "Generated mesh maximum x should match centered rectangle.");
        failIfNotNear(bounds.minimum.y, -5.0, "Generated mesh minimum y should match centered rectangle.");
        failIfNotNear(bounds.maximum.y, 1.0, "Generated mesh maximum y should match centered rectangle.");

        const cppTDGL::EdgeMesh edgeMesh = cppTDGL::EdgeMesh::fromMesh(mesh);

        failIfFalse(edgeMesh.isValid(), "Generated rectangle mesh should produce a valid EdgeMesh.");
        failIfFalse(edgeMesh.boundaryEdgeCount() == 10, "2x3 cell mesh should have 10 boundary edges.");
        failIfFalse(edgeMesh.interiorEdgeCount() == 13, "2x3 cell mesh should have 13 interior edges.");

        const cppTDGL::Device device = makeDevice();

        cppTDGL::MeshGenerationOptions deviceOptions;
        deviceOptions.xDivisions = 2;
        deviceOptions.yDivisions = 1;

        const cppTDGL::Mesh deviceMesh =
            cppTDGL::MeshGenerator::generateForDeviceFilmBoundingBox(device, deviceOptions);

        failIfFalse(deviceMesh.isValid(), "Generated device film-bounding-box mesh should be valid.");
        failIfFalse(deviceMesh.vertexCount() == 6, "2x1 device mesh should have 6 vertices.");
        failIfFalse(deviceMesh.triangleCount() == 4, "2x1 device mesh should have 4 triangles.");
        failIfNotNear(deviceMesh.totalArea(), 40.0, "Generated device mesh should match film bounding-box area.");

        const cppTDGL::Device meshedDevice =
            cppTDGL::MeshGenerator::attachGeneratedFilmBoundingBoxMesh(device, deviceOptions);

        failIfFalse(meshedDevice.hasMesh(), "Generated mesh should be attached to returned Device.");
        failIfFalse(meshedDevice.mesh().isValid(), "Attached generated mesh should be valid.");
        failIfFalse(meshedDevice.isValid(), "Device should remain valid after generated mesh attachment.");
        failIfNotNear(meshedDevice.mesh().totalArea(), 40.0, "Attached generated mesh area should match film bounding-box area.");

        cppTDGL::MeshGenerationOptions invalidOptions;
        invalidOptions.xDivisions = 0;
        invalidOptions.yDivisions = 1;

        failIfFalse(!invalidOptions.isValid(), "Zero xDivisions should invalidate mesh generation options.");

        bool invalidOptionsThrew = false;
        try {
            static_cast<void>(cppTDGL::MeshGenerator::generateRectangle(1.0, 1.0, invalidOptions));
        } catch (const std::invalid_argument&) {
            invalidOptionsThrew = true;
        }

        failIfFalse(invalidOptionsThrew, "Invalid mesh generation options should throw.");

        bool invalidWidthThrew = false;
        try {
            static_cast<void>(cppTDGL::MeshGenerator::generateRectangle(0.0, 1.0, options));
        } catch (const std::invalid_argument&) {
            invalidWidthThrew = true;
        }

        failIfFalse(invalidWidthThrew, "Invalid rectangle width should throw.");

        std::cout << "Mesh generator smoke test passed.\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "Mesh generator smoke test exception: " << error.what() << '\n';
        return 1;
    }
}
