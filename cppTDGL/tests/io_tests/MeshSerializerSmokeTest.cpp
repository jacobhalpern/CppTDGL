#include "EdgeMesh.hpp"
#include "Mesh.hpp"
#include "MeshSerializer.hpp"

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

} // namespace

int main() {
    try {
        const cppTDGL::Mesh original = makeSquareMesh();

        failIfFalse(original.isValid(), "Original mesh should be valid.");

        const std::filesystem::path meshPath =
            std::filesystem::temp_directory_path() / "CppTDGL_MeshSerializerSmokeTest.mesh";

        cppTDGL::MeshSerializer::save(original, meshPath);
        const cppTDGL::Mesh loaded = cppTDGL::MeshSerializer::load(meshPath);

        failIfFalse(loaded.isValid(), "Loaded mesh should be valid.");
        failIfFalse(loaded.vertexCount() == original.vertexCount(), "Vertex count should round trip.");
        failIfFalse(loaded.triangleCount() == original.triangleCount(), "Triangle count should round trip.");

        failIfFalse(
            loaded.boundaryVertexIndices().size() == original.boundaryVertexIndices().size(),
            "Boundary vertex count should round trip."
        );

        for (std::size_t i = 0; i < original.vertices().size(); ++i) {
            failIfNotNear(loaded.vertices()[i].x, original.vertices()[i].x, "Vertex x should round trip.");
            failIfNotNear(loaded.vertices()[i].y, original.vertices()[i].y, "Vertex y should round trip.");
        }

        for (std::size_t i = 0; i < original.triangles().size(); ++i) {
            failIfFalse(loaded.triangles()[i].a == original.triangles()[i].a, "Triangle a index should round trip.");
            failIfFalse(loaded.triangles()[i].b == original.triangles()[i].b, "Triangle b index should round trip.");
            failIfFalse(loaded.triangles()[i].c == original.triangles()[i].c, "Triangle c index should round trip.");
        }

        failIfNotNear(loaded.totalArea(), 1.0, "Loaded square mesh area should be 1.");

        const cppTDGL::EdgeMesh edgeMesh = cppTDGL::EdgeMesh::fromMesh(loaded);

        failIfFalse(edgeMesh.edgeCount() == 5, "Loaded square mesh should produce 5 unique edges.");
        failIfFalse(edgeMesh.boundaryEdgeCount() == 4, "Loaded square mesh should produce 4 boundary edges.");
        failIfFalse(edgeMesh.interiorEdgeCount() == 1, "Loaded square mesh should produce 1 interior edge.");

        std::filesystem::remove(meshPath);

        std::cout << "Mesh serializer smoke test passed.\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "Mesh serializer smoke test exception: " << error.what() << '\n';
        return 1;
    }
}
