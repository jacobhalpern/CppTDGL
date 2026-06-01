#include "EdgeMesh.hpp"

#include <algorithm>
#include <cmath>
#include <map>
#include <stdexcept>
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

void addTriangleEdge(
    std::map<EdgeKey, std::size_t>& edgeUseCounts,
    std::size_t first,
    std::size_t second
) {
    ++edgeUseCounts[makeEdgeKey(first, second)];
}

[[nodiscard]] double distanceBetween(const Point2D& a, const Point2D& b) noexcept {
    const double dx = b.x - a.x;
    const double dy = b.y - a.y;
    return std::sqrt(dx * dx + dy * dy);
}

[[nodiscard]] Point2D midpoint(const Point2D& a, const Point2D& b) noexcept {
    return {
        0.5 * (a.x + b.x),
        0.5 * (a.y + b.y),
    };
}

} // namespace

EdgeMesh::EdgeMesh(std::vector<MeshEdge> edges)
    : edges_(std::move(edges)) {}

EdgeMesh EdgeMesh::fromMesh(const Mesh& mesh) {
    const std::vector<std::string> meshErrors = mesh.validationErrors();
    if (!meshErrors.empty()) {
        throw std::invalid_argument("Cannot create EdgeMesh from an invalid Mesh.");
    }

    std::map<EdgeKey, std::size_t> edgeUseCounts;

    for (const Triangle& triangle : mesh.triangles()) {
        addTriangleEdge(edgeUseCounts, triangle.a, triangle.b);
        addTriangleEdge(edgeUseCounts, triangle.b, triangle.c);
        addTriangleEdge(edgeUseCounts, triangle.c, triangle.a);
    }

    std::vector<MeshEdge> edges;
    edges.reserve(edgeUseCounts.size());

    for (const auto& [key, useCount] : edgeUseCounts) {
        const Point2D& first = mesh.vertices()[key.a];
        const Point2D& second = mesh.vertices()[key.b];

        edges.push_back({
            key.a,
            key.b,
            midpoint(first, second),
            distanceBetween(first, second),
            useCount,
            useCount == 1,
        });
    }

    return EdgeMesh(std::move(edges));
}

const std::vector<MeshEdge>& EdgeMesh::edges() const noexcept {
    return edges_;
}

std::size_t EdgeMesh::edgeCount() const noexcept {
    return edges_.size();
}

std::size_t EdgeMesh::boundaryEdgeCount() const noexcept {
    return static_cast<std::size_t>(
        std::count_if(
            edges_.begin(),
            edges_.end(),
            [](const MeshEdge& edge) { return edge.isBoundary; }
        )
    );
}

std::size_t EdgeMesh::interiorEdgeCount() const noexcept {
    return edgeCount() - boundaryEdgeCount();
}

double EdgeMesh::totalBoundaryLength() const noexcept {
    double total = 0.0;

    for (const MeshEdge& edge : edges_) {
        if (edge.isBoundary) {
            total += edge.length;
        }
    }

    return total;
}

bool EdgeMesh::isValid() const {
    return validationErrors().empty();
}

std::vector<std::string> EdgeMesh::validationErrors() const {
    std::vector<std::string> errors;

    if (edges_.empty()) {
        errors.emplace_back("EdgeMesh must contain at least one edge.");
    }

    for (std::size_t i = 0; i < edges_.size(); ++i) {
        const MeshEdge& edge = edges_[i];

        if (edge.a == edge.b) {
            errors.emplace_back("Edge " + std::to_string(i) + " has duplicate vertex indices.");
        }

        if (edge.length <= 0.0) {
            errors.emplace_back("Edge " + std::to_string(i) + " must have positive length.");
        }

        if (edge.triangleUseCount == 0) {
            errors.emplace_back("Edge " + std::to_string(i) + " must be used by at least one triangle.");
        }

        if (edge.isBoundary != (edge.triangleUseCount == 1)) {
            errors.emplace_back("Edge " + std::to_string(i) + " has inconsistent boundary state.");
        }
    }

    return errors;
}

} // namespace cppTDGL
