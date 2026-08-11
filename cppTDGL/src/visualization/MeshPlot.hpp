#pragma once

#include "Device.hpp"
#include "EdgeMesh.hpp"
#include "Mesh.hpp"

#include <cstddef>
#include <string>
#include <vector>

namespace cppTDGL {

struct MeshPlotVertex {
    std::size_t index = 0;
    Point2D position{};
    bool isBoundary = false;
};

struct MeshPlotTriangle {
    std::size_t index = 0;
    std::size_t a = 0;
    std::size_t b = 0;
    std::size_t c = 0;
    Point2D centroid{};
    double area = 0.0;
};

struct MeshPlotEdge {
    std::size_t index = 0;
    std::size_t a = 0;
    std::size_t b = 0;
    Point2D center{};
    double length = 0.0;
    bool isBoundary = false;
};

struct MeshPlotData {
    bool hasMesh = false;
    bool meshIsValid = false;
    std::vector<MeshPlotVertex> vertices;
    std::vector<MeshPlotTriangle> triangles;
    std::vector<MeshPlotEdge> edges;
    BoundingBox boundingBox{};
    double totalArea = 0.0;
    double totalBoundaryLength = 0.0;
    std::vector<std::string> diagnostics;

    [[nodiscard]] bool isValid() const noexcept;
    [[nodiscard]] bool hasDrawableGeometry() const noexcept;
};

class MeshPlot {
public:
    MeshPlot() = default;
    virtual ~MeshPlot() = default;

    [[nodiscard]] std::string className() const;

    [[nodiscard]] static MeshPlotData fromMesh(const Mesh& mesh);
    [[nodiscard]] static MeshPlotData fromDevice(const Device& device);
};

} // namespace cppTDGL
