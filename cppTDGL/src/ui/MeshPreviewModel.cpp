#include "MeshPreviewModel.hpp"

#include "MeshOperators.hpp"

#include <algorithm>
#include <stdexcept>
#include <string>
#include <utility>

namespace cppTDGL {
namespace {

[[nodiscard]] bool containsIndex(const std::vector<std::size_t>& values, std::size_t target) {
    return std::find(values.begin(), values.end(), target) != values.end();
}

} // namespace

MeshPreviewSelection MeshPreviewSelection::none() noexcept {
    return MeshPreviewSelection{};
}

MeshPreviewSelection MeshPreviewSelection::vertex(std::size_t index) noexcept {
    return MeshPreviewSelection{true, MeshPreviewSelectionKind::Vertex, index};
}

MeshPreviewSelection MeshPreviewSelection::triangle(std::size_t index) noexcept {
    return MeshPreviewSelection{true, MeshPreviewSelectionKind::Triangle, index};
}

MeshPreviewSelection MeshPreviewSelection::edge(std::size_t index) noexcept {
    return MeshPreviewSelection{true, MeshPreviewSelectionKind::Edge, index};
}

MeshPreviewModel MeshPreviewModel::fromMesh(Mesh mesh) {
    MeshPreviewModel model;
    model.setMesh(std::move(mesh));
    return model;
}

MeshPreviewModel MeshPreviewModel::fromDevice(const Device& device) {
    MeshPreviewModel model;

    if (device.hasMesh()) {
        model.setMesh(device.mesh());
    }

    return model;
}

bool MeshPreviewModel::hasMesh() const noexcept {
    return mesh_.has_value();
}

const Mesh& MeshPreviewModel::mesh() const {
    return requireMesh();
}

void MeshPreviewModel::setMesh(Mesh mesh) {
    mesh_ = std::move(mesh);
    selection_ = MeshPreviewSelection::none();
}

void MeshPreviewModel::clearMesh() {
    mesh_.reset();
    selection_ = MeshPreviewSelection::none();
}

const MeshPreviewSelection& MeshPreviewModel::selection() const noexcept {
    return selection_;
}

void MeshPreviewModel::clearSelection() noexcept {
    selection_ = MeshPreviewSelection::none();
}

void MeshPreviewModel::selectVertex(std::size_t index) {
    validateVertexIndexOrThrow(index);
    selection_ = MeshPreviewSelection::vertex(index);
}

void MeshPreviewModel::selectTriangle(std::size_t index) {
    validateTriangleIndexOrThrow(index);
    selection_ = MeshPreviewSelection::triangle(index);
}

void MeshPreviewModel::selectEdge(std::size_t index) {
    validateEdgeIndexOrThrow(index);
    selection_ = MeshPreviewSelection::edge(index);
}

MeshPreviewSummary MeshPreviewModel::summary() const {
    MeshPreviewSummary result;
    result.hasMesh = hasMesh();

    if (!hasMesh()) {
        result.validationErrorCount = 1;
        return result;
    }

    const Mesh& currentMesh = requireMesh();
    const std::vector<std::string> errors = currentMesh.validationErrors();

    result.isValid = errors.empty();
    result.validationErrorCount = errors.size();
    result.vertexCount = currentMesh.vertexCount();
    result.triangleCount = currentMesh.triangleCount();

    if (!result.isValid) {
        return result;
    }

    const EdgeMesh currentEdges = EdgeMesh::fromMesh(currentMesh);

    result.edgeCount = currentEdges.edgeCount();
    result.boundaryEdgeCount = currentEdges.boundaryEdgeCount();
    result.interiorEdgeCount = currentEdges.interiorEdgeCount();
    result.totalArea = currentMesh.totalArea();
    result.totalBoundaryLength = currentEdges.totalBoundaryLength();
    result.boundingBox = currentMesh.boundingBox();

    return result;
}

std::vector<MeshPreviewVertexInfo> MeshPreviewModel::vertexInfos() const {
    if (!hasValidMesh()) {
        return {};
    }

    const Mesh& currentMesh = requireMesh();
    const std::vector<double> controlVolumes = MeshOperators::vertexControlVolumes(currentMesh);

    std::vector<MeshPreviewVertexInfo> result;
    result.reserve(currentMesh.vertexCount());

    for (std::size_t i = 0; i < currentMesh.vertexCount(); ++i) {
        result.push_back({
            i,
            currentMesh.vertices()[i],
            controlVolumes[i],
            containsIndex(currentMesh.boundaryVertexIndices(), i),
        });
    }

    return result;
}

std::vector<MeshPreviewTriangleInfo> MeshPreviewModel::triangleInfos() const {
    if (!hasValidMesh()) {
        return {};
    }

    const Mesh& currentMesh = requireMesh();
    const std::vector<Point2D> centroids = MeshOperators::triangleCentroids(currentMesh);

    std::vector<MeshPreviewTriangleInfo> result;
    result.reserve(currentMesh.triangleCount());

    for (std::size_t i = 0; i < currentMesh.triangleCount(); ++i) {
        const Triangle& triangle = currentMesh.triangles()[i];
        result.push_back({
            i,
            triangle.a,
            triangle.b,
            triangle.c,
            centroids[i],
            currentMesh.triangleArea(i),
        });
    }

    return result;
}

std::vector<MeshPreviewEdgeInfo> MeshPreviewModel::edgeInfos() const {
    if (!hasValidMesh()) {
        return {};
    }

    const EdgeMesh currentEdges = buildEdgeMesh();

    std::vector<MeshPreviewEdgeInfo> result;
    result.reserve(currentEdges.edgeCount());

    for (std::size_t i = 0; i < currentEdges.edgeCount(); ++i) {
        const MeshEdge& edge = currentEdges.edges()[i];
        result.push_back({
            i,
            edge.a,
            edge.b,
            edge.center,
            edge.length,
            edge.triangleUseCount,
            edge.isBoundary,
        });
    }

    return result;
}

std::vector<MeshPreviewEdgeInfo> MeshPreviewModel::boundaryEdgeInfos() const {
    std::vector<MeshPreviewEdgeInfo> result;

    for (const MeshPreviewEdgeInfo& edge : edgeInfos()) {
        if (edge.isBoundary) {
            result.push_back(edge);
        }
    }

    return result;
}

std::vector<double> MeshPreviewModel::vertexControlVolumes() const {
    if (!hasValidMesh()) {
        return {};
    }

    return MeshOperators::vertexControlVolumes(requireMesh());
}

bool MeshPreviewModel::isValid() const {
    return validationErrors().empty();
}

std::vector<std::string> MeshPreviewModel::validationErrors() const {
    std::vector<std::string> errors;

    if (!hasMesh()) {
        errors.emplace_back("Mesh preview model requires an attached mesh.");
        return errors;
    }

    const Mesh& currentMesh = requireMesh();
    const std::vector<std::string> meshErrors = currentMesh.validationErrors();

    for (const std::string& error : meshErrors) {
        errors.push_back("Mesh: " + error);
    }

    if (!meshErrors.empty()) {
        return errors;
    }

    if (selection_.hasSelection) {
        if (selection_.kind == MeshPreviewSelectionKind::Vertex && selection_.index >= currentMesh.vertexCount()) {
            errors.emplace_back("Selected vertex index is out of range.");
        } else if (selection_.kind == MeshPreviewSelectionKind::Triangle && selection_.index >= currentMesh.triangleCount()) {
            errors.emplace_back("Selected triangle index is out of range.");
        } else if (selection_.kind == MeshPreviewSelectionKind::Edge && selection_.index >= buildEdgeMesh().edgeCount()) {
            errors.emplace_back("Selected edge index is out of range.");
        }
    }

    return errors;
}

std::string MeshPreviewModel::diagnosticSummary() const {
    const MeshPreviewSummary currentSummary = summary();

    if (!currentSummary.hasMesh) {
        return "No mesh loaded.";
    }

    if (!currentSummary.isValid) {
        return "Mesh invalid: " + std::to_string(currentSummary.validationErrorCount) + " validation error(s).";
    }

    return "Mesh valid: " +
           std::to_string(currentSummary.vertexCount) + " vertices, " +
           std::to_string(currentSummary.triangleCount) + " triangles, " +
           std::to_string(currentSummary.edgeCount) + " edges.";
}

const Mesh& MeshPreviewModel::requireMesh() const {
    if (!mesh_.has_value()) {
        throw std::logic_error("MeshPreviewModel does not contain a mesh.");
    }

    return *mesh_;
}

bool MeshPreviewModel::hasValidMesh() const {
    return hasMesh() && requireMesh().isValid();
}

EdgeMesh MeshPreviewModel::buildEdgeMesh() const {
    return EdgeMesh::fromMesh(requireMesh());
}

void MeshPreviewModel::validateVertexIndexOrThrow(std::size_t index) const {
    const Mesh& currentMesh = requireMesh();

    if (index >= currentMesh.vertexCount()) {
        throw std::out_of_range("Mesh preview vertex selection index is out of range.");
    }
}

void MeshPreviewModel::validateTriangleIndexOrThrow(std::size_t index) const {
    const Mesh& currentMesh = requireMesh();

    if (index >= currentMesh.triangleCount()) {
        throw std::out_of_range("Mesh preview triangle selection index is out of range.");
    }
}

void MeshPreviewModel::validateEdgeIndexOrThrow(std::size_t index) const {
    const EdgeMesh currentEdges = buildEdgeMesh();

    if (index >= currentEdges.edgeCount()) {
        throw std::out_of_range("Mesh preview edge selection index is out of range.");
    }
}

} // namespace cppTDGL
