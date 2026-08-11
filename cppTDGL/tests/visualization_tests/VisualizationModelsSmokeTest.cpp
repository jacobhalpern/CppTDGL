#include "CurrentDensityPlot.hpp"
#include "EdgeMesh.hpp"
#include "FieldPlot.hpp"
#include "Mesh.hpp"
#include "MeshPlot.hpp"
#include "Solution.hpp"
#include "SolverState.hpp"
#include "TimeSeriesPlot.hpp"

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

cppTDGL::Solution makeSolution(const cppTDGL::Mesh& mesh) {
    const cppTDGL::EdgeMesh edgeMesh = cppTDGL::EdgeMesh::fromMesh(mesh);

    cppTDGL::SolverFrame first = cppTDGL::SolverFrame::uniformOrderParameterFor(
        mesh,
        edgeMesh,
        0.0,
        1.0,
        0.0
    );

    cppTDGL::SolverFrame second = cppTDGL::SolverFrame::zerosFor(mesh, edgeMesh, 0.5);
    second.orderParameter.real = {3.0, 0.0, 5.0, 8.0};
    second.orderParameter.imaginary = {4.0, 2.0, 12.0, 15.0};
    second.currentDensity.values = {0.0, 1.0, -2.0, 3.0, -4.0};

    cppTDGL::Solution solution("visualization_smoke_test", mesh, {first, second});
    solution.setCompleted(true);
    return solution;
}

} // namespace

