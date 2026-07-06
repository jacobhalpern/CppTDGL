#include "MeshGenerator.hpp"

#include <stdexcept>
#include <string>
#include <utility>

namespace cppTDGL {

bool MeshGenerationOptions::isValid() const {
    return validationErrors().empty();
}

std::vector<std::string> MeshGenerationOptions::validationErrors() const {
    std::vector<std::string> errors;

    if (xDivisions == 0) {
        errors.emplace_back("Mesh generation xDivisions must be greater than zero.");
    }

    if (yDivisions == 0) {
        errors.emplace_back("Mesh generation yDivisions must be greater than zero.");
    }

    return errors;
}

Mesh MeshGenerator::generateRectangle(
    double width,
    double height,
    const MeshGenerationOptions& options,
    Point2D center
) {
    validateOptionsOrThrow(options);
    validateRectangleDimensionsOrThrow(width, height);

    const std::size_t vertexColumns = options.xDivisions + 1;
    const std::size_t vertexRows = options.yDivisions + 1;

    const double dx = width / static_cast<double>(options.xDivisions);
    const double dy = height / static_cast<double>(options.yDivisions);

    const double xMinimum = center.x - width / 2.0;
    const double yMinimum = center.y - height / 2.0;

    std::vector<Point2D> vertices;
    vertices.reserve(vertexColumns * vertexRows);

    for (std::size_t iy = 0; iy < vertexRows; ++iy) {
        for (std::size_t ix = 0; ix < vertexColumns; ++ix) {
            vertices.push_back({
                xMinimum + static_cast<double>(ix) * dx,
                yMinimum + static_cast<double>(iy) * dy,
            });
        }
    }

    auto vertexIndex = [vertexColumns](std::size_t ix, std::size_t iy) {
        return iy * vertexColumns + ix;
    };

    std::vector<Triangle> triangles;
    triangles.reserve(options.xDivisions * options.yDivisions * 2);

    for (std::size_t iy = 0; iy < options.yDivisions; ++iy) {
        for (std::size_t ix = 0; ix < options.xDivisions; ++ix) {
            const std::size_t lowerLeft = vertexIndex(ix, iy);
            const std::size_t lowerRight = vertexIndex(ix + 1, iy);
            const std::size_t upperRight = vertexIndex(ix + 1, iy + 1);
            const std::size_t upperLeft = vertexIndex(ix, iy + 1);

            triangles.push_back({lowerLeft, lowerRight, upperRight});
            triangles.push_back({lowerLeft, upperRight, upperLeft});
        }
    }

    std::vector<std::size_t> boundaryVertices;

    if (options.includeBoundaryVertices) {
        boundaryVertices.reserve((2 * vertexColumns) + (2 * vertexRows));

        for (std::size_t iy = 0; iy < vertexRows; ++iy) {
            for (std::size_t ix = 0; ix < vertexColumns; ++ix) {
                const bool isBoundary =
                    ix == 0 ||
                    iy == 0 ||
                    ix == options.xDivisions ||
                    iy == options.yDivisions;

                if (isBoundary) {
                    boundaryVertices.push_back(vertexIndex(ix, iy));
                }
            }
        }
    }

    return Mesh(std::move(vertices), std::move(triangles), std::move(boundaryVertices));
}

Mesh MeshGenerator::generateForFilmBoundingBox(
    const Polygon2D& film,
    const MeshGenerationOptions& options
) {
    if (!film.isValid()) {
        throw std::invalid_argument("Cannot generate a mesh for an invalid film polygon.");
    }

    const BoundingBox bounds = film.boundingBox();

    if (!bounds.isValid()) {
        throw std::invalid_argument("Cannot generate a mesh for a film polygon with an invalid bounding box.");
    }

    const Point2D center{
        (bounds.minimum.x + bounds.maximum.x) / 2.0,
        (bounds.minimum.y + bounds.maximum.y) / 2.0,
    };

    return generateRectangle(bounds.width(), bounds.height(), options, center);
}

Mesh MeshGenerator::generateForDeviceFilmBoundingBox(
    const Device& device,
    const MeshGenerationOptions& options
) {
    return generateForFilmBoundingBox(device.film(), options);
}

Device MeshGenerator::attachGeneratedFilmBoundingBoxMesh(
    Device device,
    const MeshGenerationOptions& options
) {
    device.setMesh(generateForDeviceFilmBoundingBox(device, options));
    return device;
}

void MeshGenerator::validateOptionsOrThrow(const MeshGenerationOptions& options) {
    const std::vector<std::string> errors = options.validationErrors();

    if (!errors.empty()) {
        throw std::invalid_argument(errors.front());
    }
}

void MeshGenerator::validateRectangleDimensionsOrThrow(double width, double height) {
    if (width <= 0.0) {
        throw std::invalid_argument("Generated rectangle mesh width must be greater than zero.");
    }

    if (height <= 0.0) {
        throw std::invalid_argument("Generated rectangle mesh height must be greater than zero.");
    }
}

} // namespace cppTDGL
