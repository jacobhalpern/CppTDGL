#pragma once

#include "EdgeMesh.hpp"
#include "Solution.hpp"

#include <cstddef>
#include <filesystem>
#include <optional>
#include <string>
#include <vector>

namespace cppTDGL {

struct ResultsFieldRange {
    bool valid = false;
    double minimum = 0.0;
    double maximum = 0.0;

    [[nodiscard]] double span() const noexcept;
};

struct ResultsFrameSummary {
    std::size_t frameIndex = 0;
    double time = 0.0;
    ResultsFieldRange orderParameterMagnitude;
    ResultsFieldRange scalarPotential;
    ResultsFieldRange vectorPotential;
    ResultsFieldRange currentDensity;
};

struct ResultsDashboardSummary {
    std::string projectName;
    bool hasSolution = false;
    bool solutionValid = false;
    bool completed = false;
    bool hasResultPath = false;
    std::filesystem::path resultPath;
    bool hasMesh = false;
    std::size_t vertexCount = 0;
    std::size_t triangleCount = 0;
    std::size_t edgeCount = 0;
    std::size_t boundaryEdgeCount = 0;
    std::size_t interiorEdgeCount = 0;
    double meshArea = 0.0;
    bool hasFrames = false;
    std::size_t frameCount = 0;
    double startTime = 0.0;
    double endTime = 0.0;
    double latestTime = 0.0;
    std::size_t diagnosticCount = 0;
    std::string statusText;
};

enum class ResultsDashboardSelectionKind {
    None,
    Frame,
    Vertex,
    Triangle,
    Edge,
};

struct ResultsDashboardSelection {
    ResultsDashboardSelectionKind kind = ResultsDashboardSelectionKind::None;
    std::size_t index = 0;

    [[nodiscard]] static ResultsDashboardSelection none() noexcept;
    [[nodiscard]] static ResultsDashboardSelection frame(std::size_t index) noexcept;
    [[nodiscard]] static ResultsDashboardSelection vertex(std::size_t index) noexcept;
    [[nodiscard]] static ResultsDashboardSelection triangle(std::size_t index) noexcept;
    [[nodiscard]] static ResultsDashboardSelection edge(std::size_t index) noexcept;
};

class ResultsDashboardModel {
public:
    ResultsDashboardModel() = default;

    [[nodiscard]] static ResultsDashboardModel fromSolution(Solution solution);

    [[nodiscard]] bool hasSolution() const noexcept;
    [[nodiscard]] const Solution& solution() const;
    void setSolution(Solution solution);
    void clearSolution();

    [[nodiscard]] const ResultsDashboardSelection& selection() const noexcept;
    void clearSelection() noexcept;
    void selectFrame(std::size_t index);
    void selectVertex(std::size_t index);
    void selectTriangle(std::size_t index);
    void selectEdge(std::size_t index);

    [[nodiscard]] bool isValid() const;
    [[nodiscard]] std::vector<std::string> validationErrors() const;
    [[nodiscard]] std::vector<std::string> diagnostics() const;
    [[nodiscard]] std::string statusText() const;

    [[nodiscard]] ResultsDashboardSummary summary() const;
    [[nodiscard]] std::vector<ResultsFrameSummary> frameSummaries() const;
    [[nodiscard]] ResultsFrameSummary frameSummary(std::size_t index) const;

private:
    std::optional<Solution> solution_;
    ResultsDashboardSelection selection_ = ResultsDashboardSelection::none();

    [[nodiscard]] const Solution& requireSolution() const;
    [[nodiscard]] const Mesh& requireMesh() const;
    [[nodiscard]] EdgeMesh buildEdgeMesh() const;

    void validateFrameIndexOrThrow(std::size_t index) const;
    void validateVertexIndexOrThrow(std::size_t index) const;
    void validateTriangleIndexOrThrow(std::size_t index) const;
    void validateEdgeIndexOrThrow(std::size_t index) const;
};

} // namespace cppTDGL
