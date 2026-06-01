#include "EdgeMesh.hpp"
#include "Mesh.hpp"
#include "MeshOperators.hpp"

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

} // namespace

int main() {
    try {
        const cppTDGL::Mesh mesh = makeSquareMesh();

        failIfFalse(mesh.isValid(), "Square mesh should be valid.");

        const std::vector<cppTDGL::Point2D> centroids =
            cppTDGL::MeshOperators::triangleCentroids(mesh);

        failIfFalse(centroids.size() == 2, "Square mesh should have 2 triangle centroids.");

        failIfNotNear(centroids[0].x, 2.0 / 3.0, "First centroid x should be 2/3.");
        failIfNotNear(centroids[0].y, 1.0 / 3.0, "First centroid y should be 1/3.");
        failIfNotNear(centroids[1].x, 1.0 / 3.0, "Second centroid x should be 1/3.");
        failIfNotNear(centroids[1].y, 2.0 / 3.0, "Second centroid y should be 2/3.");

        const std::vector<double> controlVolumes =
            cppTDGL::MeshOperators::vertexControlVolumes(mesh);

        failIfFalse(controlVolumes.size() == 4, "Square mesh should have 4 control-volume values.");

        failIfNotNear(controlVolumes[0], 1.0 / 3.0, "Vertex 0 control volume should be 1/3.");
        failIfNotNear(controlVolumes[1], 1.0 / 6.0, "Vertex 1 control volume should be 1/6.");
        failIfNotNear(controlVolumes[2], 1.0 / 3.0, "Vertex 2 control volume should be 1/3.");
        failIfNotNear(controlVolumes[3], 1.0 / 6.0, "Vertex 3 control volume should be 1/6.");
        failIfNotNear(cppTDGL::MeshOperators::sum(controlVolumes), 1.0, "Control volumes should sum to total area.");

        failIfFalse(
            cppTDGL::MeshOperators::finiteVolumeAreasAreConsistent(mesh),
            "Finite-volume control areas should be consistent with mesh total area."
        );

        const std::vector<std::vector<std::size_t>> vertexToTriangle =
            cppTDGL::MeshOperators::vertexToTriangleAdjacency(mesh);

        failIfFalse(vertexToTriangle.size() == 4, "Vertex-to-triangle adjacency should have 4 entries.");
        failIfFalse(vertexToTriangle[0].size() == 2, "Vertex 0 should touch 2 triangles.");
        failIfFalse(vertexToTriangle[1].size() == 1, "Vertex 1 should touch 1 triangle.");
        failIfFalse(vertexToTriangle[2].size() == 2, "Vertex 2 should touch 2 triangles.");
        failIfFalse(vertexToTriangle[3].size() == 1, "Vertex 3 should touch 1 triangle.");

        const std::vector<cppTDGL::EdgeTriangleAdjacency> edgeToTriangle =
            cppTDGL::MeshOperators::edgeToTriangleAdjacency(mesh);

        std::size_t boundaryEdgeCount = 0;
        std::size_t interiorEdgeCount = 0;

        for (const cppTDGL::EdgeTriangleAdjacency& edge : edgeToTriangle) {
            if (edge.isBoundary()) {
                ++boundaryEdgeCount;
            }
            if (edge.isInterior()) {
                ++interiorEdgeCount;
            }
        }

        failIfFalse(edgeToTriangle.size() == 5, "Square mesh should have 5 edge-to-triangle entries.");
        failIfFalse(boundaryEdgeCount == 4, "Square mesh should have 4 boundary edge adjacencies.");
        failIfFalse(interiorEdgeCount == 1, "Square mesh should have 1 interior edge adjacency.");

        const cppTDGL::EdgeMesh edgeMesh = cppTDGL::EdgeMesh::fromMesh(mesh);

        const std::vector<std::vector<std::size_t>> vertexToEdge =
            cppTDGL::MeshOperators::vertexToEdgeAdjacency(edgeMesh, mesh.vertexCount());

        failIfFalse(vertexToEdge.size() == 4, "Vertex-to-edge adjacency should have 4 entries.");
        failIfFalse(vertexToEdge[0].size() == 3, "Vertex 0 should touch 3 edges.");
        failIfFalse(vertexToEdge[1].size() == 2, "Vertex 1 should touch 2 edges.");
        failIfFalse(vertexToEdge[2].size() == 3, "Vertex 2 should touch 3 edges.");
        failIfFalse(vertexToEdge[3].size() == 2, "Vertex 3 should touch 2 edges.");

        std::cout << "Mesh operators smoke test passed.\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "Mesh operators smoke test exception: " << error.what() << '\n';
        return 1;
    }
}
