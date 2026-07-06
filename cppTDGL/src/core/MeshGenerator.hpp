#pragma once

#include "Device.hpp"
#include "Mesh.hpp"
#include "Polygon2D.hpp"

#include <cstddef>
#include <string>
#include <vector>

namespace cppTDGL {

struct MeshGenerationOptions {
    std::size_t xDivisions = 1;
    std::size_t yDivisions = 1;
    bool includeBoundaryVertices = true;

    [[nodiscard]] bool isValid() const;
    [[nodiscard]] std::vector<std::string> validationErrors() const;
};

class MeshGenerator {
public:
    [[nodiscard]] static Mesh generateRectangle(
        double width,
        double height,
        const MeshGenerationOptions& options,
        Point2D center = {}
    );

    [[nodiscard]] static Mesh generateForFilmBoundingBox(
        const Polygon2D& film,
        const MeshGenerationOptions& options
    );

    [[nodiscard]] static Mesh generateForDeviceFilmBoundingBox(
        const Device& device,
        const MeshGenerationOptions& options
    );

    [[nodiscard]] static Device attachGeneratedFilmBoundingBoxMesh(
        Device device,
        const MeshGenerationOptions& options
    );

private:
    static void validateOptionsOrThrow(const MeshGenerationOptions& options);
    static void validateRectangleDimensionsOrThrow(double width, double height);
};

} // namespace cppTDGL
