#include "EdgeMesh.hpp"
#include "Mesh.hpp"
#include "ResultsDashboardModel.hpp"
#include "Solution.hpp"
#include "SolverState.hpp"

#include <cmath>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

constexpr double kTolerance = 1.0e-12;

void failIfFalse(bool condition, const std::string& message) {
    if (!condition) {
        throw std::runtime_error(message);
    }
}

void failIfNotNear(double actual, double expected, const std::string& message) {
    if (std::abs(actual - expected) > kTolerance) {
        throw std::runtime_error(
            message + " Expected " + std::to_string(expected) +
            ", got " + std::to_string(actual) + "."
        );
    }
}

cppTDGL::Mesh makeSquareMesh() {
    const std::vector<cppTDGL::Point2D> vertices{
        {0.0, 0.0},
        {1.0, 0.0},
        {1.0, 1.0},
        {0.0, 1.0},
    };

    const std::vector<cppTDGL::Triangle> triangles{
        {0, 1, 2},
        {0, 2, 3},
    };

    const std::vector<std::size_t> boundaryVertices{0, 1, 2, 3};

    return cppTDGL::Mesh(vertices, triangles, boundaryVertices);
}

cppTDGL::Solution makeSolution() {
    const cppTDGL::Mesh mesh = makeSquareMesh();
    const cppTDGL::EdgeMesh edgeMesh = cppTDGL::EdgeMesh::fromMesh(mesh);

    cppTDGL::SolverFrame first = cppTDGL::SolverFrame::uniformOrderParameterFor(
        mesh,
        edgeMesh,
        0.0,
        1.0,
        0.0
    );

    first.scalarPotential.values = {0.0, 1.0, 2.0, 3.0};
    first.vectorPotential.values = {0.0, 1.0, 2.0, 3.0, 4.0};
    first.currentDensity.values = {5.0, 4.0, 3.0, 2.0, 1.0};

    cppTDGL::SolverFrame second = cppTDGL::SolverFrame::uniformOrderParameterFor(
        mesh,
        edgeMesh,
        2.5,
        0.5,
        0.5
    );

    second.scalarPotential.values = {3.0, 2.0, 1.0, 0.0};
    second.vectorPotential.values = {2.0, 2.0, 2.0, 2.0, 2.0};
    second.currentDensity.values = {0.0, 0.25, 0.5, 0.75, 1.0};

    cppTDGL::Solution solution(
        "results_dashboard_smoke_test",
        mesh,
        {first, second},
        "results_dashboard_smoke_test.h5"
    );

    solution.setCompleted(true);
    solution.addDiagnostic("results dashboard smoke diagnostic");

    return solution;
}

bool throwsOutOfRangeForFrame(cppTDGL::ResultsDashboardModel& model) {
    try {
        model.selectFrame(99);
    } catch (const std::out_of_range&) {
        return true;
    }

    return false;
}

bool throwsOutOfRangeForEdge(cppTDGL::ResultsDashboardModel& model) {
    try {
        model.selectEdge(99);
    } catch (const std::out_of_range&) {
        return true;
    }

    return false;
}

} // namespace

