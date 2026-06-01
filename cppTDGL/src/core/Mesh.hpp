#pragma once

#include "Polygon2D.hpp"

#include <cstddef>
#include <string>
#include <vector>

namespace cppTDGL {

struct Triangle {
    std::size_t a = 0;
    std::size_t b = 0;
    std::size_t c = 0;
};

class Mesh {
public:
    Mesh() = default;
    Mesh(
        std::vector<Point2D> vertices,
        std::vector<Triangle> triangles,
        std::vector<std::size_t> boundaryVertexIndices = {}
    );

    [[nodiscard]] const std::vector<Point2D>& vertices() const noexcept;
    [[nodiscard]] const std::vector<Triangle>& triangles() const noexcept;
    [[nodiscard]] const std::vector<std::size_t>& boundaryVertexIndices() const noexcept;

    void setVertices(std::vector<Point2D> value);
    void setTriangles(std::vector<Triangle> value);
    void setBoundaryVertexIndices(std::vector<std::size_t> value);

    [[nodiscard]] std::size_t vertexCount() const noexcept;
    [[nodiscard]] std::size_t triangleCount() const noexcept;
    [[nodiscard]] bool isEmpty() const noexcept;

    [[nodiscard]] double triangleArea(std::size_t triangleIndex) const;
    [[nodiscard]] std::vector<double> triangleAreas() const;
    [[nodiscard]] double totalArea() const;
    [[nodiscard]] BoundingBox boundingBox() const;

    [[nodiscard]] bool isValid() const;
    [[nodiscard]] std::vector<std::string> validationErrors() const;

private:
    [[nodiscard]] bool triangleIndicesInRange(const Triangle& triangle) const noexcept;
    [[nodiscard]] double signedTriangleArea(const Triangle& triangle) const;

    std::vector<Point2D> vertices_;
    std::vector<Triangle> triangles_;
    std::vector<std::size_t> boundaryVertexIndices_;
};

} // namespace cppTDGL
