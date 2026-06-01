#pragma once

#include "EdgeMesh.hpp"
#include "Mesh.hpp"

#include <cstddef>
#include <vector>

namespace cppTDGL {

struct EdgeTriangleAdjacency {
    std::size_t a = 0;
    std::size_t b = 0;
    std::vector<std::size_t> triangleIndices;

    [[nodiscard]] bool isBoundary() const noexcept;
    [[nodiscard]] bool isInterior() const noexcept;
};

class MeshOperators {
public:
    [[nodiscard]] static std::vector<Point2D> triangleCentroids(const Mesh& mesh);

    [[nodiscard]] static std::vector<double> vertexControlVolumes(const Mesh& mesh);

    [[nodiscard]] static std::vector<std::vector<std::size_t>> vertexToTriangleAdjacency(
        const Mesh& mesh
    );

    [[nodiscard]] static std::vector<EdgeTriangleAdjacency> edgeToTriangleAdjacency(
        const Mesh& mesh
    );

    [[nodiscard]] static std::vector<std::vector<std::size_t>> vertexToEdgeAdjacency(
        const EdgeMesh& edgeMesh,
        std::size_t vertexCount
    );

    [[nodiscard]] static bool finiteVolumeAreasAreConsistent(
        const Mesh& mesh,
        double tolerance = 1.0e-10
    );

    [[nodiscard]] static double sum(const std::vector<double>& values) noexcept;

private:
    static void validateMeshOrThrow(const Mesh& mesh);
};

} // namespace cppTDGL
