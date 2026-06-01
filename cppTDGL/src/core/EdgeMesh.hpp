#pragma once

#include "Mesh.hpp"

#include <cstddef>
#include <string>
#include <vector>

namespace cppTDGL {

struct MeshEdge {
    std::size_t a = 0;
    std::size_t b = 0;
    Point2D center{};
    double length = 0.0;
    std::size_t triangleUseCount = 0;
    bool isBoundary = false;
};

class EdgeMesh {
public:
    EdgeMesh() = default;
    explicit EdgeMesh(std::vector<MeshEdge> edges);

    [[nodiscard]] static EdgeMesh fromMesh(const Mesh& mesh);

    [[nodiscard]] const std::vector<MeshEdge>& edges() const noexcept;
    [[nodiscard]] std::size_t edgeCount() const noexcept;
    [[nodiscard]] std::size_t boundaryEdgeCount() const noexcept;
    [[nodiscard]] std::size_t interiorEdgeCount() const noexcept;
    [[nodiscard]] double totalBoundaryLength() const noexcept;

    [[nodiscard]] bool isValid() const;
    [[nodiscard]] std::vector<std::string> validationErrors() const;

private:
    std::vector<MeshEdge> edges_;
};

} // namespace cppTDGL
