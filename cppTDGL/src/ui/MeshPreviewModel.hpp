#pragma once

#include "Device.hpp"
#include "EdgeMesh.hpp"
#include "Mesh.hpp"

#include <cstddef>
#include <optional>
#include <string>
#include <vector>

namespace cppTDGL {

struct MeshPreviewSummary {
    bool hasMesh = false;
    bool isValid = false;
    std::size_t validationErrorCount = 0;
    std::size_t vertexCount = 0;
    std::size_t triangleCount = 0;
    std::size_t edgeCount = 0;
    std::size_t boundaryEdgeCount = 0;
    std::size_t interiorEdgeCount = 0;
    double totalArea = 0.0;
    double totalBoundaryLength = 0.0;
    BoundingBox boundingBox{};
};

struct MeshPreviewVertexInfo {
    std::size_t index = 0;
    Point2D position{};
    double controlVolume = 0.0;
    bool isBoundary = false;
};

struct MeshPreviewTriangleInfo {
    std::size_t index = 0;
    std::size_t a = 0;
    std::size_t b = 0;
    std::size_t c = 0;
    Point2D centroid{};
    double area = 0.0;
};

struct MeshPreviewEdgeInfo {
    std::size_t index = 0;
    std::size_t a = 0;
    std::size_t b = 0;
    Point2D center{};
    double length = 0.0;
    std::size_t triangleUseCount = 0;
    bool isBoundary = false;
};

enum class MeshPreviewSelectionKind {
    None,
    Vertex,
    Triangle,
    Edge,
};

struct MeshPreviewSelection {
    bool hasSelection = false;
    MeshPreviewSelectionKind kind = MeshPreviewSelectionKind::None;
    std::size_t index = 0;

    [[nodiscard]] static MeshPreviewSelection none() noexcept;
    [[nodiscard]] static MeshPreviewSelection vertex(std::size_t index) noexcept;
    [[nodiscard]] static MeshPreviewSelection triangle(std::size_t index) noexcept;
    [[nodiscard]] static MeshPreviewSelection edge(std::size_t index) noexcept;
};

class MeshPreviewModel {
public:
    MeshPreviewModel() = default;

    [[nodiscard]] static MeshPreviewModel fromMesh(Mesh mesh);
    [[nodiscard]] static MeshPreviewModel fromDevice(const Device& device);

    [[nodiscard]] bool hasMesh() const noexcept;
    [[nodiscard]] const Mesh& mesh() const;
    void setMesh(Mesh mesh);
    void clearMesh();

    [[nodiscard]] const MeshPreviewSelection& selection() const noexcept;
    void clearSelection() noexcept;
    void selectVertex(std::size_t index);
    void selectTriangle(std::size_t index);
    void selectEdge(std::size_t index);

    [[nodiscard]] MeshPreviewSummary summary() const;
    [[nodiscard]] std::vector<MeshPreviewVertexInfo> vertexInfos() const;
    [[nodiscard]] std::vector<MeshPreviewTriangleInfo> triangleInfos() const;
    [[nodiscard]] std::vector<MeshPreviewEdgeInfo> edgeInfos() const;
    [[nodiscard]] std::vector<MeshPreviewEdgeInfo> boundaryEdgeInfos() const;
    [[nodiscard]] std::vector<double> vertexControlVolumes() const;

    [[nodiscard]] bool isValid() const;
    [[nodiscard]] std::vector<std::string> validationErrors() const;
    [[nodiscard]] std::string diagnosticSummary() const;

private:
    [[nodiscard]] const Mesh& requireMesh() const;
    [[nodiscard]] bool hasValidMesh() const;
    [[nodiscard]] EdgeMesh buildEdgeMesh() const;
    void validateVertexIndexOrThrow(std::size_t index) const;
    void validateTriangleIndexOrThrow(std::size_t index) const;
    void validateEdgeIndexOrThrow(std::size_t index) const;

    std::optional<Mesh> mesh_;
    MeshPreviewSelection selection_ = MeshPreviewSelection::none();
};

} // namespace cppTDGL