int main() {
    try {
        const cppTDGL::Mesh mesh = makeSquareMesh();
        const cppTDGL::EdgeMesh edgeMesh = cppTDGL::EdgeMesh::fromMesh(mesh);

        const cppTDGL::MeshPlotData meshPlot = cppTDGL::MeshPlot::fromMesh(mesh);

        failIfFalse(meshPlot.isValid(), "Mesh plot data should be valid for a valid mesh.");
        failIfFalse(meshPlot.hasDrawableGeometry(), "Mesh plot data should be drawable.");
        failIfFalse(meshPlot.vertices.size() == 4, "Mesh plot should expose 4 vertices.");
        failIfFalse(meshPlot.triangles.size() == 2, "Mesh plot should expose 2 triangles.");
        failIfFalse(meshPlot.edges.size() == 5, "Mesh plot should expose 5 edges.");
        failIfNotNear(meshPlot.totalArea, 1.0, "Mesh plot total area should be 1.");
        failIfNotNear(meshPlot.totalBoundaryLength, 4.0, "Mesh plot boundary length should be 4.");
        failIfFalse(meshPlot.vertices[0].isBoundary, "Boundary vertex flag should be set.");
        failIfNotNear(meshPlot.triangles[0].area, 0.5, "Triangle area should be exposed.");

        cppTDGL::VertexScalarField scalarField;
        scalarField.values = {0.0, 1.0, 2.0, 3.0};

        const cppTDGL::FieldPlotData scalarPlot = cppTDGL::FieldPlot::fromVertexScalarField(
            mesh,
            scalarField,
            "scalar potential"
        );

        failIfFalse(scalarPlot.isValid(), "Scalar field plot should be valid.");
        failIfFalse(scalarPlot.samples.size() == 4, "Scalar field plot should expose 4 samples.");
        failIfNotNear(scalarPlot.range.minimum, 0.0, "Scalar field minimum should be 0.");
        failIfNotNear(scalarPlot.range.maximum, 3.0, "Scalar field maximum should be 3.");
        failIfNotNear(scalarPlot.range.span(), 3.0, "Scalar field span should be 3.");

        cppTDGL::VertexComplexField complexField;
        complexField.real = {3.0, 0.0, 5.0, 8.0};
        complexField.imaginary = {4.0, 2.0, 12.0, 15.0};

        const cppTDGL::FieldPlotData magnitudePlot = cppTDGL::FieldPlot::orderParameterMagnitude(
            mesh,
            complexField
        );

        failIfFalse(magnitudePlot.isValid(), "Order-parameter magnitude plot should be valid.");
        failIfNotNear(magnitudePlot.range.minimum, 2.0, "Order-parameter magnitude minimum should be 2.");
        failIfNotNear(magnitudePlot.range.maximum, 17.0, "Order-parameter magnitude maximum should be 17.");

        cppTDGL::EdgeScalarField currentDensity;
        currentDensity.values = {0.0, 1.0, -2.0, 3.0, -4.0};

        const cppTDGL::CurrentDensityPlotData currentPlot = cppTDGL::CurrentDensityPlot::fromMesh(
            mesh,
            currentDensity
        );

        failIfFalse(currentPlot.isValid(), "Current-density plot should be valid.");
        failIfFalse(currentPlot.vectors.size() == edgeMesh.edgeCount(), "Current-density plot should expose one vector per edge.");
        failIfNotNear(currentPlot.magnitudeRange.minimum, 0.0, "Current-density magnitude minimum should be 0.");
        failIfNotNear(currentPlot.magnitudeRange.maximum, 4.0, "Current-density magnitude maximum should be 4.");

        const cppTDGL::Solution solution = makeSolution(mesh);

        const cppTDGL::TimeSeriesData frameTimes = cppTDGL::TimeSeriesPlot::frameTimes(solution);
        failIfFalse(frameTimes.isValid(), "Frame-time series should be valid.");
        failIfFalse(frameTimes.points.size() == 2, "Frame-time series should expose 2 points.");
        failIfNotNear(frameTimes.points[0].y, 0.0, "First frame time should be 0.");
        failIfNotNear(frameTimes.points[1].y, 0.5, "Second frame time should be 0.5.");

        const cppTDGL::TimeSeriesData orderMagnitude = cppTDGL::TimeSeriesPlot::orderParameterMeanMagnitude(solution);
        failIfFalse(orderMagnitude.isValid(), "Mean order-parameter magnitude series should be valid.");
        failIfNotNear(orderMagnitude.points[0].y, 1.0, "First frame mean order magnitude should be 1.");
        failIfNotNear(orderMagnitude.points[1].y, 9.25, "Second frame mean order magnitude should be 9.25.");

        const cppTDGL::TimeSeriesData currentMagnitude = cppTDGL::TimeSeriesPlot::currentDensityMeanMagnitude(solution);
        failIfFalse(currentMagnitude.isValid(), "Mean current-density magnitude series should be valid.");
        failIfNotNear(currentMagnitude.points[0].y, 0.0, "First frame mean current-density magnitude should be 0.");
        failIfNotNear(currentMagnitude.points[1].y, 2.0, "Second frame mean current-density magnitude should be 2.");

        const cppTDGL::TimeSeriesData mismatchedSeries = cppTDGL::TimeSeriesPlot::fromValues(
            "bad series",
            "x",
            "y",
            {0.0, 1.0},
            {0.0}
        );

        failIfFalse(!mismatchedSeries.isValid(), "Mismatched time-series arrays should be invalid.");
        failIfFalse(!mismatchedSeries.diagnostics.empty(), "Mismatched time-series arrays should report diagnostics.");

        cppTDGL::MeshPlot meshPlotObject;
        cppTDGL::FieldPlot fieldPlotObject;
        cppTDGL::CurrentDensityPlot currentPlotObject;
        cppTDGL::TimeSeriesPlot timeSeriesPlotObject;

        failIfFalse(meshPlotObject.className() == "MeshPlot", "MeshPlot class name should be preserved.");
        failIfFalse(fieldPlotObject.className() == "FieldPlot", "FieldPlot class name should be preserved.");
        failIfFalse(currentPlotObject.className() == "CurrentDensityPlot", "CurrentDensityPlot class name should be preserved.");
        failIfFalse(timeSeriesPlotObject.className() == "TimeSeriesPlot", "TimeSeriesPlot class name should be preserved.");

        std::cout << "Visualization models smoke test passed.\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "Visualization models smoke test exception: " << error.what() << '\n';
        return 1;
    }
}
