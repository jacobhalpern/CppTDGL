#include "ResultsDashboardModel.hpp"

#include <algorithm>
#include <cmath>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>

namespace cppTDGL {
namespace {

[[nodiscard]] ResultsFieldRange rangeForValues(const std::vector<double>& values) {
    if (values.empty()) {
        return {};
    }

    ResultsFieldRange range;
    range.valid = true;
    range.minimum = std::numeric_limits<double>::infinity();
    range.maximum = -std::numeric_limits<double>::infinity();

    for (const double value : values) {
        range.minimum = std::min(range.minimum, value);
        range.maximum = std::max(range.maximum, value);
    }

    return range;
}

[[nodiscard]] ResultsFieldRange orderParameterMagnitudeRange(const SolverFrame& frame) {
    const std::size_t valueCount = std::min(
        frame.orderParameter.real.size(),
        frame.orderParameter.imaginary.size()
    );

    std::vector<double> magnitudes;
    magnitudes.reserve(valueCount);

    for (std::size_t i = 0; i < valueCount; ++i) {
        const double real = frame.orderParameter.real[i];
        const double imaginary = frame.orderParameter.imaginary[i];
        magnitudes.push_back(std::sqrt((real * real) + (imaginary * imaginary)));
    }

    return rangeForValues(magnitudes);
}

void appendErrors(
    std::vector<std::string>& destination,
    const std::string& prefix,
    const std::vector<std::string>& source
) {
    for (const std::string& error : source) {
        destination.push_back(prefix + error);
    }
}

[[nodiscard]] std::string frameCountText(std::size_t frameCount) {
    return std::to_string(frameCount) + (frameCount == 1 ? " frame" : " frames");
}

} // namespace

double ResultsFieldRange::span() const noexcept {
    if (!valid) {
        return 0.0;
    }

    return maximum - minimum;
}

ResultsDashboardSelection ResultsDashboardSelection::none() noexcept {
    return {};
}

ResultsDashboardSelection ResultsDashboardSelection::frame(std::size_t index) noexcept {
    return {ResultsDashboardSelectionKind::Frame, index};
}

ResultsDashboardSelection ResultsDashboardSelection::vertex(std::size_t index) noexcept {
    return {ResultsDashboardSelectionKind::Vertex, index};
}

ResultsDashboardSelection ResultsDashboardSelection::triangle(std::size_t index) noexcept {
    return {ResultsDashboardSelectionKind::Triangle, index};
}

ResultsDashboardSelection ResultsDashboardSelection::edge(std::size_t index) noexcept {
    return {ResultsDashboardSelectionKind::Edge, index};
}

ResultsDashboardModel ResultsDashboardModel::fromSolution(Solution solution) {
    ResultsDashboardModel model;
    model.setSolution(std::move(solution));
    return model;
}

bool ResultsDashboardModel::hasSolution() const noexcept {
    return solution_.has_value();
}

const Solution& ResultsDashboardModel::solution() const {
    return requireSolution();
}

void ResultsDashboardModel::setSolution(Solution solution) {
    solution_ = std::move(solution);
    clearSelection();
}

void ResultsDashboardModel::clearSolution() {
    solution_.reset();
    clearSelection();
}

const ResultsDashboardSelection& ResultsDashboardModel::selection() const noexcept {
    return selection_;
}

void ResultsDashboardModel::clearSelection() noexcept {
    selection_ = ResultsDashboardSelection::none();
}

void ResultsDashboardModel::selectFrame(std::size_t index) {
    validateFrameIndexOrThrow(index);
    selection_ = ResultsDashboardSelection::frame(index);
}

void ResultsDashboardModel::selectVertex(std::size_t index) {
    validateVertexIndexOrThrow(index);
    selection_ = ResultsDashboardSelection::vertex(index);
}

void ResultsDashboardModel::selectTriangle(std::size_t index) {
    validateTriangleIndexOrThrow(index);
    selection_ = ResultsDashboardSelection::triangle(index);
}

void ResultsDashboardModel::selectEdge(std::size_t index) {
    validateEdgeIndexOrThrow(index);
    selection_ = ResultsDashboardSelection::edge(index);
}

bool ResultsDashboardModel::isValid() const {
    return validationErrors().empty();
}

std::vector<std::string> ResultsDashboardModel::validationErrors() const {
    std::vector<std::string> errors;

    if (!solution_.has_value()) {
        errors.emplace_back("ResultsDashboardModel requires a loaded Solution.");
        return errors;
    }

    appendErrors(errors, "Solution: ", solution_->validationErrors());
    return errors;
}

std::vector<std::string> ResultsDashboardModel::diagnostics() const {
    std::vector<std::string> values;

    if (!solution_.has_value()) {
        values.emplace_back("No solution loaded.");
        return values;
    }

    values.insert(
        values.end(),
        solution_->diagnostics().begin(),
        solution_->diagnostics().end()
    );

    appendErrors(values, "Validation: ", validationErrors());
    return values;
}

std::string ResultsDashboardModel::statusText() const {
    if (!solution_.has_value()) {
        return "No solution loaded.";
    }

    if (!solution_->isValid()) {
        return "Solution has validation errors.";
    }

    if (solution_->completed()) {
        return "Simulation completed with " + frameCountText(solution_->frameCount()) + ".";
    }

    if (solution_->hasFrames()) {
        return "Solution loaded with " + frameCountText(solution_->frameCount()) + ".";
    }

    return "Solution loaded without saved frames.";
}

ResultsDashboardSummary ResultsDashboardModel::summary() const {
    ResultsDashboardSummary value;
    value.statusText = statusText();

    if (!solution_.has_value()) {
        return value;
    }

    value.hasSolution = true;
    value.solutionValid = solution_->isValid();
    value.projectName = solution_->projectName();
    value.completed = solution_->completed();
    value.hasResultPath = solution_->hasResultPath();
    value.resultPath = solution_->resultPath();
    value.hasMesh = solution_->hasMesh();
    value.hasFrames = solution_->hasFrames();
    value.frameCount = solution_->frameCount();
    value.diagnosticCount = solution_->diagnostics().size();

    if (solution_->hasMesh() && solution_->mesh().isValid()) {
        const Mesh& mesh = solution_->mesh();
        const EdgeMesh edgeMesh = EdgeMesh::fromMesh(mesh);

        value.vertexCount = mesh.vertexCount();
        value.triangleCount = mesh.triangleCount();
        value.meshArea = mesh.totalArea();
        value.edgeCount = edgeMesh.edgeCount();
        value.boundaryEdgeCount = edgeMesh.boundaryEdgeCount();
        value.interiorEdgeCount = edgeMesh.interiorEdgeCount();
    }

    if (solution_->hasFrames()) {
        value.startTime = solution_->frame(0).time;
        value.endTime = solution_->latestFrame().time;
        value.latestTime = solution_->latestFrame().time;
    }

    return value;
}

std::vector<ResultsFrameSummary> ResultsDashboardModel::frameSummaries() const {
    const Solution& result = requireSolution();

    std::vector<ResultsFrameSummary> summaries;
    summaries.reserve(result.frameCount());

    for (std::size_t i = 0; i < result.frameCount(); ++i) {
        summaries.push_back(frameSummary(i));
    }

    return summaries;
}

ResultsFrameSummary ResultsDashboardModel::frameSummary(std::size_t index) const {
    validateFrameIndexOrThrow(index);

    const Solution& result = requireSolution();
    const SolverFrame& frame = result.frame(index);

    return ResultsFrameSummary{
        index,
        frame.time,
        orderParameterMagnitudeRange(frame),
        rangeForValues(frame.scalarPotential.values),
        rangeForValues(frame.vectorPotential.values),
        rangeForValues(frame.currentDensity.values),
    };
}

const Solution& ResultsDashboardModel::requireSolution() const {
    if (!solution_.has_value()) {
        throw std::logic_error("ResultsDashboardModel does not contain a Solution.");
    }

    return *solution_;
}

const Mesh& ResultsDashboardModel::requireMesh() const {
    const Solution& result = requireSolution();

    if (!result.hasMesh()) {
        throw std::logic_error("ResultsDashboardModel Solution does not contain a Mesh.");
    }

    return result.mesh();
}

EdgeMesh ResultsDashboardModel::buildEdgeMesh() const {
    const Mesh& mesh = requireMesh();

    if (!mesh.isValid()) {
        throw std::logic_error("ResultsDashboardModel Solution contains an invalid Mesh.");
    }

    return EdgeMesh::fromMesh(mesh);
}

void ResultsDashboardModel::validateFrameIndexOrThrow(std::size_t index) const {
    const Solution& result = requireSolution();

    if (index >= result.frameCount()) {
        throw std::out_of_range("ResultsDashboardModel frame index is out of range.");
    }
}

void ResultsDashboardModel::validateVertexIndexOrThrow(std::size_t index) const {
    const Mesh& mesh = requireMesh();

    if (index >= mesh.vertexCount()) {
        throw std::out_of_range("ResultsDashboardModel vertex index is out of range.");
    }
}

void ResultsDashboardModel::validateTriangleIndexOrThrow(std::size_t index) const {
    const Mesh& mesh = requireMesh();

    if (index >= mesh.triangleCount()) {
        throw std::out_of_range("ResultsDashboardModel triangle index is out of range.");
    }
}

void ResultsDashboardModel::validateEdgeIndexOrThrow(std::size_t index) const {
    const EdgeMesh edgeMesh = buildEdgeMesh();

    if (index >= edgeMesh.edgeCount()) {
        throw std::out_of_range("ResultsDashboardModel edge index is out of range.");
    }
}

} // namespace cppTDGL
