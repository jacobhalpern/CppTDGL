#include "MeshPlot.hpp"

#include "MeshOperators.hpp"

#include <algorithm>
#include <string>

namespace cppTDGL {
namespace {

[[nodiscard]] bool containsIndex(const std::vector<std::size_t>& values, std::size_t index) {
    return std::find(values.begin(), values.end(), index) != values.end();
}

} // namespace

bool MeshPlotData::isValid() const noexcept {
    return hasMesh && meshIsValid && diagnostics.empty();
}

bool MeshPlotData::hasDrawableGeometry() const noexcept {
    return isValid() && !vertices.empty() && !triangles.empty();
}

std::string MeshPlot::className() const {
    return "MeshPlot";
}

MeshPlotData MeshPlot::fromMesh(const Mesh& mesh) {
    MeshPlotData data;
    data.hasMesh = true;
    data.meshIsValid = mesh.isValid();
    data.diagnostics = mesh.validationErrors();

    if (!data.meshIsValid) {
        return data;
    }

    data.boundingBox = mesh.boundingBox();
    data.totalArea = mesh.totalArea();

    data.vertices.reserve(mesh.vertexCount());
    for (std::size_t i = 0; i < mesh.vertexCount(); ++i) {
        data.vertices.push_back({
            i,
            mesh.vertices()[i],
            containsIndex(mesh.boundaryVertexIndices(), i),
        });
    }

    const std::vector<Point2D> centroids = MeshOperators::triangleCentroids(mesh);
    data.triangles.reserve(mesh.triangleCount());
    for (std::size_t i = 0; i < mesh.triangleCount(); ++i) {
        const Triangle& triangle = mesh.triangles()[i];
        data.triangles.push_back({
            i,
            triangle.a,
            triangle.b,
            triangle.c,
            centroids[i],
            mesh.triangleArea(i),
        });
    }

    const EdgeMesh edgeMesh = EdgeMesh::fromMesh(mesh);
    data.totalBoundaryLength = edgeMesh.totalBoundaryLength();

    data.edges.reserve(edgeMesh.edgeCount());
    for (std::size_t i = 0; i < edgeMesh.edgeCount(); ++i) {
        const MeshEdge& edge = edgeMesh.edges()[i];
        data.edges.push_back({
            i,
            edge.a,
            edge.b,
            edge.center,
            edge.length,
            edge.isBoundary,
        });
    }

    return data;
}

MeshPlotData MeshPlot::fromDevice(const Device& device) {
    if (!device.hasMesh()) {
        MeshPlotData data;
        data.hasMesh = false;
        data.meshIsValid = false;
        data.diagnostics.emplace_back("Device does not have an attached mesh.");
        return data;
    }

    return fromMesh(device.mesh());
}

} // namespace cppTDGL
