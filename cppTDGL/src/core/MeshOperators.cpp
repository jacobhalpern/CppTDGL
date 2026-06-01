#include "MeshOperators.hpp"

#include <cmath>
#include <map>
#include <numeric>
#include <stdexcept>
#include <string>
#include <utility>

namespace cppTDGL {
namespace {

struct EdgeKey {
    std::size_t a = 0;
    std::size_t b = 0;

    [[nodiscard]] bool operator<(const EdgeKey& other) const noexcept {
        if (a != other.a) {
            return a < other.a;
        }
        return b < other.b;
    }
};

[[nodiscard]] EdgeKey makeEdgeKey(std::size_t first, std::size_t second) noexcept {
    if (first < second) {
        return {first, second};
    }

    return {second, first};
}

void addEdgeUse(
    std::map<EdgeKey, std::vector<std::size_t>>& edgeUses,
    std::size_t first,
    std::size_t second,
    std::size_t triangleIndex
) {
    edgeUses[makeEdgeKey(first, second)].push_back(triangleIndex);
}

} // namespace

bool EdgeTriangleAdjacency::isBoundary() const noexcept {
    return triangleIndices.size() == 1;
}

bool EdgeTriangleAdjacency::isInterior() const noexcept {
    return triangleIndices.size() == 2;
}

std::vector<Point2D> MeshOperators::triangleCentroids(const Mesh& mesh) {
    validateMeshOrThrow(mesh);

    std::vector<Point2D> centroids;
    centroids.reserve(mesh.triangleCount());

    for (const Triangle& triangle : mesh.triangles()) {
        const Point2D& a = mesh.vertices()[triangle.a];
        const Point2D& b = mesh.vertices()[triangle.b];
        const Point2D& c = mesh.vertices()[triangle.c];

        centroids.push_back({
            (a.x + b.x + c.x) / 3.0,
            (a.y + b.y + c.y) / 3.0,
        });
    }

    return centroids;
}

std::vector<double> MeshOperators::vertexControlVolumes(const Mesh& mesh) {
    validateMeshOrThrow(mesh);

    std::vector<double> controlVolumes(mesh.vertexCount(), 0.0);

    for (std::size_t triangleIndex = 0; triangleIndex < mesh.triangleCount(); ++triangleIndex) {
        const Triangle& triangle = mesh.triangles()[triangleIndex];
        const double contribution = mesh.triangleArea(triangleIndex) / 3.0;

        controlVolumes[triangle.a] += contribution;
        controlVolumes[triangle.b] += contribution;
        controlVolumes[triangle.c] += contribution;
    }

    return controlVolumes;
}

std::vector<std::vector<std::size_t>> MeshOperators::vertexToTriangleAdjacency(
    const Mesh& mesh
) {
    validateMeshOrThrow(mesh);

    std::vector<std::vector<std::size_t>> adjacency(mesh.vertexCount());

    for (std::size_t triangleIndex = 0; triangleIndex < mesh.triangleCount(); ++triangleIndex) {
        const Triangle& triangle = mesh.triangles()[triangleIndex];

        adjacency[triangle.a].push_back(triangleIndex);
        adjacency[triangle.b].push_back(triangleIndex);
        adjacency[triangle.c].push_back(triangleIndex);
    }

    return adjacency;
}

std::vector<EdgeTriangleAdjacency> MeshOperators::edgeToTriangleAdjacency(
    const Mesh& mesh
) {
    validateMeshOrThrow(mesh);

    std::map<EdgeKey, std::vector<std::size_t>> edgeUses;

    for (std::size_t triangleIndex = 0; triangleIndex < mesh.triangleCount(); ++triangleIndex) {
        const Triangle& triangle = mesh.triangles()[triangleIndex];

        addEdgeUse(edgeUses, triangle.a, triangle.b, triangleIndex);
        addEdgeUse(edgeUses, triangle.b, triangle.c, triangleIndex);
        addEdgeUse(edgeUses, triangle.c, triangle.a, triangleIndex);
    }

    std::vector<EdgeTriangleAdjacency> adjacency;
    adjacency.reserve(edgeUses.size());

    for (auto& [edge, triangleIndices] : edgeUses) {
        adjacency.push_back({
            edge.a,
            edge.b,
            std::move(triangleIndices),
        });
    }

    return adjacency;
}

std::vector<std::vector<std::size_t>> MeshOperators::vertexToEdgeAdjacency(
    const EdgeMesh& edgeMesh,
    std::size_t vertexCount
) {
    if (!edgeMesh.isValid()) {
        throw std::invalid_argument("Cannot compute vertex-to-edge adjacency from an invalid EdgeMesh.");
    }

    std::vector<std::vector<std::size_t>> adjacency(vertexCount);

    for (std::size_t edgeIndex = 0; edgeIndex < edgeMesh.edgeCount(); ++edgeIndex) {
        const MeshEdge& edge = edgeMesh.edges()[edgeIndex];

        if (edge.a >= vertexCount || edge.b >= vertexCount) {
            throw std::invalid_argument("EdgeMesh contains a vertex index outside the requested vertex count.");
        }

        adjacency[edge.a].push_back(edgeIndex);
        adjacency[edge.b].push_back(edgeIndex);
    }

    return adjacency;
}

bool MeshOperators::finiteVolumeAreasAreConsistent(
    const Mesh& mesh,
    double tolerance
) {
    validateMeshOrThrow(mesh);

    const std::vector<double> controlVolumes = vertexControlVolumes(mesh);
    return std::abs(sum(controlVolumes) - mesh.totalArea()) <= tolerance;
}

double MeshOperators::sum(const std::vector<double>& values) noexcept {
    return std::accumulate(values.begin(), values.end(), 0.0);
}

void MeshOperators::validateMeshOrThrow(const Mesh& mesh) {
    const std::vector<std::string> errors = mesh.validationErrors();

    if (!errors.empty()) {
        throw std::invalid_argument("Cannot build mesh operators from an invalid Mesh.");
    }
}

} // namespace cppTDGL
