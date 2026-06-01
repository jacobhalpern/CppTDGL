#include "Mesh.hpp"

#include <algorithm>
#include <cmath>
#include <limits>
#include <stdexcept>
#include <unordered_set>
#include <utility>

namespace cppTDGL {
namespace {

constexpr double kAreaTolerance = 1.0e-12;

[[nodiscard]] double signedAreaFromPoints(
    const Point2D& a,
    const Point2D& b,
    const Point2D& c
) noexcept {
    return 0.5 * (
        (b.x - a.x) * (c.y - a.y) -
        (c.x - a.x) * (b.y - a.y)
    );
}

} // namespace

Mesh::Mesh(
    std::vector<Point2D> vertices,
    std::vector<Triangle> triangles,
    std::vector<std::size_t> boundaryVertexIndices
)
    : vertices_(std::move(vertices)),
      triangles_(std::move(triangles)),
      boundaryVertexIndices_(std::move(boundaryVertexIndices)) {}

const std::vector<Point2D>& Mesh::vertices() const noexcept {
    return vertices_;
}

const std::vector<Triangle>& Mesh::triangles() const noexcept {
    return triangles_;
}

const std::vector<std::size_t>& Mesh::boundaryVertexIndices() const noexcept {
    return boundaryVertexIndices_;
}

void Mesh::setVertices(std::vector<Point2D> value) {
    vertices_ = std::move(value);
}

void Mesh::setTriangles(std::vector<Triangle> value) {
    triangles_ = std::move(value);
}

void Mesh::setBoundaryVertexIndices(std::vector<std::size_t> value) {
    boundaryVertexIndices_ = std::move(value);
}

std::size_t Mesh::vertexCount() const noexcept {
    return vertices_.size();
}

std::size_t Mesh::triangleCount() const noexcept {
    return triangles_.size();
}

bool Mesh::isEmpty() const noexcept {
    return vertices_.empty() && triangles_.empty();
}

double Mesh::triangleArea(std::size_t triangleIndex) const {
    if (triangleIndex >= triangles_.size()) {
        throw std::out_of_range("Triangle index is out of range.");
    }

    return std::abs(signedTriangleArea(triangles_[triangleIndex]));
}

std::vector<double> Mesh::triangleAreas() const {
    std::vector<double> areas;
    areas.reserve(triangles_.size());

    for (std::size_t i = 0; i < triangles_.size(); ++i) {
        areas.push_back(triangleArea(i));
    }

    return areas;
}

double Mesh::totalArea() const {
    double sum = 0.0;

    for (std::size_t i = 0; i < triangles_.size(); ++i) {
        sum += triangleArea(i);
    }

    return sum;
}

BoundingBox Mesh::boundingBox() const {
    if (vertices_.empty()) {
        throw std::logic_error("Cannot compute bounding box for a mesh with no vertices.");
    }

    BoundingBox box{
        {std::numeric_limits<double>::max(), std::numeric_limits<double>::max()},
        {std::numeric_limits<double>::lowest(), std::numeric_limits<double>::lowest()},
    };

    for (const Point2D& point : vertices_) {
        box.minimum.x = std::min(box.minimum.x, point.x);
        box.minimum.y = std::min(box.minimum.y, point.y);
        box.maximum.x = std::max(box.maximum.x, point.x);
        box.maximum.y = std::max(box.maximum.y, point.y);
    }

    return box;
}

bool Mesh::isValid() const {
    return validationErrors().empty();
}

std::vector<std::string> Mesh::validationErrors() const {
    std::vector<std::string> errors;

    if (vertices_.size() < 3) {
        errors.emplace_back("Mesh must contain at least three vertices.");
    }

    if (triangles_.empty()) {
        errors.emplace_back("Mesh must contain at least one triangle.");
    }

    for (std::size_t i = 0; i < triangles_.size(); ++i) {
        const Triangle& triangle = triangles_[i];

        if (!triangleIndicesInRange(triangle)) {
            errors.emplace_back("Triangle " + std::to_string(i) + " contains a vertex index outside the mesh vertex array.");
            continue;
        }

        if (triangle.a == triangle.b || triangle.b == triangle.c || triangle.a == triangle.c) {
            errors.emplace_back("Triangle " + std::to_string(i) + " contains duplicate vertex indices.");
            continue;
        }

        if (std::abs(signedTriangleArea(triangle)) <= kAreaTolerance) {
            errors.emplace_back("Triangle " + std::to_string(i) + " has zero or near-zero area.");
        }
    }

    std::unordered_set<std::size_t> seenBoundaryVertices;
    for (const std::size_t index : boundaryVertexIndices_) {
        if (index >= vertices_.size()) {
            errors.emplace_back("Boundary vertex index " + std::to_string(index) + " is outside the mesh vertex array.");
        }

        if (!seenBoundaryVertices.insert(index).second) {
            errors.emplace_back("Boundary vertex index " + std::to_string(index) + " is duplicated.");
        }
    }

    return errors;
}

bool Mesh::triangleIndicesInRange(const Triangle& triangle) const noexcept {
    return triangle.a < vertices_.size() &&
           triangle.b < vertices_.size() &&
           triangle.c < vertices_.size();
}

double Mesh::signedTriangleArea(const Triangle& triangle) const {
    if (!triangleIndicesInRange(triangle)) {
        throw std::out_of_range("Triangle contains a vertex index outside the mesh vertex array.");
    }

    return signedAreaFromPoints(
        vertices_[triangle.a],
        vertices_[triangle.b],
        vertices_[triangle.c]
    );
}

} // namespace cppTDGL