int main() {
    try {
        cppTDGL::ResultsDashboardModel emptyModel;

        failIfFalse(!emptyModel.hasSolution(), "Default dashboard model should start without a solution.");
        failIfFalse(!emptyModel.isValid(), "Dashboard model without a solution should not be valid.");
        failIfFalse(
            emptyModel.statusText() == "No solution loaded.",
            "Empty dashboard status text should report that no solution is loaded."
        );
        failIfFalse(
            emptyModel.summary().statusText == "No solution loaded.",
            "Empty dashboard summary should preserve empty status text."
        );

        cppTDGL::ResultsDashboardModel model = cppTDGL::ResultsDashboardModel::fromSolution(makeSolution());

        failIfFalse(model.hasSolution(), "Dashboard model should report loaded solution.");
        failIfFalse(model.isValid(), "Dashboard model should be valid for valid solution.");
        failIfFalse(model.diagnostics().size() == 1, "Dashboard diagnostics should include solution diagnostics.");

        const cppTDGL::ResultsDashboardSummary summary = model.summary();

        failIfFalse(summary.hasSolution, "Dashboard summary should report a loaded solution.");
        failIfFalse(summary.solutionValid, "Dashboard summary should report valid solution.");
        failIfFalse(summary.completed, "Dashboard summary should report completed solution.");
        failIfFalse(summary.hasResultPath, "Dashboard summary should report result path.");
        failIfFalse(summary.hasMesh, "Dashboard summary should report mesh.");
        failIfFalse(summary.hasFrames, "Dashboard summary should report frames.");
        failIfFalse(summary.projectName == "results_dashboard_smoke_test", "Project name should be summarized.");
        failIfFalse(summary.frameCount == 2, "Frame count should be summarized.");
        failIfFalse(summary.diagnosticCount == 1, "Diagnostic count should be summarized.");
        failIfFalse(summary.vertexCount == 4, "Vertex count should be summarized.");
        failIfFalse(summary.triangleCount == 2, "Triangle count should be summarized.");
        failIfFalse(summary.edgeCount == 5, "Edge count should be summarized.");
        failIfFalse(summary.boundaryEdgeCount == 4, "Boundary edge count should be summarized.");
        failIfFalse(summary.interiorEdgeCount == 1, "Interior edge count should be summarized.");
        failIfNotNear(summary.meshArea, 1.0, "Mesh area should be summarized.");
        failIfNotNear(summary.startTime, 0.0, "Start time should be summarized.");
        failIfNotNear(summary.endTime, 2.5, "End time should be summarized.");
        failIfNotNear(summary.latestTime, 2.5, "Latest time should be summarized.");
        failIfFalse(
            summary.statusText == "Simulation completed with 2 frames.",
            "Dashboard status text should summarize completed frame count."
        );

        const std::vector<cppTDGL::ResultsFrameSummary> frameSummaries = model.frameSummaries();

        failIfFalse(frameSummaries.size() == 2, "Dashboard should summarize every solution frame.");
        failIfFalse(frameSummaries[0].frameIndex == 0, "First frame summary should preserve index.");
        failIfFalse(frameSummaries[1].frameIndex == 1, "Second frame summary should preserve index.");
        failIfNotNear(frameSummaries[1].time, 2.5, "Second frame summary should preserve time.");

        failIfFalse(frameSummaries[0].orderParameterMagnitude.valid, "Order parameter range should be valid.");
        failIfNotNear(frameSummaries[0].orderParameterMagnitude.minimum, 1.0, "First frame OP magnitude minimum should be 1.");
        failIfNotNear(frameSummaries[0].orderParameterMagnitude.maximum, 1.0, "First frame OP magnitude maximum should be 1.");
        failIfNotNear(frameSummaries[0].scalarPotential.minimum, 0.0, "First frame scalar potential minimum should be 0.");
        failIfNotNear(frameSummaries[0].scalarPotential.maximum, 3.0, "First frame scalar potential maximum should be 3.");
        failIfNotNear(frameSummaries[0].vectorPotential.minimum, 0.0, "First frame vector potential minimum should be 0.");
        failIfNotNear(frameSummaries[0].vectorPotential.maximum, 4.0, "First frame vector potential maximum should be 4.");
        failIfNotNear(frameSummaries[0].currentDensity.minimum, 1.0, "First frame current density minimum should be 1.");
        failIfNotNear(frameSummaries[0].currentDensity.maximum, 5.0, "First frame current density maximum should be 5.");

        const double expectedMagnitude = std::sqrt(0.5);
        failIfNotNear(frameSummaries[1].orderParameterMagnitude.minimum, expectedMagnitude, "Second frame OP magnitude minimum should match.");
        failIfNotNear(frameSummaries[1].orderParameterMagnitude.maximum, expectedMagnitude, "Second frame OP magnitude maximum should match.");

        model.selectFrame(1);
        failIfFalse(model.selection().kind == cppTDGL::ResultsDashboardSelectionKind::Frame, "Frame selection should be recorded.");
        failIfFalse(model.selection().index == 1, "Frame selection index should be recorded.");

        model.selectVertex(3);
        failIfFalse(model.selection().kind == cppTDGL::ResultsDashboardSelectionKind::Vertex, "Vertex selection should be recorded.");
        failIfFalse(model.selection().index == 3, "Vertex selection index should be recorded.");

        model.selectTriangle(1);
        failIfFalse(model.selection().kind == cppTDGL::ResultsDashboardSelectionKind::Triangle, "Triangle selection should be recorded.");
        failIfFalse(model.selection().index == 1, "Triangle selection index should be recorded.");

        model.selectEdge(4);
        failIfFalse(model.selection().kind == cppTDGL::ResultsDashboardSelectionKind::Edge, "Edge selection should be recorded.");
        failIfFalse(model.selection().index == 4, "Edge selection index should be recorded.");

        failIfFalse(throwsOutOfRangeForFrame(model), "Invalid frame selection should throw.");
        failIfFalse(throwsOutOfRangeForEdge(model), "Invalid edge selection should throw.");

        model.clearSolution();
        failIfFalse(!model.hasSolution(), "clearSolution() should clear the loaded solution.");
        failIfFalse(model.selection().kind == cppTDGL::ResultsDashboardSelectionKind::None, "clearSolution() should clear selection.");

        std::cout << "Results dashboard model smoke test passed.\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "Results dashboard model smoke test exception: " << error.what() << '\n';
        return 1;
    }
}
