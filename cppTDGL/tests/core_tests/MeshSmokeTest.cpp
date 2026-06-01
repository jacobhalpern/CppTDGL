#include "EdgeMesh.hpp"
#include "Mesh.hpp"

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

void printErrors(const std::vector<std::string>& errors) {
    for (const std::string& error : errors) {
        std::cerr << "  - " << error << '\n';
    }
}

} // namespace

int main() {
    try {
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

        cppTDGL::Mesh mesh(vertices, triangles, boundaryVertices);

        const std::vector<std::string> meshErrors = mesh.validationErrors();
        if (!meshErrors.empty()) {
            std::cerr << "Mesh validation failed:\n";
            printErrors(meshErrors);
            return 1;
        }

        failIfFalse(mesh.vertexCount() == 4, "Mesh should have 4 vertices.");
        failIfFalse(mesh.triangleCount() == 2, "Mesh should have 2 triangles.");
        failIfNotNear(mesh.triangleArea(0), 0.5, "First triangle area should be 0.5.");
        failIfNotNear(mesh.triangleArea(1), 0.5, "Second triangle area should be 0.5.");
        failIfNotNear(mesh.totalArea(), 1.0, "Total mesh area should be 1.0.");

        const cppTDGL::BoundingBox box = mesh.boundingBox();
        failIfNotNear(box.minimum.x, 0.0, "Bounding box minimum x should be 0.");
        failIfNotNear(box.minimum.y, 0.0, "Bounding box minimum y should be 0.");
        failIfNotNear(box.maximum.x, 1.0, "Bounding box maximum x should be 1.");
        failIfNotNear(box.maximum.y, 1.0, "Bounding box maximum y should be 1.");

        cppTDGL::EdgeMesh edgeMesh = cppTDGL::EdgeMesh::fromMesh(mesh);

        const std::vector<std::string> edgeErrors = edgeMesh.validationErrors();
        if (!edgeErrors.empty()) {
            std::cerr << "EdgeMesh validation failed:\n";
            printErrors(edgeErrors);
            return 1;
        }

        failIfFalse(edgeMesh.edgeCount() == 5, "Square mesh should have 5 unique edges.");
        failIfFalse(edgeMesh.boundaryEdgeCount() == 4, "Square mesh should have 4 boundary edges.");
        failIfFalse(edgeMesh.interiorEdgeCount() == 1, "Square mesh should have 1 interior edge.");
        failIfNotNear(edgeMesh.totalBoundaryLength(), 4.0, "Square boundary length should be 4.");

        std::cout << "Mesh smoke test passed.\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "Mesh smoke test exception: " << error.what() << '\n';
        return 1;
    }
}
